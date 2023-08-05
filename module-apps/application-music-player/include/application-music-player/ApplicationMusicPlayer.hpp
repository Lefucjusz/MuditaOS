// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Application.hpp>
#include <Audio/decoder/Decoder.hpp>
#include <purefs/filesystem_paths.hpp>

namespace gui::name::window
{
    inline constexpr auto track_info = "Track Info";
    inline constexpr auto albums     = "Albums";
    inline constexpr auto artists    = "Artists";
    inline constexpr auto songs_list = "Songs List";
} // namespace gui::name::window

namespace app
{
    namespace music_player::internal
    {
        class MusicPlayerPriv;
    }

    inline constexpr auto name_music_player = "ApplicationMusicPlayer";

    class ApplicationMusicPlayer : public Application
    {

      public:
        explicit ApplicationMusicPlayer(std::string name                    = name_music_player,
                                        std::string parent                  = {},
                                        StatusIndicators statusIndicators   = StatusIndicators{},
                                        StartInBackground startInBackground = {false});
        ~ApplicationMusicPlayer() override;

        sys::MessagePointer DataReceivedHandler(sys::DataMessage *msgl,
                                                [[maybe_unused]] sys::ResponseMessage *resp) override;
        sys::ReturnCodes InitHandler() override;
        sys::ReturnCodes DeinitHandler() override;

        sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override
        {
            return sys::ReturnCodes::Success;
        }

        void createUserInterface() final;
        void destroyUserInterface() final;

      private:
        std::unique_ptr<music_player::internal::MusicPlayerPriv> priv;
    };

    template <>
    struct ManifestTraits<ApplicationMusicPlayer>
    {
        static auto GetManifest() -> manager::ApplicationManifest
        {
            return {{manager::actions::Launch,
                     manager::actions::PhoneModeChanged,
                     manager::actions::BluetoothModeChanged,
                     manager::actions::AlarmClockStatusChanged},
                    locks::AutoLockPolicy::DetermineByWindow};
        }
    };
} /* namespace app */
