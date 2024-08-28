// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsPresenter.hpp"
#include <Timers/TimerFactory.hpp>
#include <algorithm>

namespace
{
    constexpr auto songProgressTimerInterval = std::chrono::seconds{1};
    constexpr auto songProgressTimerName     = "MusicPlayerSongProgress";

    constexpr auto autoStopTimerDelay = std::chrono::minutes{30};
    constexpr auto autoStopTimerName  = "MusicPlayerAutoStop";
} // namespace

namespace app::music_player
{
    SongsPresenter::SongsPresenter(app::ApplicationCommon *app,
                                   std::shared_ptr<app::music::SongsModel> songsModel,
                                   std::unique_ptr<AbstractAudioOperations> &&audioOperations)
        : songsModel{std::move(songsModel)}, audioOperations{std::move(audioOperations)}
    {
        songProgressTimer = sys::TimerFactory::createPeriodicTimer(
            app, songProgressTimerName, songProgressTimerInterval, [this]([[maybe_unused]] sys::Timer &t) {
                handleTrackProgressTick();
            });

        autoStopTimer = sys::TimerFactory::createSingleShotTimer(
            app, autoStopTimerName, autoStopTimerDelay, [this]([[maybe_unused]] sys::Timer &t) {
                LOG_INFO("Stopping paused playback after %" PRIi64 " minutes of inactivity",
                         autoStopTimerDelay.count());
                stop();
            });
    }

    auto SongsPresenter::getModel() const -> std::shared_ptr<app::music::SongsModel>
    {
        return songsModel;
    }

    auto SongsPresenter::setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) -> void
    {
        songsModel->setCurrentlyViewedAlbum(album);
    }

    auto SongsPresenter::setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) -> void
    {
        songsModel->setCurrentlyViewedArtist(artist);
    }

    auto SongsPresenter::setAllSongsView() -> void
    {
        songsModel->setAllSongsView();
    }

    auto SongsPresenter::createData() -> void
    {
        songsModel->createData([this](const std::string &fileName) { return requestAudioOperation(fileName); },
                               [this]() { stop(); },
                               [this](const UTF8 &text) { setViewNavBarTemporaryMode(text); },
                               [this]() { restoreViewNavBarFromTemporaryMode(); });
        updateViewSongState();
    }

    auto SongsPresenter::play(const std::string &filePath) -> bool
    {
        waitingToPlay = true;
        stopAutoStopTimer();
        const auto operationStatus =
            audioOperations->play(filePath, [this, filePath](audio::RetCode retCode, const audio::Token &token) {
                playCallback(retCode, token, filePath);
            });
        return operationStatus;
    }

    auto SongsPresenter::pause() -> bool
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        startAutoStopTimer();
        const auto operationStatus =
            audioOperations->pause(currentFileToken.value(), [this](audio::RetCode retCode, const audio::Token &token) {
                pauseCallback(retCode, token);
            });
        return operationStatus;
    }

    auto SongsPresenter::resume() -> bool
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        stopAutoStopTimer();
        const auto operationStatus = audioOperations->resume(
            currentFileToken.value(),
            [this](audio::RetCode retCode, const audio::Token &token) { resumeCallback(retCode, token); });
        return operationStatus;
    }

    auto SongsPresenter::stop() -> bool
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        stopAutoStopTimer();
        const auto operationStatus =
            audioOperations->stop(currentFileToken.value(), [this](audio::RetCode retCode, const audio::Token &token) {
                stopCallback(retCode, token);
            });
        return operationStatus;
    }

    auto SongsPresenter::playNext() -> bool
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        const auto &nextSongToPlay     = songsModel->getNextFilePath(currentSongContext.filePath);
        if (nextSongToPlay.empty()) {
            return false;
        }

        return play(nextSongToPlay);
    }

    auto SongsPresenter::playPrevious() -> bool
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        const auto &prevSongToPlay     = songsModel->getPreviousFilePath(currentSongContext.filePath);
        if (prevSongToPlay.empty()) {
            return false;
        }

        return play(prevSongToPlay);
    }

    auto SongsPresenter::songsStateRequest() -> void
    {
        updateViewSongState();
    }

    auto SongsPresenter::progressStateRequest() -> void
    {
        updateViewProgressState();
    }

    auto SongsPresenter::setPlayingStateCallback(OnPlayingStateChangeCallback &&cb) -> void
    {
        changePlayingStateCallback = std::move(cb);
    }

    auto SongsPresenter::handleAudioStopNotification(audio::Token token) -> bool
    {
        if (token != songsModel->getCurrentFileToken()) {
            return false;
        }

        songsModel->clearCurrentSongContext();
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::NotPlaying);
        }
        if (!waitingToPlay) {
            updateViewSongState();
            refreshView();
        }
        return true;
    }

    auto SongsPresenter::handleAudioEofNotification(audio::Token token) -> bool
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        if (token != currentSongContext.currentFileToken) {
            return false;
        }

        songsModel->clearCurrentSongContext();
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::NotPlaying);
        }
        const auto &nextSongToPlay = songsModel->getNextFilePath(currentSongContext.filePath);
        if (!nextSongToPlay.empty()) {
            requestAudioOperation(nextSongToPlay);
        }
        else {
            updateViewSongState();
            refreshView();
        }
        return true;
    }

    auto SongsPresenter::handleAudioPausedNotification(audio::Token token) -> bool
    {
        if (token != songsModel->getCurrentFileToken()) {
            return false;
        }

        songsModel->setCurrentSongState(app::music::SongState::NotPlaying);
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::NotPlaying);
        }
        updateViewSongState();
        songProgressTimer.stop();
        startAutoStopTimer();
        updateTrackProgressRatio();
        updateViewProgressState();
        refreshView();
        return true;
    }

    auto SongsPresenter::handleAudioResumedNotification(audio::Token token) -> bool
    {
        if (token != songsModel->getCurrentFileToken()) {
            return false;
        }

        songsModel->setCurrentSongState(app::music::SongState::Playing);
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::Playing);
        }
        updateViewSongState();
        songProgressTimestamp = std::chrono::system_clock::now();
        songProgressTimer.start();
        stopAutoStopTimer();
        refreshView();
        return true;
    }

    auto SongsPresenter::handlePlayOrPauseRequest() -> bool
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();

        if (currentSongContext.isPaused()) {
            return resume();
        }

        if (currentSongContext.isPlaying()) {
            return pause();
        }

        return false;
    }

    auto SongsPresenter::updateTrackProgressRatio() -> void
    {
        std::uint32_t secondsTotal = 0;
        const auto &activatedRecord = songsModel->getActivatedRecord();
        if (activatedRecord.has_value()) {
            secondsTotal = activatedRecord->audioProperties.songLength;
        }

        const auto now = std::chrono::system_clock::now();
        const auto millisecondsToAdd =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - songProgressTimestamp);
        songMillisecondsElapsed += millisecondsToAdd;
        songProgressTimestamp = now;

        if (secondsTotal == 0) {
            currentProgressRatio = 0.0f;
        }
        else {
            currentProgressRatio =
                static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(songMillisecondsElapsed).count()) /
                static_cast<float>(secondsTotal);
        }
        currentProgressRatio = std::clamp(currentProgressRatio, 0.0f, 1.0f);
    }

    auto SongsPresenter::resetTrackProgressRatio() -> void
    {
        currentProgressRatio    = 0.0f;
        songMillisecondsElapsed = std::chrono::milliseconds::zero();
        songProgressTimestamp   = std::chrono::system_clock::now();
        updateTrackProgressRatio();
    }

    auto SongsPresenter::handleTrackProgressTick() -> void
    {
        updateTrackProgressRatio();
        updateViewProgressState();
        refreshView();
    }

    auto SongsPresenter::requestAudioOperation(const std::string &filePath) -> bool
    {
        songsModel->initRepository();
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        if (currentSongContext.isValid() && (filePath.empty() || currentSongContext.filePath == filePath)) {
            return currentSongContext.isPlaying() ? pause() : resume();
        }
        return play(filePath);
    }

    auto SongsPresenter::setViewNavBarTemporaryMode(const std::string &text) -> void
    {
        if (auto view = getView(); view != nullptr) {
            view->setNavBarTemporaryMode(text);
        }
    }

    auto SongsPresenter::restoreViewNavBarFromTemporaryMode() -> void
    {
        if (auto view = getView(); view != nullptr) {
            view->restoreFromNavBarTemporaryMode();
        }
    }

    auto SongsPresenter::updateViewSongState() -> void
    {
        if (auto view = getView(); view != nullptr) {
            const auto &currentSongContext = songsModel->getCurrentSongContext();
            const auto &currentRecord      = songsModel->getActivatedRecord();
            view->updateSongsState(currentRecord,
                                   currentSongContext.isPlaying()
                                       ? SongsContract::View::RecordState::Playing
                                       : (currentSongContext.isPaused() ? SongsContract::View::RecordState::Paused
                                                                        : SongsContract::View::RecordState::Stopped));
        }
    }

    auto SongsPresenter::updateViewProgressState() -> void
    {
        if (auto view = getView(); view != nullptr) {
            view->updateSongProgress(currentProgressRatio);
        }
    }

    auto SongsPresenter::refreshView() -> void
    {
        if (auto view = getView(); view != nullptr) {
            view->refreshWindow();
        }
    }

    auto SongsPresenter::playCallback(audio::RetCode retCode, const audio::Token &token, const std::string &filePath)
        -> void
    {
        waitingToPlay = false;
        if (retCode != audio::RetCode::Success || !token.IsValid()) {
            LOG_ERROR("Playback audio operation failed, retcode = %s, token validity = %d",
                      str(retCode).c_str(),
                      token.IsValid());
            refreshView();

            const auto &nextSongToPlay = songsModel->getNextFilePath(filePath);
            if (nextSongToPlay.empty()) {
                return;
            }

            play(nextSongToPlay);
            return;
        }

        songsModel->setCurrentSongContext(
            {app::music::SongState::Playing, token, filePath, app::music::SongContext::StartPos});
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::Playing);
        }
        updateViewSongState();
        songsModel->updateRepository(filePath);
        resetTrackProgressRatio();
        songProgressTimer.start();
        updateViewProgressState();
        refreshView();
    }

    auto SongsPresenter::pauseCallback(audio::RetCode retCode, const audio::Token &token) -> void
    {
        if (retCode != audio::RetCode::Success || !token.IsValid()) {
            LOG_ERROR("Pause audio operation failed, retcode = %s, token validity = %d",
                      str(retCode).c_str(),
                      token.IsValid());
            return;
        }

        if (token != songsModel->getCurrentFileToken()) {
            LOG_ERROR("Pause audio operation failed, wrong token");
            return;
        }

        songsModel->setCurrentSongState(app::music::SongState::NotPlaying);
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::NotPlaying);
        }
        updateViewSongState();
        songProgressTimer.stop();
        updateTrackProgressRatio();
        updateViewProgressState();
        refreshView();
    }

    auto SongsPresenter::resumeCallback(audio::RetCode retCode, const audio::Token &token) -> void
    {
        if (retCode != audio::RetCode::Success || !token.IsValid()) {
            LOG_ERROR("Resume audio operation failed, retcode = %s, token validity = %d",
                      str(retCode).c_str(),
                      token.IsValid());
            return;
        }

        if (token != songsModel->getCurrentFileToken()) {
            LOG_ERROR("Resume audio operation failed, wrong token");
            return;
        }

        songsModel->setCurrentSongState(app::music::SongState::Playing);
        if (changePlayingStateCallback != nullptr) {
            changePlayingStateCallback(app::music::SongState::Playing);
        }
        updateViewSongState();
        songProgressTimestamp = std::chrono::system_clock::now();
        songProgressTimer.start();
        refreshView();
    }

    auto SongsPresenter::stopCallback([[maybe_unused]] audio::RetCode retCode,
                                      [[maybe_unused]] const audio::Token &token) -> void
    {
        updateViewSongState();
        songProgressTimer.stop();
        resetTrackProgressRatio();
        updateViewProgressState();
        refreshView();
    }

    auto SongsPresenter::startAutoStopTimer() -> void
    {
        autoStopTimer.restart(autoStopTimerDelay);
    }

    auto SongsPresenter::stopAutoStopTimer() -> void
    {
        autoStopTimer.stop();
    }
} // namespace app::music_player
