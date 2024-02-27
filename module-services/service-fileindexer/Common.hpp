// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <array>
#include <algorithm>
#include <filesystem>
#include <array>

namespace service::detail
{
    /* File extensions indexing allow list */
    constexpr std::array<std::string_view, 3> allowedExtensions{".wav", ".mp3", ".flac"};

    /* This is a debug feature, enabling to override allowed extension types above */
    constexpr auto allowAllExtensions = false;

    inline bool isExtensionSupported(const std::filesystem::path &path)
    {
        if (allowAllExtensions) {
            return true;
        }

        return std::any_of(allowedExtensions.begin(), allowedExtensions.end(), [&path](const auto &extension) {
            if (path.has_extension() && path.extension() == extension) {
                return true;
            }

            /* If empty string with extension only */
            return (path == extension);
        });
    }
} // namespace service::detail
