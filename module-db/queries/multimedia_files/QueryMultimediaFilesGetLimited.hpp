// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "module-db/Interface/MultimediaFilesRecord.hpp"

#include <Common/Query.hpp>

#include <string>

namespace db::multimedia_files::query
{
    class GetLimited : public Query
    {
      public:
        GetLimited(std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetLimitedForArtist : public Query
    {
      public:
        GetLimitedForArtist(Artist artist, std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const Artist artist;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetLimitedForAlbum : public Query
    {
      public:
        GetLimitedForAlbum(Album album, std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const Album album;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetLimitedResult : public QueryResult
    {
        const std::vector<MultimediaFilesRecord> records;
        unsigned int dbRecordsCount;

      public:
        explicit GetLimitedResult(std::vector<MultimediaFilesRecord> records, unsigned int dbRecordsCount);
        [[nodiscard]] auto getResult() const -> std::vector<MultimediaFilesRecord>;
        [[nodiscard]] auto getCount() const noexcept -> unsigned int;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class GetArtistsLimited : public Query
    {
      public:
        GetArtistsLimited(std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetArtistsLimitedResult : public QueryResult
    {
        const std::vector<Artist> records;
        unsigned int dbRecordsCount;

      public:
        explicit GetArtistsLimitedResult(std::vector<Artist> records, unsigned int dbRecordsCount);
        [[nodiscard]] auto getResult() const -> std::vector<Artist>;
        [[nodiscard]] auto getCount() const noexcept -> unsigned int;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class GetArtistsWithMetadataLimited : public Query
    {
      public:
        GetArtistsWithMetadataLimited(std::uint32_t offset, std::uint32_t limit); // TODO std::uint32_t
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetArtistsWithMetadataLimitedResult : public QueryResult
    {
        const std::vector<ArtistWithMetadata> records;
        unsigned int dbRecordsCount;

      public:
        explicit GetArtistsWithMetadataLimitedResult(std::vector<ArtistWithMetadata> records,
                                                     unsigned int dbRecordsCount);
        [[nodiscard]] auto getResult() const -> std::vector<ArtistWithMetadata>;
        [[nodiscard]] auto getCount() const noexcept -> unsigned int;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class GetAlbumsLimited : public Query
    {
      public:
        GetAlbumsLimited(std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetAlbumsLimitedResult : public QueryResult
    {
        const std::vector<Album> records;
        unsigned int dbRecordsCount;

      public:
        explicit GetAlbumsLimitedResult(std::vector<Album> records, unsigned int dbRecordsCount);
        [[nodiscard]] auto getResult() const -> std::vector<Album>;
        [[nodiscard]] auto getCount() const noexcept -> unsigned int;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class GetAlbumsWithMetadataLimited : public Query
    {
      public:
        GetAlbumsWithMetadataLimited(std::uint32_t offset, std::uint32_t limit); // TODO std::uint32_t
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };

    class GetAlbumsWithMetadataLimitedResult : public QueryResult
    {
        const std::vector<AlbumWithMetadata> records;
        unsigned int dbRecordsCount;

      public:
        explicit GetAlbumsWithMetadataLimitedResult(std::vector<AlbumWithMetadata> records,
                                                    unsigned int dbRecordsCount);
        [[nodiscard]] auto getResult() const -> std::vector<AlbumWithMetadata>;
        [[nodiscard]] auto getCount() const noexcept -> unsigned int;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class GetLimitedByPaths : public Query
    {
      public:
        GetLimitedByPaths(const std::vector<std::string> &paths, std::uint32_t offset, std::uint32_t limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        const std::vector<std::string> paths;
        const std::uint32_t offset = 0;
        const std::uint32_t limit  = 0;
    };
} // namespace db::multimedia_files::query
