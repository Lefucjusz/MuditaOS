// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-evtmgr/EventManager.hpp>

#include <backlight-handler/BacklightHandler.hpp>
#include <bsp/vibrator/vibrator.hpp>
#include <vibra/Vibra.hpp>

class PureEventManager : public EventManager
{
  public:
    explicit PureEventManager(const std::string &name = service::name::evt_manager)
        : EventManager(name), Vibra(std::make_unique<vibra_handle::Vibra>(this)), backlightHandler(settings, this)
    {}

  private:
    sys::ReturnCodes InitHandler() override;
    sys::MessagePointer DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp) override;
    void toggleTorchOnOff();
    void toggleTorchColor();
    void handleKeyEvent(sys::Message *msg) override;
    void handleKeyMoveEvent(RawKey key);

    bool processVibraRequest(bsp::vibrator::Action act,
                             std::chrono::milliseconds RepetitionTime = std::chrono::milliseconds{1000});

    std::unique_ptr<vibra_handle::Vibra> Vibra;
    backlight::Handler backlightHandler;
};

namespace sys
{
    template <> struct ManifestTraits<PureEventManager>
    {
        static auto GetManifest() -> ServiceManifest
        {
            ServiceManifest manifest;
            manifest.name         = service::name::evt_manager;
            manifest.dependencies = {service::name::db};
            return manifest;
        }
    };
} // namespace sys