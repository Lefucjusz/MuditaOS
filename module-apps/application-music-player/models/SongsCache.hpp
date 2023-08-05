// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <cstdint>
#include <cstddef>
#include <module-db/Interface/MultimediaFilesRecord.hpp>
#include <apps-common/ApplicationCommon.hpp>

namespace app::music // TODO add AbstractSongsCache?
{
    class SongsCache : public app::AsyncCallbackReceiver
    {
      public:
        using OnUpdateCacheCallback = std::function<void(
            const std::vector<db::multimedia_files::MultimediaFilesRecord> &, unsigned, std::uint32_t)>;

        explicit SongsCache(app::ApplicationCommon *app);

        auto init() -> void;
        auto init(const db::multimedia_files::Album &album) -> void;
        auto init(const db::multimedia_files::Artist &artist) -> void;

        auto update(const std::string &currentFilePath, std::uint32_t currentOffset) -> void;

        auto getCachedFileIndex(const std::string &filePath) const -> std::size_t;
        auto getPreviousFilePath(const std::string &currentFilePath) const -> std::string;
        auto getNextFilePath(const std::string &currentFilePath) const -> std::string;
        auto getRecord(const std::string &filePath) const -> std::optional<db::multimedia_files::MultimediaFilesRecord>;

        auto getCurrentAlbum() const noexcept -> std::optional<db::multimedia_files::Album>;
        auto getCurrentArtist() const noexcept -> std::optional<db::multimedia_files::Artist>;

      private:
        auto setCurrentAlbum(const db::multimedia_files::Album &album) -> void;
        auto setCurrentArtist(const db::multimedia_files::Artist &artist) -> void;
        auto clearMetadata() -> void;

        auto updateCache(std::uint32_t offset, std::uint32_t limit, const OnUpdateCacheCallback &callback) -> void;
        auto calculateQueryOffset(std::uint32_t viewOffset) const noexcept -> std::uint32_t;

        auto initCacheCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                               unsigned newRecordsCount) -> void;
        auto newDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                             unsigned newRecordsCount,
                             std::uint32_t offset) -> void;
        auto newBackDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                 unsigned newRecordsCount,
                                 std::uint32_t offset) -> void;
        auto newFrontDataCallback(const std::vector<db::multimedia_files::MultimediaFilesRecord> &newRecords,
                                  unsigned newRecordsCount,
                                  std::uint32_t offset) -> void;

        std::deque<db::multimedia_files::MultimediaFilesRecord> records;
        std::uint32_t recordsOffset = 0;
        std::uint32_t recordsCount  = 0;

        std::optional<db::multimedia_files::Album> currentAlbum;
        std::optional<db::multimedia_files::Artist> currentArtist;

        app::ApplicationCommon *application = nullptr;
    };
} // namespace app::music
