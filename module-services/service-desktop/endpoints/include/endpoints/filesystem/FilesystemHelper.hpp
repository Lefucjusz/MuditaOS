// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <endpoints/BaseHelper.hpp>
#include "FileOperations.hpp"

namespace sdesktop::endpoints
{
    namespace json
    {
        namespace fs
        {
            inline constexpr auto removeFile   = "removeFile";
            inline constexpr auto renameFile   = "renameFile";
            inline constexpr auto destFilename = "destFilename";
            inline constexpr auto listDir      = "listDir";
            inline constexpr auto path         = "path";
            inline constexpr auto fileName     = "fileName";
            inline constexpr auto fileSize     = "fileSize";
            inline constexpr auto type         = "type";
            inline constexpr auto fileCrc32    = "fileCrc32";
            inline constexpr auto chunkSize    = "chunkSize";
            inline constexpr auto chunkNo      = "chunkNo";
            inline constexpr auto data         = "data";
            inline constexpr auto rxID         = "rxID";
            inline constexpr auto txID         = "txID";
        } // namespace fs

        namespace reason
        {
            inline constexpr auto fileDoesNotExist = "file does not exist";
        }
    } // namespace json

    class FilesystemHelper : public BaseHelper
    {
      public:
        FilesystemHelper(sys::Service *p, FileOperations &fileOperations)
            : BaseHelper(p), fileOperations{fileOperations}
        {}

        [[nodiscard]] auto processGet(Context &context) -> ProcessResult final;
        [[nodiscard]] auto processPut(Context &context) -> ProcessResult final;
        [[nodiscard]] auto processDelete(Context &context) -> ProcessResult final;

      private:
        enum class EntryType
        {
            Directory,
            RegularFile,
            Symlink,
            Other
        };

        [[nodiscard]] auto startGetFile(Context &context) const -> ResponseContext;
        [[nodiscard]] auto getFileChunk(Context &context) const -> ResponseContext;

        [[nodiscard]] auto startSendFile(Context &context) const -> ResponseContext;
        [[nodiscard]] auto sendFileChunk(Context &context) const -> ResponseContext;

        auto requestFileRemoval(const std::filesystem::path &path) -> bool;
        auto requestFileRename(const std::filesystem::path &fileName,
                               const std::filesystem::path &destFileName) noexcept -> bool;
        [[nodiscard]] auto requestListDir(const std::filesystem::path &path) const -> ResponseContext;

        auto requestLogsFlush() const -> void;

        [[nodiscard]] auto getFreeSpaceForUserFilesMiB() const -> float;
        [[nodiscard]] auto parseFileEntry(const std::filesystem::directory_entry &entry) const -> json11::Json;

        FileOperations &fileOperations;
    };
} // namespace sdesktop::endpoints
