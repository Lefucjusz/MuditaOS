// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <SwitchData.hpp>
#include <MultimediaFilesRecord.hpp>

namespace gui
{
    using namespace db::multimedia_files;

    class SongsListAlbumData : public SwitchData
    {
      public:
        explicit SongsListAlbumData(const Album &album) : album(album)
        {}

        auto getAlbum() -> Album
        {
            return album;
        }

      private:
        const Album album;
    };

    class SongsListArtistData : public SwitchData
    {
      public:
        explicit SongsListArtistData(const Artist &artist) : artist(artist)
        {}

        auto getArtist() -> Artist
        {
            return artist;
        }

      private:
        const Artist artist;
    };
} // namespace gui
