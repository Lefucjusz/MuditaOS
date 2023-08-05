// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsCache.hpp"
#include <module-db/queries/multimedia_files/QueryMultimediaFilesGetLimited.hpp>
#include <algorithm>

namespace
{
    constexpr std::uint32_t cacheMaxSize   = 30;
    constexpr std::uint32_t cacheThreshold = 10;
} // namespace

namespace app::music
{
    SongsCache::SongsCache(app::ApplicationCommon *app) : app::AsyncCallbackReceiver(app), application(app)
    {}

    auto SongsCache::init() -> void
    {
        clearMetadata();

        const auto taskCallback = [this](const auto &response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            initCacheCallback(result->getResult(), result->getCount());
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimited>(0, cacheMaxSize);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsCache::init(const db::multimedia_files::Album &album) -> void
    {
        setCurrentAlbum(album);

        const auto taskCallback = [this](const auto &response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            initCacheCallback(result->getResult(), result->getCount());
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimitedForAlbum>(album, 0, cacheMaxSize);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsCache::init(const db::multimedia_files::Artist &artist) -> void
    {
        setCurrentArtist(artist);

        const auto taskCallback = [this](const auto &response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            initCacheCallback(result->getResult(), result->getCount());
            return true;
        };

        auto query = std::make_unique<db::multimedia_files::query::GetLimitedForArtist>(artist, 0, cacheMaxSize);
        auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsCache::update(const std::string &currentFilePath, std::uint32_t currentOffset) -> void
    {
        const auto currentIndex = getCachedFileIndex(currentFilePath);

        /* No valid data in cache at all - refresh completely */
        if (currentIndex == std::numeric_limits<std::size_t>::max()) {
            updateCache(calculateQueryOffset(currentOffset),
                        cacheMaxSize,
                        [this](const auto &newRecords, unsigned newRecordsCount, std::uint32_t offset) {
                            newDataCallback(newRecords, newRecordsCount, offset);
                        });
            return;
        }

        /* Reached back threshold - append data */
        if (currentIndex > (records.size() - cacheThreshold)) {
            updateCache(recordsOffset + records.size() + 1,
                        cacheThreshold,
                        [this](const auto &newRecords, unsigned newRecordsCount, std::uint32_t offset) {
                            newBackDataCallback(newRecords, newRecordsCount, offset);
                        });
            return;
        }

        /* Reached front threshold - prepend data */
        if (currentIndex < cacheThreshold) {
            const auto offset = (recordsOffset > cacheThreshold) ? (recordsOffset - cacheThreshold) : 0;
            const auto limit  = std::min(cacheThreshold, recordsOffset);

            updateCache(offset, limit, [this](const auto &newRecords, unsigned newRecordsCount, std::uint32_t offset) {
                newFrontDataCallback(newRecords, newRecordsCount, offset);
            });
            return;
        }
    }

    auto SongsCache::getCachedFileIndex(const std::string &filePath) const -> std::size_t
    {
        const auto it = std::find_if(records.begin(), records.end(), [&filePath](const auto &record) {
            return record.fileInfo.path == filePath;
        });

        if (it != records.end()) {
            return std::distance(records.begin(), it);
        }

        return std::numeric_limits<std::size_t>::max();
    }

    auto SongsCache::getPreviousFilePath(const std::string &currentFilePath) const -> std::string
    {
        const auto currentIndex = getCachedFileIndex(currentFilePath);
        if ((currentIndex == std::numeric_limits<size_t>::max()) || (currentIndex == 0)) {
            return "";
        }

        return records[currentIndex - 1].fileInfo.path;
    }

    auto SongsCache::getNextFilePath(const std::string &currentFilePath) const -> std::string
    {
        const auto currentIndex = getCachedFileIndex(currentFilePath);
        if ((currentIndex == std::numeric_limits<std::size_t>::max()) || (currentIndex >= records.size() - 1)) {
            return "";
        }

        return records[currentIndex + 1].fileInfo.path;
    }

    auto SongsCache::getRecord(const std::string &filePath) const
        -> std::optional<db::multimedia_files::MultimediaFilesRecord>
    {
        if (const auto index = getCachedFileIndex(filePath); index != std::numeric_limits<std::size_t>::max()) {
            return records[index];
        }
        return std::nullopt;
    }

    auto SongsCache::getCurrentAlbum() const noexcept -> std::optional<db::multimedia_files::Album>
    {
        return currentAlbum;
    }

    auto SongsCache::getCurrentArtist() const noexcept -> std::optional<db::multimedia_files::Artist>
    {
        return currentArtist;
    }

    auto SongsCache::setCurrentAlbum(const db::multimedia_files::Album &album) -> void
    {
        currentAlbum  = album;
        currentArtist = std::nullopt;
    }

    auto SongsCache::setCurrentArtist(const db::multimedia_files::Artist &artist) -> void
    {
        currentArtist = artist;
        currentAlbum  = std::nullopt;
    }

    auto SongsCache::clearMetadata() -> void
    {
        currentArtist = std::nullopt;
        currentAlbum  = std::nullopt;
    }

    auto SongsCache::updateCache(std::uint32_t offset, std::uint32_t limit, const OnUpdateCacheCallback &callback)
        -> void
    {
        std::unique_ptr<AsyncQuery> task;

        if (!currentAlbum.has_value() && !currentArtist.has_value()) {
            auto query = std::make_unique<db::multimedia_files::query::GetLimited>(offset, limit);
            task       = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        }
        else if (currentAlbum.has_value()) {
            auto query =
                std::make_unique<db::multimedia_files::query::GetLimitedForAlbum>(currentAlbum.value(), offset, limit);
            task = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        }
        else if (currentArtist.has_value()) {
            auto query = std::make_unique<db::multimedia_files::query::GetLimitedForArtist>(
                currentArtist.value(), offset, limit);
            task = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::MultimediaFiles);
        }
        else {
            LOG_ERROR("Unhandled case when updating cache!");
            return;
        }

        const auto taskCallback = [offset, callback](const auto &response) {
            const auto result = dynamic_cast<db::multimedia_files::query::GetLimitedResult *>(response);
            if (result == nullptr) {
                return false;
            }

            callback(result->getResult(), result->getCount(), offset);
            return true;
        };

        task->setCallback(taskCallback);
        task->execute(application, this);
    }

    auto SongsCache::calculateQueryOffset(std::uint32_t viewOffset) const noexcept -> std::uint32_t
    {
        /* All records fit in cache */
        if (recordsCount < cacheMaxSize) {
            return 0;
        }

        /* Threshold not reached */
        if (viewOffset < cacheThreshold) {
            return 0;
        }

        /* "Saturation" - all records from offset to recordsCount fit in cache */
        if (viewOffset > (recordsCount - (cacheMaxSize - cacheThreshold))) {
            return recordsCount - cacheMaxSize;
        }

        /* Progressively fetch new entries */
        return viewOffset - cacheThreshold;
    }

    auto SongsCache::initCacheCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                       unsigned newRecordsCount) -> void
    {
        records.clear();
        std::copy(newRecords.begin(), newRecords.end(), std::back_inserter(records));
        recordsOffset = 0;
        recordsCount  = newRecordsCount;
    }

    auto SongsCache::newDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                     unsigned newRecordsCount,
                                     std::uint32_t offset) -> void
    {
        records.clear();
        std::copy(newRecords.begin(), newRecords.end(), std::back_inserter(records));
        recordsOffset = offset;
        recordsCount  = newRecordsCount;
    }

    auto SongsCache::newBackDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                         unsigned newRecordsCount,
                                         [[maybe_unused]] std::uint32_t offset) -> void
    {
        for (const auto &record : newRecords) {
            records.push_back(record);
            records.pop_front();
        }
        recordsOffset += newRecords.size();
        recordsCount = newRecordsCount;
    }

    auto SongsCache::newFrontDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                          unsigned newRecordsCount,
                                          std::uint32_t offset) -> void
    {
        for (auto it = newRecords.rbegin(); it != newRecords.rend(); ++it) {
            records.push_front(*it);
            records.pop_back();
        }
        recordsOffset = offset;
        recordsCount  = newRecordsCount;
    }
} // namespace app::music
