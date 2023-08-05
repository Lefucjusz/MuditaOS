// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsModel.hpp"
#include "Style.hpp"
#include "application-music-player/widgets/SongItem.hpp"
#include <ListView.hpp>
#include <time/time_conversion.hpp>

namespace app::music
{
    SongsModel::SongsModel(app::ApplicationCommon *app, std::shared_ptr<SongsRepository> songsRepository)
        : DatabaseModel(app), songsRepository(std::move(songsRepository))
    {}

    auto SongsModel::requestRecordsCount() -> unsigned
    {
        return recordsCount;
    }

    auto SongsModel::getMinimalItemSpaceRequired() const -> unsigned
    {
        return musicPlayerStyle::listItem::h + style::margins::small * 2;
    }

    auto SongsModel::requestRecords(std::uint32_t offset, std::uint32_t limit) -> void
    {
        currentListOffset = offset;

        /* All songs case */
        if (!currentlyViewedArtist.has_value() && !currentlyViewedAlbum.has_value()) {
            songsRepository->getAllMusicFilesList(offset, limit, [this](const auto &records, unsigned recordsCount) {
                return onMusicListRetrieved(records, recordsCount);
            });
            return;
        }

        /* Album case */
        if (currentlyViewedAlbum.has_value()) {
            songsRepository->getMusicFilesListForAlbum(
                offset, limit, currentlyViewedAlbum.value(), [this](const auto &records, unsigned recordsCount) {
                    return onMusicListRetrieved(records, recordsCount);
                });
            return;
        }

        /* Artist case */
        if (currentlyViewedArtist.has_value()) {
            songsRepository->getMusicFilesListForArtist(
                offset, limit, currentlyViewedArtist.value(), [this](const auto &records, unsigned recordsCount) {
                    return onMusicListRetrieved(records, recordsCount);
                });
            return;
        }

        LOG_ERROR("Unhandled view case!");
    }

    auto SongsModel::getItem(gui::Order order) -> gui::ListItem *
    {
        const auto song = getRecord(order);
        if (song == nullptr) {
            return nullptr;
        }

        auto item = new gui::SongItem(song->tags.album.artist,
                                      song->tags.title,
                                      utils::time::Duration(song->audioProperties.songLength).str(),
                                      navBarTemporaryMode,
                                      navBarRestoreFromTemporaryMode);

        if (songContext.filePath == song->fileInfo.path) {
            item->setState(songContext.isPlaying() ? gui::SongItem::ItemState::Playing
                                                   : (songContext.isPaused() ? gui::SongItem::ItemState::Paused
                                                                             : gui::SongItem::ItemState::None));
        }
        else {
            item->setState(gui::SongItem::ItemState::None);
        }

        item->activatedCallback = [this, song](gui::Item &) {
            if (shortReleaseCallback != nullptr) {
                activatedRecord = *song;
                shortReleaseCallback(song->fileInfo.path);
                return true;
            }
            return false;
        };

        item->inputCallback = [=](gui::Item &, const gui::InputEvent &event) {
            if (event.isLongRelease(gui::KeyCode::KEY_ENTER)) {
                if (longPressCallback != nullptr) {
                    longPressCallback();
                    return true;
                }
            }
            return false;
        };

        return item;
    }

    auto SongsModel::createData(const OnShortReleaseCallback &shortReleaseCb,
                                const OnLongPressCallback &longPressCb,
                                const OnSetNavBarTemporaryCallback &navBarTemporaryModeCb,
                                const OnRestoreNavBarTemporaryCallback &navBarRestoreFromTemporaryModeCb) -> void
    {
        this->shortReleaseCallback           = shortReleaseCb;
        this->longPressCallback              = longPressCb;
        this->navBarTemporaryMode            = navBarTemporaryModeCb;
        this->navBarRestoreFromTemporaryMode = navBarRestoreFromTemporaryModeCb;
    }

    auto SongsModel::setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) -> void
    {
        currentlyViewedArtist = std::nullopt;
        currentlyViewedAlbum  = album;
    }

    auto SongsModel::setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) -> void
    {
        currentlyViewedAlbum  = std::nullopt;
        currentlyViewedArtist = artist;
    }

    auto SongsModel::setAllSongsView() -> void
    {
        currentlyViewedAlbum  = std::nullopt;
        currentlyViewedArtist = std::nullopt;
    }

    auto SongsModel::checkIfViewedMatchPlayed() -> bool
    {
        /* All songs case */
        if (!currentlyViewedAlbum.has_value() && !currentlyViewedArtist.has_value() &&
            !songsRepository->getCurrentCacheAlbum().has_value() &&
            !songsRepository->getCurrentCacheArtist().has_value()) {
            return true;
        }

        /* Album songs case */
        const auto &cachedAlbum = songsRepository->getCurrentCacheAlbum();
        if (currentlyViewedAlbum.has_value() && cachedAlbum.has_value() &&
            currentlyViewedAlbum.value() == cachedAlbum.value()) {
            return true;
        }

        /* Artist songs case */
        const auto &cachedArtist = songsRepository->getCurrentCacheArtist();
        if (currentlyViewedArtist.has_value() && cachedArtist.has_value() &&
            currentlyViewedArtist.value() == cachedArtist.value()) {
            return true;
        }

        return false;
    }

    auto SongsModel::initRepository() -> void
    {
        if (checkIfViewedMatchPlayed()) {
            return;
        }

        if (currentlyViewedAlbum.has_value()) {
            songsRepository->initCache(currentlyViewedAlbum.value());
            return;
        }

        if (currentlyViewedArtist.has_value()) {
            songsRepository->initCache(currentlyViewedArtist.value());
            return;
        }

        songsRepository->initCache();
    }

    auto SongsModel::updateRepository(const std::string &filePath) -> void
    {
        songsRepository->refreshCache(filePath, currentListOffset);
    }

    auto SongsModel::isSongPlaying() const noexcept -> bool
    {
        return songContext.currentSongState == SongState::Playing;
    }

    auto SongsModel::setCurrentSongState(SongState songState) noexcept -> void
    {
        songContext.currentSongState = songState;
    }

    auto SongsModel::getCurrentFileToken() const noexcept -> std::optional<audio::Token>
    {
        return songContext.currentFileToken;
    }

    auto SongsModel::getActivatedRecord() const noexcept -> std::optional<db::multimedia_files::MultimediaFilesRecord>
    {
        return activatedRecord;
    }

    auto SongsModel::getNextFilePath(const std::string &filePath) const -> std::string
    {
        return songsRepository->getNextFilePath(filePath);
    }

    auto SongsModel::getPreviousFilePath(const std::string &filePath) const -> std::string
    {
        return songsRepository->getPreviousFilePath(filePath);
    }

    auto SongsModel::getCurrentSongContext() const noexcept -> SongContext
    {
        return songContext;
    }

    auto SongsModel::setCurrentSongContext(const SongContext &context) -> void
    {
        songContext = context;

        /* Dirty workaround for play operation callback vs. cache init callback
         * race condition, resulting in setting empty activated record when
         * playing first song after view has been changed. If currentRecord
         * is empty, activatedRecord was already set in item's activatedCallback,
         * so ignore this empty value. */
        const auto &currentRecord = songsRepository->getRecord(context.filePath);
        if (currentRecord.has_value()) {
            activatedRecord = currentRecord.value();
        }
    }

    auto SongsModel::clearCurrentSongContext() -> void
    {
        songContext.clear();
        activatedRecord = std::nullopt;
    }

    auto SongsModel::clearData() -> void
    {
        if (list != nullptr) {
            list->reset();
        }
    }

    auto SongsModel::updateRecords(std::vector<db::multimedia_files::MultimediaFilesRecord> records) -> bool
    {
        DatabaseModel::updateRecords(std::move(records));
        list->onProviderDataUpdate();
        return true;
    }

    auto SongsModel::onMusicListRetrieved(const std::vector<db::multimedia_files::MultimediaFilesRecord> &records,
                                          unsigned repoRecordsCount) -> bool
    {
        if (list != nullptr && recordsCount != repoRecordsCount) {
            recordsCount = repoRecordsCount;
            list->reSendLastRebuildRequest();
            return false;
        }
        return updateRecords(records);
    }
} // namespace app::music
