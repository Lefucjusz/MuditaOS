// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/AudioOperations.hpp>
#include <apps-common/BasePresenter.hpp>
#include <models/SongsModel.hpp>

namespace app::music_player
{
    class SongsContract
    {
      public:
        class View
        {
          public:
            enum class RecordState
            {
                Playing,
                Paused,
                Stopped
            };

            virtual ~View() noexcept                                     = default;
            virtual void updateSongsState(std::optional<db::multimedia_files::MultimediaFilesRecord> record,
                                          RecordState state)             = 0;
            virtual void updateSongProgress(float progress)              = 0;
            virtual void refreshWindow()                                 = 0;
            virtual void setNavBarTemporaryMode(const std::string &text) = 0;
            virtual void restoreFromNavBarTemporaryMode()                = 0;
        };

        class Presenter : public BasePresenter<SongsContract::View>
        {
          public:
            using OnPlayingStateChangeCallback = std::function<void(app::music::SongState)>;

            virtual ~Presenter() noexcept = default;

            virtual std::shared_ptr<app::music::SongsModel> getModel() const             = 0;

            virtual void setCurrentlyViewedAlbum(const db::multimedia_files::Album &album)    = 0;
            virtual void setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) = 0;
            virtual void setAllSongsView()                                                    = 0;

            virtual void createData()                                                    = 0;

            virtual bool play(const std::string &filePath) = 0;
            virtual bool pause()                           = 0;
            virtual bool resume()                          = 0;
            virtual bool stop()                            = 0;
            virtual bool playNext()                        = 0;
            virtual bool playPrevious()                    = 0;

            virtual void songsStateRequest()                                      = 0;
            virtual void setPlayingStateCallback(OnPlayingStateChangeCallback cb) = 0;
            virtual bool handleAudioStopNotification(audio::Token token)          = 0;
            virtual bool handleAudioEofNotification(audio::Token token)           = 0;
            virtual bool handleAudioPausedNotification(audio::Token token)        = 0;
            virtual bool handleAudioResumedNotification(audio::Token token)       = 0;
            virtual bool handlePlayOrPauseRequest()                               = 0;
        };
    };

    class SongsPresenter : public SongsContract::Presenter
    {
      public:
        explicit SongsPresenter(app::ApplicationCommon *app,
                                std::shared_ptr<app::music::SongsModel> model,
                                std::unique_ptr<AbstractAudioOperations> &&audioOperations);

        std::shared_ptr<app::music::SongsModel> getModel() const override;

        void setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) override;
        void setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) override;
        void setAllSongsView() override;

        void createData() override;

        bool play(const std::string &filePath) override;
        bool pause() override;
        bool resume() override;
        bool stop() override;
        bool playNext() override;
        bool playPrevious() override;

        void songsStateRequest() override;
        void setPlayingStateCallback(std::function<void(app::music::SongState)> cb) override;
        bool handleAudioStopNotification(audio::Token token) override;
        bool handleAudioEofNotification(audio::Token token) override;
        bool handleAudioPausedNotification(audio::Token token) override;
        bool handleAudioResumedNotification(audio::Token token) override;
        bool handlePlayOrPauseRequest() override;

      protected:
        void handleTrackProgressTick();

      private:
        void updateViewSongState();
        void updateViewProgressState();
        void refreshView();
        void updateTrackProgressRatio();
        void resetTrackProgressRatio();

        /// Request state dependant audio operation
        bool requestAudioOperation(const std::string &filePath = "");
        void setViewNavBarTemporaryMode(const std::string &text);
        void restoreViewNavBarFromTemporaryMode();

        void playCallback(audio::RetCode retCode, const audio::Token &token, const std::string &filePath);
        void pauseCallback(audio::RetCode retCode, const audio::Token &token);
        void resumeCallback(audio::RetCode retCode, const audio::Token &token);
        void stopCallback(audio::RetCode retCode, const audio::Token &token);

        std::shared_ptr<app::music::SongsModel> songsModel;
        std::unique_ptr<AbstractAudioOperations> audioOperations;
        std::function<void(app::music::SongState)> changePlayingStateCallback = nullptr;

        sys::TimerHandle songProgressTimer;
        std::chrono::time_point<std::chrono::system_clock> songProgressTimestamp;
        std::chrono::milliseconds songMillisecondsElapsed{0};
        float currentProgressRatio = 0.0f;
        bool waitingToPlay         = false;
    };
} // namespace app::music_player
