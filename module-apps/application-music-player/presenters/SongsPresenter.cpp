// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsPresenter.hpp"
#include <Timers/TimerFactory.hpp>
#include <algorithm>

namespace
{
    constexpr auto songProgressTimerInterval = std::chrono::seconds{1};
    constexpr auto songProgressTimerName     = "MusicPlayerSongProgress";
} // namespace

namespace app::music_player
{
    SongsPresenter::SongsPresenter(app::ApplicationCommon *app,
                                   std::shared_ptr<app::music::SongsModel> songsModel,
                                   std::unique_ptr<AbstractAudioOperations> &&audioOperations)
        : songsModel{std::move(songsModel)}, audioOperations{std::move(audioOperations)}
    {
        songProgressTimer = sys::TimerFactory::createPeriodicTimer(
            app, songProgressTimerName, songProgressTimerInterval, [this](sys::Timer &) { handleTrackProgressTick(); });
    }

    std::shared_ptr<app::music::SongsModel> SongsPresenter::getModel() const
    {
        return songsModel;
    }

    void SongsPresenter::setCurrentlyViewedAlbum(const db::multimedia_files::Album &album)
    {
        songsModel->setCurrentlyViewedAlbum(album);
    }

    void SongsPresenter::setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist)
    {
        songsModel->setCurrentlyViewedArtist(artist);
    }

    void SongsPresenter::setAllSongsView()
    {
        songsModel->setAllSongsView();
    }

    void SongsPresenter::createData()
    {
        songsModel->createData([this](const std::string &fileName) { return requestAudioOperation(fileName); },
                               [this]() { stop(); },
                               [this](const UTF8 &text) { setViewNavBarTemporaryMode(text); },
                               [this]() { restoreViewNavBarFromTemporaryMode(); });
        updateViewSongState();
    }

    bool SongsPresenter::play(const std::string &filePath)
    {
        waitingToPlay = true;
        const auto operationStatus =
            audioOperations->play(filePath, [this, filePath](audio::RetCode retCode, const audio::Token &token) {
                playCallback(retCode, token, filePath);
            });
        return operationStatus;
    }

    bool SongsPresenter::pause()
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        const auto operationStatus =
            audioOperations->pause(currentFileToken.value(), [this](audio::RetCode retCode, const audio::Token &token) {
                pauseCallback(retCode, token);
            });
        return operationStatus;
    }

    bool SongsPresenter::resume()
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        const auto operationStatus = audioOperations->resume(
            currentFileToken.value(),
            [this](audio::RetCode retCode, const audio::Token &token) { resumeCallback(retCode, token); });
        return operationStatus;
    }

    bool SongsPresenter::stop()
    {
        const auto &currentFileToken = songsModel->getCurrentFileToken();
        if (!currentFileToken.has_value()) {
            return false;
        }

        const auto operationStatus =
            audioOperations->stop(currentFileToken.value(), [this](audio::RetCode retCode, const audio::Token &token) {
                stopCallback(retCode, token);
            });
        return operationStatus;
    }

    bool SongsPresenter::playNext()
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        const auto &nextSongToPlay     = songsModel->getNextFilePath(currentSongContext.filePath);
        if (nextSongToPlay.empty()) {
            return false;
        }

        return play(nextSongToPlay);
    }

    bool SongsPresenter::playPrevious()
    {
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        const auto &prevSongToPlay     = songsModel->getPreviousFilePath(currentSongContext.filePath);
        if (prevSongToPlay.empty()) {
            return false;
        }

        return play(prevSongToPlay);
    }

    void SongsPresenter::songsStateRequest()
    {
        updateViewSongState();
    }

    void SongsPresenter::setPlayingStateCallback(std::function<void(app::music::SongState)> cb)
    {
        changePlayingStateCallback = std::move(cb);
    }

    bool SongsPresenter::handleAudioStopNotification(audio::Token token)
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

    bool SongsPresenter::handleAudioEofNotification(audio::Token token)
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

    bool SongsPresenter::handleAudioPausedNotification(audio::Token token)
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
        updateTrackProgressRatio();
        updateViewProgressState();
        refreshView();
        return true;
    }

    bool SongsPresenter::handleAudioResumedNotification(audio::Token token)
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
        refreshView();
        return true;
    }

    bool SongsPresenter::handlePlayOrPauseRequest()
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

    void SongsPresenter::updateTrackProgressRatio()
    {
        using namespace std::chrono;

        std::uint32_t secondsTotal = 0;
        if (songsModel->getActivatedRecord().has_value()) {
            secondsTotal = songsModel->getActivatedRecord()->audioProperties.songLength;
        }

        const auto now               = std::chrono::system_clock::now();
        const auto millisecondsToAdd = duration_cast<std::chrono::milliseconds>(now - songProgressTimestamp);
        songMillisecondsElapsed += millisecondsToAdd;
        songProgressTimestamp = now;

        if (secondsTotal == 0) {
            currentProgressRatio = 0.0f;
        }
        else {
            currentProgressRatio =
                static_cast<float>(duration_cast<std::chrono::seconds>(songMillisecondsElapsed).count()) /
                static_cast<float>(secondsTotal);
        }
        currentProgressRatio = std::clamp(currentProgressRatio, 0.0f, 1.0f);
    }

    void SongsPresenter::resetTrackProgressRatio()
    {
        currentProgressRatio    = 0.0f;
        songMillisecondsElapsed = std::chrono::milliseconds::zero();
        songProgressTimestamp   = std::chrono::system_clock::now();
        updateTrackProgressRatio();
    }

    void SongsPresenter::handleTrackProgressTick()
    {
        updateTrackProgressRatio();
        updateViewProgressState();
        refreshView();
    }

    bool SongsPresenter::requestAudioOperation(const std::string &filePath)
    {
        songsModel->initRepository();
        const auto &currentSongContext = songsModel->getCurrentSongContext();
        if (currentSongContext.isValid() && (filePath.empty() || currentSongContext.filePath == filePath)) {
            return currentSongContext.isPlaying() ? pause() : resume();
        }
        return play(filePath);
    }

    void SongsPresenter::setViewNavBarTemporaryMode(const std::string &text)
    {
        if (auto view = getView(); view != nullptr) {
            view->setNavBarTemporaryMode(text);
        }
    }

    void SongsPresenter::restoreViewNavBarFromTemporaryMode()
    {
        if (auto view = getView(); view != nullptr) {
            view->restoreFromNavBarTemporaryMode();
        }
    }

    void SongsPresenter::updateViewSongState()
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

    void SongsPresenter::updateViewProgressState()
    {
        if (auto view = getView(); view != nullptr) {
            view->updateSongProgress(currentProgressRatio);
        }
    }

    void SongsPresenter::refreshView()
    {
        if (auto view = getView(); view != nullptr) {
            view->refreshWindow();
        }
    }

    void SongsPresenter::playCallback(audio::RetCode retCode, const audio::Token &token, const std::string &filePath)
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

    void SongsPresenter::pauseCallback(audio::RetCode retCode, const audio::Token &token)
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

    void SongsPresenter::resumeCallback(audio::RetCode retCode, const audio::Token &token)
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

    void SongsPresenter::stopCallback([[maybe_unused]] audio::RetCode retCode,
                                      [[maybe_unused]] const audio::Token &token)
    {
        updateViewSongState();
        songProgressTimer.stop();
        resetTrackProgressRatio();
        updateViewProgressState();
        refreshView();
    }
} // namespace app::music_player
