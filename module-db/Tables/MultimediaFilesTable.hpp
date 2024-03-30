// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Record.hpp"
#include "Table.hpp"
#include <Database/Database.hpp>

#include <string>

namespace db::multimedia_files
{
    using Artist = std::string;

    struct ArtistWithMetadata
    {
        Artist artist{};
        std::uint32_t songsCount;
        std::uint32_t totalLength; /// in seconds
    };

    struct Album
    {
        Artist artist{};
        std::string title{};

        inline bool operator==(const Album &rhs) const
        {
            return ((this->artist == rhs.artist) && (this->title == rhs.title));
        }
    };

    struct AlbumWithMetadata
    {
        Artist artist{};
        std::string title{};
        std::uint32_t songsCount;
        std::uint32_t totalLength; /// in seconds
    };

    struct Tags
    {
        std::string title{};
        Album album{};
        std::string comment{};
        std::string genre{};
        std::uint32_t year{};
        std::uint32_t track{};
    };

    struct AudioProperties
    {
        std::uint32_t songLength{}; /// in seconds
        std::uint32_t bitrate{};    /// in kb/s
        std::uint32_t sampleRate{}; /// in Hz
        std::uint32_t channels{};   /// 1 - mono, 2 - stereo
    };

    struct FileInfo
    {
        std::string path{};
        std::string mediaType{}; /// mime type e.g. "audio/mp3"
        std::size_t size{};      /// in bytes
    };

    struct TableRow : public Record
    {
        FileInfo fileInfo{};
        Tags tags{};
        AudioProperties audioProperties{};

        auto isValid() const -> bool
        {
            return (!fileInfo.path.empty() && Record::isValid());
        }
    };

    enum class TableFields
    {
        path,
        media_type,
        size,
        title,
        artist,
        album,
        comment,
        genre,
        year,
        track,
        song_length,
        bitrate,
        sample_rate,
        channels
    };

    class MultimediaFilesTable : public Table<TableRow, TableFields>
    {
      public:
        explicit MultimediaFilesTable(Database *db);
        virtual ~MultimediaFilesTable() = default;

        auto create() -> bool override;
        auto add(TableRow entry) -> bool override;
        auto removeById(std::uint32_t id) -> bool override;
        auto removeByField(TableFields field, const char *str) -> bool override;
        auto removeAll() -> bool override final;
        auto update(TableRow entry) -> bool override;
        auto getById(std::uint32_t id) -> TableRow override;
        auto getLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<TableRow> override;
        auto getLimitOffsetByField(std::uint32_t offset, std::uint32_t limit, TableFields field, const char *str)
            -> std::vector<TableRow> override;
        auto count() -> std::uint32_t override;
        auto countByFieldId(const char *field, std::uint32_t id) -> std::uint32_t override;

        auto getArtistsLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<Artist>;
        auto getArtistsWithMetadataLimitOffset(std::uint32_t offset, std::uint32_t limit)
            -> std::vector<ArtistWithMetadata>;
        auto countArtists() -> std::uint32_t;

        auto getAlbumsLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<Album>;
        auto getAlbumsWithMetadataLimitOffset(std::uint32_t offset, std::uint32_t limit)
            -> std::vector<AlbumWithMetadata>;
        auto countAlbums() -> std::uint32_t;

        auto getLimitOffset(const Artist &artist, std::uint32_t offset, std::uint32_t limit) -> std::vector<TableRow>;
        auto count(const Artist &artist) -> std::uint32_t;
        auto countTotalLength(const Artist &artist) -> std::uint32_t;

        auto getLimitOffset(const Album &album, std::uint32_t offset, std::uint32_t limit) -> std::vector<TableRow>;
        auto count(const Album &album) -> std::uint32_t;
        auto countTotalLength(const Album &album) -> std::uint32_t;

        auto getLimitOffsetByPaths(const std::vector<std::string> &paths, std::uint32_t offset, std::uint32_t limit)
            -> std::vector<TableRow>;
        auto count(const std::vector<std::string> &paths) -> std::uint32_t;
        TableRow getByPath(const std::string &path);

        /// @note entry.ID is skipped
        auto addOrUpdate(const TableRow &entry, const std::string &oldPath = "") -> bool;

      private:
        auto getFieldName(TableFields field) const -> std::string;
    };
} // namespace db::multimedia_files
