// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsRepository.hpp"
#include <module-db/queries/multimedia_files/QueryMultimediaFilesGetLimited.hpp>

namespace app::music
{
    SongsRepository::SongsRepository(ApplicationCommon *application, std::unique_ptr<SongsCache> songsCache)
        : app::AsyncCallbackReceiver(application), application(application), songsCache(std::move(songsCache))
    {}

    auto SongsRepository::getAllMusicFilesList(std::uint32_t offset,
                                               std::uint32_t limit,
                                               const OnGetMusicFilesListCallback &callback) -> void
    {
        const auto taskCallback = [callback](const auto response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            if (callback != nullptr) {
                callback(result->getResult(), result->getCount());
            }
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimited>(offset, limit);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsRepository::getMusicFilesListForAlbum(std::uint32_t offset,
                                                    std::uint32_t limit,
                                                    const db::multimedia_files::Album &album,
                                                    const OnGetMusicFilesListCallback &callback) -> void
    {
        const auto taskCallback = [callback](const auto response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            if (callback != nullptr) {
                callback(result->getResult(), result->getCount());
            }
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimitedForAlbum>(album, offset, limit);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsRepository::getMusicFilesListForArtist(std::uint32_t offset,
                                                     std::uint32_t limit,
                                                     const db::multimedia_files::Artist &artist,
                                                     const OnGetMusicFilesListCallback &callback) -> void
    {
        const auto taskCallback = [callback](const auto response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            if (callback != nullptr) {
                callback(result->getResult(), result->getCount());
            }
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimitedForArtist>(artist, offset, limit);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsRepository::getMusicAlbumsWithMetadataList(std::uint32_t offset,
                                                         std::uint32_t limit,
                                                         const OnGetAlbumsListCallback &callback) -> void
    {
        const auto taskCallback = [callback](const auto response) {
            const auto result =
                dynamic_cast<db::multimedia_files::query::GetAlbumsWithMetadataLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            if (callback != nullptr) {
                callback(result->getResult(), result->getCount());
            }
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetAlbumsWithMetadataLimited>(offset, limit);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsRepository::getMusicArtistsWithMetadataList(std::uint32_t offset,
                                                          std::uint32_t limit,
                                                          const OnGetArtistListCallback &callback) -> void
    {
        const auto taskCallback = [callback](const auto response) {
            const auto result =
                dynamic_cast<db::multimedia_files::query::GetArtistsWithMetadataLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            if (callback != nullptr) {
                callback(result->getResult(), result->getCount());
            }
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetArtistsWithMetadataLimited>(offset, limit);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsRepository::initCache() -> void
    {
        songsCache->init();
    }

    auto SongsRepository::initCache(const db::multimedia_files::Album &album) -> void
    {
        songsCache->init(album);
    }

    auto SongsRepository::initCache(const db::multimedia_files::Artist &artist) -> void
    {
        songsCache->init(artist);
    }

    auto SongsRepository::getCurrentCacheAlbum() const noexcept -> std::optional<db::multimedia_files::Album>
    {
        return songsCache->getCurrentAlbum();
    }

    auto SongsRepository::getCurrentCacheArtist() const noexcept -> std::optional<db::multimedia_files::Artist>
    {
        return songsCache->getCurrentArtist();
    }

    auto SongsRepository::refreshCache(const std::string &filePath, std::uint32_t currentOffset) -> void
    {
        songsCache->update(filePath, currentOffset);
    }

    auto SongsRepository::getNextFilePath(const std::string &filePath) const -> std::string
    {
        return songsCache->getNextFilePath(filePath);
    }

    auto SongsRepository::getPreviousFilePath(const std::string &filePath) const -> std::string
    {
        return songsCache->getPreviousFilePath(filePath);
    }

    auto SongsRepository::getRecord(const std::string &filePath) const
        -> std::optional<db::multimedia_files::MultimediaFilesRecord>
    {
        return songsCache->getRecord(filePath);
    }
} // namespace app::music
