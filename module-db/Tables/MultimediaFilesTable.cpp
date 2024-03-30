// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MultimediaFilesTable.hpp"
#include "Common/Types.hpp"
#include <Database/QueryResult.hpp>
#include <Utils.hpp>
#include <magic_enum.hpp>
#include <cinttypes>

namespace db::multimedia_files
{
    namespace
    {
        auto constructMatchPattern(const std::vector<std::string> &paths) -> std::string
        {
            std::string ret;
            for (auto e = paths.begin(); e != paths.end(); e++) {
                ret += "path LIKE '" + *e + "%%'";
                if (std::next(e) != paths.end()) {
                    ret += " or ";
                }
            }
            return ret;
        }

        auto CreateTableRow(const QueryResult &result) -> TableRow
        {
            if (result.getFieldCount() != magic_enum::enum_count<TableFields>() + 1) {
                return TableRow{};
            }

            return TableRow{
                result[0].getUInt32(),    // ID
                {result[1].getString(),   // path
                 result[2].getString(),   // mediaType
                 result[3].getUInt32()},  // size
                {result[4].getString(),   // title
                 {result[5].getString(),  // artist
                  result[6].getString()}, // album title
                 result[7].getString(),   // comment
                 result[8].getString(),   // genre
                 result[9].getUInt32(),   // year
                 result[10].getUInt32()}, // track
                {result[11].getUInt32(),  // songLength
                 result[12].getUInt32(),  // bitrate
                 result[13].getUInt32(),  // sample rate
                 result[14].getUInt32()}, // channels
            };
        }
    }

    MultimediaFilesTable::MultimediaFilesTable(Database *db) : Table(db)
    {
        createTableRow = CreateTableRow;
    }

    auto MultimediaFilesTable::create() -> bool
    {
        return true;
    }

    auto MultimediaFilesTable::add(TableRow entry) -> bool
    {
        return db->execute("INSERT INTO files (path, media_type, size, title, artist, album, "
                           "comment, genre, year, track, song_length, bitrate, sample_rate, channels) "
                           "VALUES(" str_c str_c u32_c str_c str_c str_c str_c str_c u32_c u32_c u32_c u32_c u32_c u32_
                           ") "
                           "ON CONFLICT(path) DO UPDATE SET "
                           "path = excluded.path, "
                           "media_type = excluded.media_type, "
                           "size = excluded.size, "
                           "title = excluded.title, "
                           "artist = excluded.artist, "
                           "album = excluded.album, "
                           "comment = excluded.comment, "
                           "genre = excluded.genre, "
                           "year = excluded.year, "
                           "track = excluded.track, "
                           "song_length = excluded.song_length, "
                           "bitrate = excluded.bitrate, "
                           "sample_rate = excluded.sample_rate, "
                           "channels = excluded.channels;",
                           entry.fileInfo.path.c_str(),
                           entry.fileInfo.mediaType.c_str(),
                           entry.fileInfo.size,
                           entry.tags.title.c_str(),
                           entry.tags.album.artist.c_str(),
                           entry.tags.album.title.c_str(),
                           entry.tags.comment.c_str(),
                           entry.tags.genre.c_str(),
                           entry.tags.year,
                           entry.tags.track,
                           entry.audioProperties.songLength,
                           entry.audioProperties.bitrate,
                           entry.audioProperties.sampleRate,
                           entry.audioProperties.channels);
    }

    auto MultimediaFilesTable::removeById(std::uint32_t id) -> bool
    {
        return db->execute("DELETE FROM files WHERE _id=" u32_ ";", id);
    }

    auto MultimediaFilesTable::removeByField(TableFields field, const char *str) -> bool
    {
        const auto &fieldName = getFieldName(field);

        if (fieldName.empty()) {
            return false;
        }

        return db->execute("DELETE FROM files WHERE %q=" str_ ";", fieldName.c_str(), str);
    }

    auto MultimediaFilesTable::removeAll() -> bool
    {
        return db->execute("DELETE FROM files;");
    }

    auto MultimediaFilesTable::update(TableRow entry) -> bool
    {
        return db->execute("UPDATE files SET path=" str_c "media_type=" str_c "size=" u32_c "title=" str_c
                           "artist=" str_c "album=" str_c "comment=" str_c "genre=" str_c "year=" u32_c "track=" u32_c
                           "song_length=" u32_c "bitrate=" u32_c "sample_rate=" u32_c "channels=" u32_
                           " WHERE _id=" u32_ ";",
                           entry.fileInfo.path.c_str(),
                           entry.fileInfo.mediaType.c_str(),
                           entry.fileInfo.size,
                           entry.tags.title.c_str(),
                           entry.tags.album.artist.c_str(),
                           entry.tags.album.title.c_str(),
                           entry.tags.comment.c_str(),
                           entry.tags.genre.c_str(),
                           entry.tags.year,
                           entry.tags.track,
                           entry.audioProperties.songLength,
                           entry.audioProperties.bitrate,
                           entry.audioProperties.sampleRate,
                           entry.audioProperties.channels,
                           entry.ID);
    }

    auto MultimediaFilesTable::addOrUpdate(const TableRow &entry, const std::string &oldPath) -> bool
    {
        const auto &path = oldPath.empty() ? entry.fileInfo.path : oldPath;

        return db->execute("BEGIN TRANSACTION; "
                           "INSERT OR IGNORE INTO files (path) VALUES (" str_ "); "
                           "UPDATE files SET path=" str_c "media_type=" str_c "size=" u32_c "title=" str_c
                           "artist=" str_c "album=" str_c "comment=" str_c "genre=" str_c "year=" u32_c "track=" u32_c
                           "song_length=" u32_c "bitrate=" u32_c "sample_rate=" u32_c "channels=" u32_
                           " WHERE path=" str_ "; "
                           "COMMIT;",
                           path.c_str(),
                           entry.fileInfo.path.c_str(),
                           entry.fileInfo.mediaType.c_str(),
                           entry.fileInfo.size,
                           entry.tags.title.c_str(),
                           entry.tags.album.artist.c_str(),
                           entry.tags.album.title.c_str(),
                           entry.tags.comment.c_str(),
                           entry.tags.genre.c_str(),
                           entry.tags.year,
                           entry.tags.track,
                           entry.audioProperties.songLength,
                           entry.audioProperties.bitrate,
                           entry.audioProperties.sampleRate,
                           entry.audioProperties.channels,
                           path.c_str());
    }

    auto MultimediaFilesTable::getById(std::uint32_t id) -> TableRow
    {
        const auto &retQuery = db->query("SELECT * FROM files WHERE _id=" u32_ ";", id);
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return {};
        }

        return CreateTableRow(*retQuery);
    }

    auto MultimediaFilesTable::getByPath(const std::string &path) -> TableRow
    {
        const auto &retQuery = db->query("SELECT * FROM files WHERE path=" str_ ";", path.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return {};
        }

        return CreateTableRow(*retQuery);
    }

    auto MultimediaFilesTable::getLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<TableRow>
    {
        auto retQuery =
            db->query("SELECT * from files ORDER BY title COLLATE NOCASE ASC LIMIT " u32_ " OFFSET " u32_ ";", limit, offset);
        return retQueryUnpack(std::move(retQuery));
    }

    auto MultimediaFilesTable::getArtistsLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<Artist>
    {
        const auto &retQuery = db->query(
            "SELECT DISTINCT artist from files ORDER BY artist COLLATE NOCASE ASC LIMIT " u32_ " OFFSET " u32_ ";", limit, offset);
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return {};
        }

        std::vector<Artist> outVector;

        do {
            outVector.push_back((*retQuery)[0].getString()); // artist
        } while (retQuery->nextRow());
        return outVector;
    }

    auto MultimediaFilesTable::getArtistsWithMetadataLimitOffset(std::uint32_t offset, std::uint32_t limit)
        -> std::vector<ArtistWithMetadata>
    {
        const auto &artists = getArtistsLimitOffset(offset, limit);

        std::vector<ArtistWithMetadata> outVector;
        outVector.reserve(artists.size());

        for (const auto &artist : artists) {
            ArtistWithMetadata artistWithMetadata;

            artistWithMetadata.artist      = artist;
            artistWithMetadata.songsCount  = count(artist);
            artistWithMetadata.totalLength = countTotalLength(artist);

            outVector.push_back(artistWithMetadata);
        }

        return outVector;
    }

    auto MultimediaFilesTable::getLimitOffsetByField(std::uint32_t offset,
                                                                      std::uint32_t limit,
                                                                      TableFields field,
                                                                      const char *str) -> std::vector<TableRow>
    {
        const auto &fieldName = getFieldName(field);
        if (fieldName.empty() || (str == nullptr)) {
            return {};
        }

        auto retQuery =
            db->query("SELECT * FROM files WHERE %q=" str_ " ORDER BY title COLLATE NOCASE ASC LIMIT " u32_ " OFFSET " u32_ ";",
                      fieldName.c_str(),
                      str,
                      limit,
                      offset);

        return retQueryUnpack(std::move(retQuery));
    }

    auto MultimediaFilesTable::count() -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT COUNT(*) FROM files;");
        if (!retQuery || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::countArtists() -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT COUNT (DISTINCT artist) FROM files;");
        if (!retQuery || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::getAlbumsLimitOffset(std::uint32_t offset, std::uint32_t limit) -> std::vector<Album>
    {
        const auto &retQuery =
            db->query("SELECT DISTINCT artist,album FROM files ORDER BY album COLLATE NOCASE ASC LIMIT " u32_ " OFFSET " u32_ ";",
                      limit,
                      offset);
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return {};
        }

        std::vector<Album> outVector;

        do {
            outVector.push_back({.artist = (*retQuery)[0].getString(), .title = (*retQuery)[1].getString()});
        } while (retQuery->nextRow());

        return outVector;
    }

    auto MultimediaFilesTable::getAlbumsWithMetadataLimitOffset(std::uint32_t offset, std::uint32_t limit)
        -> std::vector<AlbumWithMetadata>
    {
        const auto &albums = getAlbumsLimitOffset(offset, limit);

        std::vector<AlbumWithMetadata> outVector;
        outVector.reserve(albums.size());

        for (const auto &album : albums) {
            AlbumWithMetadata albumWithMetadata;

            albumWithMetadata.title       = album.title;
            albumWithMetadata.artist      = album.artist;
            albumWithMetadata.songsCount  = count(album);
            albumWithMetadata.totalLength = countTotalLength(album);

            outVector.push_back(albumWithMetadata);
        }

        return outVector;
    }

    auto MultimediaFilesTable::countAlbums() -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT COUNT(*) FROM (SELECT DISTINCT album,artist from files);");
        if (!retQuery || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::countByFieldId(const char *field, std::uint32_t id) -> std::uint32_t
    {
        if (field == nullptr) {
            return 0;
        }

        const auto &retQuery = db->query("SELECT COUNT(*) FROM files WHERE " str_ "=" u32_ ";", field, id);
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::getFieldName(TableFields field) const -> std::string
    {
        return utils::enumToString(field);
    }

    auto MultimediaFilesTable::getLimitOffset(const Artist &artist, std::uint32_t offset, std::uint32_t limit)
        -> std::vector<TableRow>
    {
        return getLimitOffsetByField(offset, limit, TableFields::artist, artist.c_str());
    }

    auto MultimediaFilesTable::count(const Artist &artist) -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT COUNT(*) FROM files WHERE artist=" str_ " ;", artist.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::countTotalLength(const Artist &artist) -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT SUM(song_length) FROM files WHERE artist=" str_ ";", artist.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::getLimitOffset(const Album &album, std::uint32_t offset, std::uint32_t limit)
        -> std::vector<TableRow>
    {
        auto retQuery = db->query("SELECT * FROM files WHERE artist=" str_ " AND album=" str_
                                  " ORDER BY track COLLATE NOCASE ASC LIMIT " u32_ " OFFSET " u32_ ";",
                                  album.artist.c_str(),
                                  album.title.c_str(),
                                  limit,
                                  offset);

        return retQueryUnpack(std::move(retQuery));
    }

    auto MultimediaFilesTable::count(const Album &album) -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT COUNT(*) FROM files WHERE artist=" str_ " AND album=" str_ ";",
                                         album.artist.c_str(),
                                         album.title.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::countTotalLength(const Album &album) -> std::uint32_t
    {
        const auto &retQuery = db->query("SELECT SUM(song_length) FROM files WHERE artist=" str_ " AND album=" str_ ";",
                                         album.artist.c_str(),
                                         album.title.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }

    auto MultimediaFilesTable::getLimitOffsetByPaths(const std::vector<std::string> &paths,
                                                     std::uint32_t offset,
                                                     std::uint32_t limit) -> std::vector<TableRow>
    {
        const auto &query = "SELECT * FROM files WHERE " + constructMatchPattern(paths) +
                            " ORDER BY title COLLATE NOCASE ASC LIMIT " + std::to_string(limit) +
                            " OFFSET " + std::to_string(offset) + ";";
        auto retQuery = db->query(query.c_str());
        return retQueryUnpack(std::move(retQuery));
    }

    auto MultimediaFilesTable::count(const std::vector<std::string> &paths) -> std::uint32_t
    {
        const auto &query    = "SELECT COUNT(*) FROM files WHERE " + constructMatchPattern(paths) + ";";
        const auto &retQuery = db->query(query.c_str());
        if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
            return 0;
        }

        return (*retQuery)[0].getUInt32();
    }
} // namespace db::multimedia_files
