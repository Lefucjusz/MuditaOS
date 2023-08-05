// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "QueryMultimediaFilesGetLimited.hpp"

namespace db::multimedia_files::query
{
    GetLimited::GetLimited(std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetLimited::debugInfo() const -> std::string
    {
        return std::string{"GetLimited"};
    }

    GetLimitedForArtist::GetLimitedForArtist(Artist artist, std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), artist(artist), offset(offset), limit(limit)
    {}

    auto GetLimitedForArtist::debugInfo() const -> std::string
    {
        return std::string{"GetLimitedForArtist"};
    }

    GetLimitedForAlbum::GetLimitedForAlbum(Album album, std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), album(album), offset(offset), limit(limit)
    {}

    auto GetLimitedForAlbum::debugInfo() const -> std::string
    {
        return std::string{"GetLimitedForAlbum"};
    }

    GetLimitedResult::GetLimitedResult(std::vector<MultimediaFilesRecord> records, unsigned int dbRecordsCount)
        : records(std::move(records)), dbRecordsCount{dbRecordsCount}
    {}

    auto GetLimitedResult::getResult() const -> std::vector<MultimediaFilesRecord>
    {
        return records;
    }

    auto GetLimitedResult::getCount() const noexcept -> unsigned int
    {
        return dbRecordsCount;
    }

    auto GetLimitedResult::debugInfo() const -> std::string
    {
        return std::string{"GetLimitedResult"};
    }

    GetArtistsLimited::GetArtistsLimited(std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetArtistsLimited::debugInfo() const -> std::string
    {
        return std::string{"GetArtistsLimited"};
    }

    GetArtistsLimitedResult::GetArtistsLimitedResult(std::vector<Artist> records, unsigned int dbRecordsCount)
        : records(std::move(records)), dbRecordsCount{dbRecordsCount}
    {}

    auto GetArtistsLimitedResult::getResult() const -> std::vector<Artist>
    {
        return records;
    }

    auto GetArtistsLimitedResult::getCount() const noexcept -> unsigned int
    {
        return dbRecordsCount;
    }

    auto GetArtistsLimitedResult::debugInfo() const -> std::string
    {
        return std::string{"GetArtistsLimitedResult"};
    }

    GetArtistsWithMetadataLimited::GetArtistsWithMetadataLimited(std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetArtistsWithMetadataLimited::debugInfo() const -> std::string
    {
        return std::string{"GetArtistsWithMetadataLimited"};
    }

    GetArtistsWithMetadataLimitedResult::GetArtistsWithMetadataLimitedResult(std::vector<ArtistWithMetadata> records,
                                                                             unsigned int dbRecordsCount)
        : records(std::move(records)), dbRecordsCount{dbRecordsCount}
    {}

    auto GetArtistsWithMetadataLimitedResult::getResult() const -> std::vector<ArtistWithMetadata>
    {
        return records;
    }

    auto GetArtistsWithMetadataLimitedResult::getCount() const noexcept -> unsigned int
    {
        return dbRecordsCount;
    }

    auto GetArtistsWithMetadataLimitedResult::debugInfo() const -> std::string
    {
        return std::string{"GetArtistsWithMetadataLimitedResult"};
    }

    GetAlbumsLimited::GetAlbumsLimited(std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetAlbumsLimited::debugInfo() const -> std::string
    {
        return std::string{"GetAlbumsLimited"};
    }

    GetAlbumsLimitedResult::GetAlbumsLimitedResult(std::vector<Album> records, unsigned int dbRecordsCount)
        : records(std::move(records)), dbRecordsCount{dbRecordsCount}
    {}

    auto GetAlbumsLimitedResult::getResult() const -> std::vector<Album>
    {
        return records;
    }

    auto GetAlbumsLimitedResult::getCount() const noexcept -> unsigned int
    {
        return dbRecordsCount;
    }

    auto GetAlbumsLimitedResult::debugInfo() const -> std::string
    {
        return std::string{"GetAlbumsLimitedResult"};
    }

    GetAlbumsWithMetadataLimited::GetAlbumsWithMetadataLimited(std::uint32_t offset, std::uint32_t limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetAlbumsWithMetadataLimited::debugInfo() const -> std::string
    {
        return std::string{"GetAlbumsWithMetadataLimited"};
    }

    GetAlbumsWithMetadataLimitedResult::GetAlbumsWithMetadataLimitedResult(std::vector<AlbumWithMetadata> records,
                                                                           unsigned int dbRecordsCount)
        : records(std::move(records)), dbRecordsCount{dbRecordsCount}
    {}

    auto GetAlbumsWithMetadataLimitedResult::getResult() const -> std::vector<AlbumWithMetadata>
    {
        return records;
    }

    auto GetAlbumsWithMetadataLimitedResult::getCount() const noexcept -> unsigned int
    {
        return dbRecordsCount;
    }

    auto GetAlbumsWithMetadataLimitedResult::debugInfo() const -> std::string
    {
        return std::string{"GetAlbumsWithMetadataLimitedResult"};
    }

    GetLimitedByPaths::GetLimitedByPaths(const std::vector<std::string> &paths,
                                         std::uint32_t offset,
                                         std::uint32_t limit)
        : Query(Query::Type::Read), paths{paths}, offset(offset), limit(limit)
    {}

    auto GetLimitedByPaths::debugInfo() const -> std::string
    {
        return std::string{"GetLimitedByPaths"};
    }
} // namespace db::multimedia_files::query
