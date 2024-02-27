
// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace tags::fetcher
{
    struct Tags
    {
        /* Total audio duration in seconds */
        std::uint32_t total_duration_s = 0;
        /* Audio duration - hours part */
        std::uint32_t duration_hour = 0;
        /* Audio duration - minutes part */
        std::uint32_t duration_min = 0;
        /* Audio duration - seconds part */
        std::uint32_t duration_sec = 0;

        /* Sample rate */
        std::uint32_t sample_rate = 0;
        /* Number of channels */
        std::uint32_t num_channel = 0;
        /* bitrate */
        std::uint32_t bitrate = 0;

        std::string artist;
        std::string genre;
        std::string album;
        std::uint32_t year = 0;
        std::string filePath;
        std::string title;
        std::string comment;
        std::uint32_t track = 0;

        Tags() = default;
        Tags(std::uint32_t total_duration_s,
             std::uint32_t duration_hour,
             std::uint32_t duration_min,
             std::uint32_t duration_sec,
             std::uint32_t sample_rate,
             std::uint32_t num_channel,
             std::uint32_t bitrate,
             std::string artist,
             std::string genre,
             std::string title,
             std::string album,
             std::uint32_t year,
             std::string filePath,
             std::string comment,
             std::uint32_t track)
            : total_duration_s{total_duration_s}, duration_hour{duration_hour}, duration_min{duration_min},
              duration_sec{duration_sec}, sample_rate{sample_rate}, num_channel{num_channel}, bitrate{bitrate},
              artist{std::move(artist)}, genre{std::move(genre)}, album{std::move(album)}, year{year},
              filePath{std::move(filePath)}, title{std::move(title)}, comment{std::move(comment)}, track{track}
        {}

        Tags(std::string filePath, std::string title) : filePath{std::move(filePath)}, title{std::move(title)}
        {}
    };

    Tags fetchTags(const std::string &fileName);
} // namespace tags::fetcher
