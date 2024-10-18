// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ScreenLightControl.hpp"

#include <Timers/TimerFactory.hpp>
#include <Service/Service.hpp>

#include <system/Constants.hpp>
#include <system/messages/SentinelRegistrationMessage.hpp>

namespace pure::screen_light_control
{
    namespace
    {
        constexpr bsp::eink_frontlight::BrightnessPercentage fadeOutBrightnessMax = 35.0f;
    }

    ScreenLightController::ScreenLightController(sys::Service *parent)
        : cpuSentinel{std::make_shared<sys::CpuSentinel>(cpuSentinelName, parent)}
    {
        auto sentinelRegistrationMsg = std::make_shared<sys::SentinelRegistrationMessage>(cpuSentinel);
        parent->bus.sendUnicast(std::move(sentinelRegistrationMsg), service::name::system_manager);

        controlTimer =
            sys::TimerFactory::createPeriodicTimer(parent,
                                                   lightControlTimerName,
                                                   std::chrono::milliseconds{controlTimerIntervalMs},
                                                   [this]([[maybe_unused]] sys::Timer &t) { controlTimerCallback(); });
        readoutTimer =
            sys::TimerFactory::createPeriodicTimer(parent,
                                                   lightReadoutTimerName,
                                                   std::chrono::milliseconds{readoutTimerIntervalMs},
                                                   [this]([[maybe_unused]] sys::Timer &t) { readoutTimerCallback(); });

        setParameters(AutomaticModeParameters{});
    }

    ScreenLightController::~ScreenLightController()
    {
        disableTimers();
    }

    auto ScreenLightController::processRequest(Action action) -> void
    {
        processRequest(action, Parameters{});
    }

    auto ScreenLightController::processRequest(Action action, const Parameters &params) -> void
    {
        switch (action) {
        case Action::turnOff:
            turnOff();
            break;
        case Action::turnOn:
            turnOn();
            break;
        case Action::enableAutomaticMode:
            enableAutomaticMode();
            break;
        case Action::disableAutomaticMode:
            disableAutomaticMode();
            break;
        case Action::setManualModeBrightness:
            if (params.hasManualModeParams()) {
                setParameters(params.getManualModeParams());
            }
            break;
        case Action::setAutomaticModeParameters:
            if (params.hasAutoModeParams()) {
                setParameters(params.getAutoModeParams());
            }
            break;
        case Action::fadeOut:
            handleFadeOut();
            break;
        default:
            break;
        }
    }

    auto ScreenLightController::controlTimerCallback() -> void
    {
        bsp::eink_frontlight::setBrightness(::screen_light_control::functions::brightnessRampOut());
    }

    auto ScreenLightController::readoutTimerCallback() -> void
    {
        if (!fadeOut) {
            ::screen_light_control::functions::calculateBrightness(bsp::light_sensor::readout());
        }
    }

    auto ScreenLightController::isLightOn() const noexcept -> bool
    {
        return lightOn;
    }

    auto ScreenLightController::isAutoModeOn() const noexcept -> bool
    {
        return automaticMode == ScreenLightMode::Automatic;
    }

    auto ScreenLightController::getBrightnessValue() const noexcept -> bsp::eink_frontlight::BrightnessPercentage
    {
        return brightnessValue;
    }

    auto ScreenLightController::enableTimers() -> void
    {
        controlTimer.start();
        readoutTimer.start();
    }

    auto ScreenLightController::disableTimers() -> void
    {
        controlTimer.stop();
        readoutTimer.stop();
    }

    auto ScreenLightController::setParameters(const AutomaticModeParameters &params) -> void
    {
        if (lightOn && automaticMode == ScreenLightMode::Automatic) {
            disableTimers();
        }

        ::screen_light_control::functions::setRampStep(
            100.0f * (static_cast<float>(controlTimerIntervalMs) / static_cast<float>(params.rampTimeMS)));
        ::screen_light_control::functions::setHysteresis(params.brightnessHysteresis);
        ::screen_light_control::functions::setFunctionFromPoints(params.functionPoints);

        if (lightOn && automaticMode == ScreenLightMode::Automatic) {
            enableTimers();
        }
    }

    auto ScreenLightController::setParameters(ManualModeParameters params) -> void
    {
        brightnessValue = params.manualModeBrightness;
        setManualBrightnessLevel();
    }

    auto ScreenLightController::enableAutomaticMode() -> void
    {
        if (lightOn) {
            enableTimers();
        }
        automaticMode = ScreenLightMode::Automatic;
    }

    auto ScreenLightController::disableAutomaticMode() -> void
    {
        disableTimers();
        automaticMode = ScreenLightMode::Manual;
        setManualBrightnessLevel();
    }

    auto ScreenLightController::turnOn() -> void
    {
        fadeOut = false;

        bsp::eink_frontlight::turnOn();
        bsp::light_sensor::wakeup();
        lockCpuFrequency();

        if (automaticMode == ScreenLightMode::Automatic) {
            if (lightOn) {
                ::screen_light_control::functions::calculateBrightness(bsp::light_sensor::readout());
            }
            else {
                // It takes some time to get initial readout -> using last saved measurement
                ::screen_light_control::functions::calculateBrightness(stashedReadout);
            }
            ::screen_light_control::functions::resetRampToTarget();
            bsp::eink_frontlight::setBrightness(::screen_light_control::functions::getRampState());
            enableTimers();
        }
        lightOn = true;
    }

    auto ScreenLightController::setManualBrightnessLevel() -> void
    {
        bsp::eink_frontlight::setBrightness(brightnessValue);
    }

    auto ScreenLightController::turnOff() -> void
    {
        bsp::eink_frontlight::turnOff();
        stashedReadout = bsp::light_sensor::readout();
        disableTimers();
        bsp::light_sensor::standby();
        releaseCpuFrequency();
        lightOn = false;
        fadeOut = false;
    }

    auto ScreenLightController::handleFadeOut() -> void
    {
        if (automaticMode == ScreenLightMode::Automatic) {
            fadeOut = true;
            // Set fadeout brightness as maximum or current ramp state if lower
            const auto rampState         = ::screen_light_control::functions::getRampState();
            const auto fadeOutBrightness = std::clamp(rampState, 0.0f, fadeOutBrightnessMax);
            ::screen_light_control::functions::setRampTarget(fadeOutBrightness);
        }
    }

    auto ScreenLightController::isFadeOutOngoing() -> bool
    {
        return fadeOut;
    }

    auto ScreenLightController::lockCpuFrequency() -> void
    {
        cpuSentinel->HoldMinimumFrequency(bsp::CpuFrequencyMHz::Level_2);
    }

    auto ScreenLightController::releaseCpuFrequency() -> void
    {
        cpuSentinel->ReleaseMinimumFrequency();
    }
} // namespace pure::screen_light_control
