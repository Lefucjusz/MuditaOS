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
            virtual auto updateSongsState(std::optional<db::multimedia_files::MultimediaFilesRecord> record,
                                          RecordState state) -> void             = 0;
            virtual auto updateSongProgress(float progress) -> void              = 0;
            virtual auto refreshWindow() -> void                                 = 0;
            virtual auto setNavBarTemporaryMode(const std::string &text) -> void = 0;
            virtual auto restoreFromNavBarTemporaryMode() -> void                = 0;
        };

        class Presenter : public BasePresenter<SongsContract::View>
        {
          public:
            using OnPlayingStateChangeCallback = std::function<void(app::music::SongState)>;

            virtual ~Presenter() noexcept = default;

            virtual auto getModel() const -> std::shared_ptr<app::music::SongsModel> = 0;

            virtual auto setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) -> void    = 0;
            virtual auto setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) -> void = 0;
            virtual auto setAllSongsView() -> void                                                    = 0;

            virtual auto createData() -> void = 0;

            virtual auto play(const std::string &filePath) -> bool = 0;
            virtual auto pause() -> bool                           = 0;
            virtual auto resume() -> bool                          = 0;
            virtual auto stop() -> bool                            = 0;
            virtual auto playNext() -> bool                        = 0;
            virtual auto playPrevious() -> bool                    = 0;

            virtual auto songsStateRequest() -> void                                        = 0;
            virtual auto progressStateRequest() -> void                                     = 0;
            virtual auto setPlayingStateCallback(OnPlayingStateChangeCallback &&cb) -> void = 0;
            virtual auto handleAudioStopNotification(audio::Token token) -> bool            = 0;
            virtual auto handleAudioEofNotification(audio::Token token) -> bool             = 0;
            virtual auto handleAudioPausedNotification(audio::Token token) -> bool          = 0;
            virtual auto handleAudioResumedNotification(audio::Token token) -> bool         = 0;
            virtual auto handlePlayOrPauseRequest() -> bool                                 = 0;
        };
    };

    class SongsPresenter : public SongsContract::Presenter
    {
      public:
        SongsPresenter(app::ApplicationCommon *app,
                       std::shared_ptr<app::music::SongsModel> model,
                       std::unique_ptr<AbstractAudioOperations> &&audioOperations);

        [[nodiscard]] auto getModel() const -> std::shared_ptr<app::music::SongsModel> override;

        auto setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) -> void override;
        auto setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) -> void override;
        auto setAllSongsView() -> void override;

        auto createData() -> void override;

        auto play(const std::string &filePath) -> bool override;
        auto pause() -> bool override;
        auto resume() -> bool override;
        auto stop() -> bool override;
        auto playNext() -> bool override;
        auto playPrevious() -> bool override;

        auto songsStateRequest() -> void override;
        auto progressStateRequest() -> void override;
        auto setPlayingStateCallback(OnPlayingStateChangeCallback &&cb) -> void override;
        auto handleAudioStopNotification(audio::Token token) -> bool override;
        auto handleAudioEofNotification(audio::Token token) -> bool override;
        auto handleAudioPausedNotification(audio::Token token) -> bool override;
        auto handleAudioResumedNotification(audio::Token token) -> bool override;
        auto handlePlayOrPauseRequest() -> bool override;

      protected:
        auto handleTrackProgressTick() -> void;

      private:
        auto updateViewSongState() -> void;
        auto updateViewProgressState() -> void;
        auto refreshView() -> void;
        auto updateTrackProgressRatio() -> void;
        auto resetTrackProgressRatio() -> void;

        /// Request state dependent audio operation
        auto requestAudioOperation(const std::string &filePath = {}) -> bool;
        auto setViewNavBarTemporaryMode(const std::string &text) -> void;
        auto restoreViewNavBarFromTemporaryMode() -> void;

        auto playCallback(audio::RetCode retCode, const audio::Token &token, const std::string &filePath) -> void;
        auto pauseCallback(audio::RetCode retCode, const audio::Token &token) -> void;
        auto resumeCallback(audio::RetCode retCode, const audio::Token &token) -> void;
        auto stopCallback(audio::RetCode retCode, const audio::Token &token) -> void;

        auto startAutoStopTimer() -> void;
        auto stopAutoStopTimer() -> void;

        std::shared_ptr<app::music::SongsModel> songsModel;
        std::unique_ptr<AbstractAudioOperations> audioOperations;
        OnPlayingStateChangeCallback changePlayingStateCallback{nullptr};

        sys::TimerHandle autoStopTimer;

        sys::TimerHandle songProgressTimer;
        std::chrono::time_point<std::chrono::system_clock> songProgressTimestamp;
        std::chrono::milliseconds songMillisecondsElapsed{0};
        float currentProgressRatio{0.0f};

        bool waitingToPlay{false};
    };
} // namespace app::music_player
