// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <application-music-player/ApplicationMusicPlayer.hpp>

#include "AudioNotificationsHandler.hpp"

#include <windows/MusicPlayerMainWindow.hpp>
#include <windows/MusicPlayerAlbumsWindow.hpp>
#include <windows/MusicPlayerSongsListWindow.hpp>
#include <windows/MusicPlayerArtistsWindow.hpp>
#include <apps-common/AudioOperations.hpp>
#include <presenters/SongsPresenter.hpp>
#include <presenters/AlbumsPresenter.hpp>
#include <presenters/ArtistsPresenter.hpp>
#include <models/SongsRepository.hpp>
#include <models/SongsModel.hpp>
#include <models/AlbumsModel.hpp>
#include <models/ArtistsModel.hpp>
#include <service-appmgr/Controller.hpp>

#include <filesystem>
#include <log/log.hpp>
#include <i18n/i18n.hpp>
#include <purefs/filesystem_paths.hpp>
#include <service-audio/AudioServiceAPI.hpp>
#include <time/ScopedTime.hpp>

namespace app
{
    namespace music_player::internal
    {
        class MusicPlayerPriv
        {
          public:
            std::shared_ptr<app::music::SongsModel> songsModel;
            std::shared_ptr<app::music_player::SongsContract::Presenter> songsPresenter;

            std::shared_ptr<app::music::AlbumsModel> albumsModel;
            std::shared_ptr<app::music_player::AlbumsContract::Presenter> albumsPresenter;

            std::shared_ptr<app::music::ArtistsModel> artistsModel;
            std::shared_ptr<app::music_player::ArtistsContract::Presenter> artistsPresenter;

            std::shared_ptr<app::music::SongsRepository> songsRepository;
        };
    } // namespace music_player::internal

    constexpr auto applicationMusicPlayerStackSize = 1024 * 6;

    ApplicationMusicPlayer::ApplicationMusicPlayer(std::string name,
                                                   std::string parent,
                                                   StatusIndicators statusIndicators,
                                                   StartInBackground startInBackground)
        : Application(
              std::move(name), std::move(parent), statusIndicators, startInBackground, applicationMusicPlayerStackSize),
          priv{std::make_unique<music_player::internal::MusicPlayerPriv>()}
    {
        bus.channels.push_back(sys::BusChannel::ServiceAudioNotifications);

        const auto &paths = std::vector<std::string>{purefs::dir::getUserMediaPath()}; // TODO why here

        auto songsCache       = std::make_unique<app::music::SongsCache>(this);
        priv->songsRepository = std::make_shared<app::music::SongsRepository>(this, std::move(songsCache));

        auto audioOperations = std::make_unique<app::AsyncAudioOperations>(this);
        priv->songsModel     = std::make_unique<app::music::SongsModel>(this, priv->songsRepository);
        priv->songsPresenter =
            std::make_unique<app::music_player::SongsPresenter>(this, priv->songsModel, std::move(audioOperations));

        priv->albumsModel     = std::make_unique<app::music::AlbumsModel>(this, priv->songsRepository);
        priv->albumsPresenter = std::make_unique<app::music_player::AlbumsPresenter>(this, priv->albumsModel);

        priv->artistsModel     = std::make_unique<app::music::ArtistsModel>(this, priv->songsRepository);
        priv->artistsPresenter = std::make_unique<app::music_player::ArtistsPresenter>(this, priv->artistsModel);

        // callback used when playing state is changed
        auto autolockCallback = [this](app::music::SongState isPlaying) {
            if (isPlaying == app::music::SongState::Playing) {
                LOG_DEBUG("Preventing autolock while playing track.");
                lockPolicyHandler.set(locks::AutoLockPolicy::DetermineByAppState);
            }
            else {
                LOG_DEBUG("Autolock reenabled because track is no longer playing.");
                lockPolicyHandler.set(locks::AutoLockPolicy::DetermineByWindow);
                app::manager::Controller::preventBlockingDevice(this);
            }
        };
        priv->songsPresenter->setPlayingStateCallback(std::move(autolockCallback));

        // callback used when track is not played and we are in DetermineByAppState
        auto stateLockCallback = []() -> bool { return true; };
        lockPolicyHandler.setPreventsAutoLockByStateCallback(std::move(stateLockCallback));

        connect(typeid(AudioStopNotification), [&](sys::Message *msg) -> sys::MessagePointer {
            auto notification = static_cast<AudioStopNotification *>(msg);
            music_player::AudioNotificationsHandler audioNotificationHandler{priv->songsPresenter};
            return audioNotificationHandler.handleAudioStopNotification(notification);
        });
        connect(typeid(AudioEOFNotification), [&](sys::Message *msg) -> sys::MessagePointer {
            auto notification = static_cast<AudioStopNotification *>(msg);
            music_player::AudioNotificationsHandler audioNotificationHandler{priv->songsPresenter};
            return audioNotificationHandler.handleAudioEofNotification(notification);
        });
        connect(typeid(AudioPausedNotification), [&](sys::Message *msg) -> sys::MessagePointer {
            auto notification = static_cast<AudioPausedNotification *>(msg);
            music_player::AudioNotificationsHandler audioNotificationHandler{priv->songsPresenter};
            return audioNotificationHandler.handleAudioPausedNotification(notification);
        });
        connect(typeid(AudioResumedNotification), [&](sys::Message *msg) -> sys::MessagePointer {
            auto notification = static_cast<AudioResumedNotification *>(msg);
            music_player::AudioNotificationsHandler audioNotificationHandler{priv->songsPresenter};
            return audioNotificationHandler.handleAudioResumedNotification(notification);
        });
    }

    ApplicationMusicPlayer::~ApplicationMusicPlayer() = default;

    sys::MessagePointer ApplicationMusicPlayer::DataReceivedHandler(sys::DataMessage *msgl,
                                                                    [[maybe_unused]] sys::ResponseMessage *resp)
    {
        auto retMsg = Application::DataReceivedHandler(msgl);
        // if message was handled by application's template there is no need to process further.
        if (static_cast<sys::ResponseMessage *>(retMsg.get())->retCode == sys::ReturnCodes::Success) {
            return retMsg;
        }

        return handleAsyncResponse(resp);
    }

    // Invoked during initialization
    sys::ReturnCodes ApplicationMusicPlayer::InitHandler()
    {
        auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success) {
            return ret;
        }

        createUserInterface();
        return ret;
    }

    sys::ReturnCodes ApplicationMusicPlayer::DeinitHandler()
    {
        priv->songsPresenter->getModel()->clearData();
        priv->songsPresenter->stop();
        priv->albumsPresenter->getModel()->clearData();
        priv->artistsPresenter->getModel()->clearData();
        return Application::DeinitHandler();
    }

    void ApplicationMusicPlayer::createUserInterface()
    {
        windowsFactory.attach(gui::name::window::main_window, [&](ApplicationCommon *app, const std::string &name) {
            return std::make_unique<gui::MusicPlayerMainWindow>(app, priv->songsPresenter);
        });

        windowsFactory.attach(gui::name::window::albums, [&](ApplicationCommon *app, const std::string &name) {
            return std::make_unique<gui::MusicPlayerAlbumsWindow>(app, priv->albumsPresenter);
        });

        windowsFactory.attach(gui::name::window::artists, [&](ApplicationCommon *app, const std::string &name) {
            return std::make_unique<gui::MusicPlayerArtistsWindow>(app, priv->artistsPresenter);
        });

        windowsFactory.attach(gui::name::window::songs_list, [&](ApplicationCommon *app, const std::string &name) {
            return std::make_unique<gui::MusicPlayerSongsListWindow>(app, priv->songsPresenter);
        });

        attachPopups({gui::popup::ID::Volume,
                      gui::popup::ID::Tethering,
                      gui::popup::ID::BluetoothAuthenticate,
                      gui::popup::ID::PhoneModes,
                      gui::popup::ID::PhoneLock,
                      gui::popup::ID::SimLock,
                      gui::popup::ID::Alarm});
    }

    void ApplicationMusicPlayer::destroyUserInterface()
    {}
} /* namespace app */
