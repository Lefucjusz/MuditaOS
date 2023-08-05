// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/ApplicationCommon.hpp>
#include <module-db/Interface/MultimediaFilesRecord.hpp>
#include "SongsCache.hpp"

namespace app::music
{
    class SongsRepository : public app::AsyncCallbackReceiver
    {
      public:
        using OnGetMusicFilesListCallback =
            std::function<bool(const std::vector<db::multimedia_files::MultimediaFilesRecord> &, unsigned)>;
        using OnGetAlbumsListCallback =
            std::function<bool(const std::vector<db::multimedia_files::AlbumWithMetadata> &, unsigned)>;
        using OnGetArtistListCallback =
            std::function<bool(const std::vector<db::multimedia_files::ArtistWithMetadata> &, unsigned)>;

        SongsRepository(ApplicationCommon *application, std::unique_ptr<SongsCache> songsCache);

        auto getAllMusicFilesList(std::uint32_t offset,
                                  std::uint32_t limit,
                                  const OnGetMusicFilesListCallback &callback) -> void;

        auto getMusicFilesListForAlbum(std::uint32_t offset,
                                       std::uint32_t limit,
                                       const db::multimedia_files::Album &album,
                                       const OnGetMusicFilesListCallback &callback) -> void;

        auto getMusicFilesListForArtist(std::uint32_t offset,
                                        std::uint32_t limit,
                                        const db::multimedia_files::Artist &artist,
                                        const OnGetMusicFilesListCallback &callback) -> void;

        auto getMusicAlbumsWithMetadataList(std::uint32_t offset,
                                            std::uint32_t limit,
                                            const OnGetAlbumsListCallback &callback) -> void;

        auto getMusicArtistsWithMetadataList(std::uint32_t offset,
                                             std::uint32_t limit,
                                             const OnGetArtistListCallback &callback) -> void;

        auto initCache() -> void;
        auto initCache(const db::multimedia_files::Album &album) -> void;
        auto initCache(const db::multimedia_files::Artist &artist) -> void;

        auto getCurrentCacheAlbum() const noexcept -> std::optional<db::multimedia_files::Album>;
        auto getCurrentCacheArtist() const noexcept -> std::optional<db::multimedia_files::Artist>;

        auto refreshCache(const std::string &filePath, std::uint32_t currentOffset) -> void;

        auto getNextFilePath(const std::string &filePath) const -> std::string;
        auto getPreviousFilePath(const std::string &filePath) const -> std::string;

        auto getRecord(const std::string &filePath) const -> std::optional<db::multimedia_files::MultimediaFilesRecord>;

      private:
        app::ApplicationCommon *application = nullptr;
        std::unique_ptr<SongsCache> songsCache;
    };
} // namespace app::music
