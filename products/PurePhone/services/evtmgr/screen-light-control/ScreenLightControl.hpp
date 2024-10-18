// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-evtmgr/screen-light-control/ScreenLightControl.hpp>
#include <service-evtmgr/screen-light-control/ScreenLightControlParameters.hpp>

#include <SystemManager/CpuSentinel.hpp>
#include <Timers/TimerHandle.hpp>

#include <memory>

namespace sys
{
    class Service;
} // namespace sys

/// Screen light control algorithm. Automatic/Manual mode of operation.
/// Processing of ambient light sensor input to screen brightness output.
namespace pure::screen_light_control
{
    /// Control screen light and keeps it's current state
    class ScreenLightController : public ::screen_light_control::ScreenLightController
    {
      public:
        using Action                       = ::screen_light_control::Action;
        using Parameters                   = ::screen_light_control::Parameters;
        using ScreenLightMode              = ::screen_light_control::ScreenLightMode;
        using ManualModeParameters         = ::screen_light_control::ManualModeParameters;
        using AutomaticModeParameters      = ::screen_light_control::AutomaticModeParameters;
        using LinearProgressModeParameters = ::screen_light_control::LinearProgressModeParameters;

        explicit ScreenLightController(sys::Service *parent);
        ~ScreenLightController() override;

        auto processRequest(Action action) -> void override;
        auto processRequest(Action action, const Parameters &params) -> void override;

        [[nodiscard]] auto isLightOn() const noexcept -> bool override;
        [[nodiscard]] auto isAutoModeOn() const noexcept -> bool override;
        [[nodiscard]] auto getBrightnessValue() const noexcept -> bsp::eink_frontlight::BrightnessPercentage override;

        [[nodiscard]] auto isFadeOutOngoing() -> bool override;

      private:
        auto controlTimerCallback() -> void;
        auto readoutTimerCallback() -> void;

        auto enableTimers() -> void;
        auto disableTimers() -> void;

        auto setParameters(const AutomaticModeParameters &params) -> void;
        auto setParameters(ManualModeParameters params) -> void;
        auto setManualBrightnessLevel() -> void;

        auto turnOff() -> void;
        auto turnOn() -> void;

        auto enableAutomaticMode() -> void;
        auto disableAutomaticMode() -> void;

        auto handleFadeOut() -> void;

        auto lockCpuFrequency() -> void;
        auto releaseCpuFrequency() -> void;

        static constexpr auto controlTimerIntervalMs = 25;
        static constexpr auto readoutTimerIntervalMs = 500;

        static constexpr auto lightControlTimerName = "LightControlTimer";
        static constexpr auto lightReadoutTimerName = "LightReadoutTimer";
        static constexpr auto cpuSentinelName       = "LightControl";

        std::shared_ptr<sys::CpuSentinel> cpuSentinel;
        sys::TimerHandle controlTimer;
        sys::TimerHandle readoutTimer;

        bool lightOn                                               = false;
        bool fadeOut                                               = false;
        ScreenLightMode automaticMode                              = ScreenLightMode::Manual;
        bsp::eink_frontlight::BrightnessPercentage brightnessValue = 0.0f;
        bsp::light_sensor::IlluminanceLux stashedReadout           = 0.0f;
    };
} // namespace pure::screen_light_control
