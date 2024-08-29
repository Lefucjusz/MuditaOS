// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <endpoints/filesystem/FilesystemHelper.hpp>
#include <service-desktop/ServiceDesktop.hpp>
#include <endpoints/message/Sender.hpp>

#include <sys/statvfs.h>

namespace sdesktop::endpoints
{
    namespace
    {
        constexpr auto bytesInMebibyte = 1024LLU * 1024LLU;
    }

    auto FilesystemHelper::processGet(Context &context) -> ProcessResult
    {
        const auto &body = context.getBody();
        ResponseContext response;

        if (body[json::fs::fileName].is_string()) {
            response = startGetFile(context);
        }
        else if (body[json::fs::rxID].is_number()) {
            response = getFileChunk(context);
        }
        else if (body[json::fs::listDir].is_string()) {
            const auto &directory = body[json::fs::listDir].string_value();
            response              = requestListDir(directory);
        }
        else {
            LOG_ERROR("Bad request, missing or invalid argument");
            response = {.status = http::Code::BadRequest};
        }

        return {sent::no, std::move(response)};
    }

    auto FilesystemHelper::processPut(Context &context) -> ProcessResult
    {
        const auto &body = context.getBody();
        ResponseContext response;

        if (body[json::fs::fileName].is_string() && body[json::fs::fileSize].is_number() &&
            body[json::fs::fileCrc32].is_string()) {
            response = startSendFile(context);
        }
        else if (body[json::fs::txID].is_number() && body[json::fs::chunkNo].is_number() &&
                 body[json::fs::data].is_string()) {
            response = sendFileChunk(context);
        }
        else if (body[json::fs::renameFile].is_string() && body[json::fs::destFilename].is_string()) {
            const auto &fileName     = body[json::fs::renameFile].string_value();
            const auto &destFilename = body[json::fs::destFilename].string_value();
            const auto code = requestFileRename(fileName, destFilename) ? http::Code::NoContent : http::Code::NotFound;

            response = {.status = code};
        }
        else {
            LOG_ERROR("Bad request, missing or invalid argument");
            response = {.status = http::Code::BadRequest};
        }

        return {sent::no, std::move(response)};
    }

    auto FilesystemHelper::processDelete(Context &context) -> ProcessResult
    {
        const auto &body = context.getBody();
        auto code        = http::Code::BadRequest;

        if (body[json::fs::removeFile].is_string()) {
            const auto &fileName = body[json::fs::removeFile].string_value();
            try {
                code = requestFileRemoval(fileName) ? http::Code::NoContent : http::Code::NotFound;
            }
            catch (const std::filesystem::filesystem_error &e) {
                LOG_ERROR("Can't remove requested file '%s', error: %d", fileName.c_str(), e.code().value());
                code = http::Code::InternalServerError;
            }
        }
        else {
            LOG_ERROR("Bad request, missing or invalid argument");
        }

        return {sent::no, ResponseContext{.status = code}};
    }

    auto FilesystemHelper::requestLogsFlush() const -> void
    {
        auto ownerService = dynamic_cast<ServiceDesktop *>(owner);
        if (ownerService != nullptr) {
            ownerService->requestLogsFlush();
        }
    }

    auto FilesystemHelper::startGetFile(Context &context) const -> ResponseContext
    {
        const std::filesystem::path &filePath = context.getBody()[json::fs::fileName].string_value();

        try {
            requestLogsFlush();
        }
        catch (const std::runtime_error &e) {
            LOG_ERROR("Logs flush exception: %s", e.what());

            json11::Json::object response({{json::common::reason, e.what()}});
            return ResponseContext{.status = http::Code::InternalServerError, .body = response};
        }

        if (!std::filesystem::exists(filePath)) {
            LOG_ERROR("File '%s' not found", filePath.c_str());

            json11::Json::object response({{json::common::reason, json::reason::fileDoesNotExist}});
            return {http::Code::NotFound, std::move(response)};
        }

        json11::Json::object response;
        auto code = http::Code::BadRequest;
        try {
            const auto [rxID, fileSize] = fileOperations.createReceiveIDForFile(filePath);

            code     = http::Code::OK;
            response = json11::Json::object({{json::fs::rxID, static_cast<int>(rxID)},
                                             {json::fs::fileSize, static_cast<int>(fileSize)},
                                             {json::fs::chunkSize, static_cast<int>(FileOperations::chunkSize)}});
        }
        catch (std::runtime_error &e) {
            LOG_ERROR("FileOperations exception: %s", e.what());

            code     = http::Code::InternalServerError;
            response = json11::Json::object({{json::common::reason, e.what()}});
        }
        catch (std::exception &e) {
            LOG_ERROR("FileOperations exception: %s", e.what());

            code     = http::Code::BadRequest;
            response = json11::Json::object({{json::common::reason, e.what()}});
        }

        return ResponseContext{code, std::move(response)};
    }

    auto FilesystemHelper::getFileChunk(Context &context) const -> ResponseContext
    {
        const auto rxID    = context.getBody()[json::fs::rxID].int_value();
        const auto chunkNo = context.getBody()[json::fs::chunkNo].int_value();
        FileOperations::EncodedDataWithCrc32 dataWithCrc32;

        try {
            dataWithCrc32 = fileOperations.getDataForReceiveID(rxID, chunkNo);
        }
        catch (std::exception &e) {
            LOG_ERROR("Exception during getting data: %s", e.what());

            json11::Json::object response({{json::common::reason, e.what()}});
            return {http::Code::BadRequest, std::move(response)};
        }

        json11::Json::object response;
        auto code = http::Code::BadRequest;
        if (!dataWithCrc32.data.empty()) {
            code     = http::Code::OK;
            response = json11::Json::object({{json::fs::rxID, static_cast<int>(rxID)},
                                             {json::fs::chunkNo, static_cast<int>(chunkNo)},
                                             {json::fs::data, dataWithCrc32.data},
                                             {json::fs::fileCrc32, dataWithCrc32.crc32}});
        }
        else {
            const auto &errorReason = std::string("Invalid request, Rx ID: ") + std::to_string(rxID) +
                                      ", chunk number: " + std::to_string(chunkNo);
            LOG_ERROR("%s", errorReason.c_str());

            code     = http::Code::BadRequest;
            response = json11::Json::object({{json::common::reason, errorReason.c_str()}});
        }

        return {code, std::move(response)};
    }

    auto FilesystemHelper::startSendFile(Context &context) const -> ResponseContext
    {
        const auto &body      = context.getBody();
        const auto &filePath  = body[json::fs::fileName].string_value();
        const auto &fileCrc32 = body[json::fs::fileCrc32].string_value();
        const auto fileSize   = static_cast<std::uint32_t>(body[json::fs::fileSize].int_value());
        auto code             = http::Code::BadRequest;

        LOG_DEBUG("Start sending of file: %s", filePath.c_str());

        if ((fileSize == 0) || fileCrc32.empty()) {
            LOG_ERROR("File '%s' corrupted", filePath.c_str());
            return ResponseContext{.status = code};
        }

        const auto freeSpaceLeftForUserFilesMiB = getFreeSpaceForUserFilesMiB();

        if ((freeSpaceLeftForUserFilesMiB - (static_cast<float>(fileSize) / bytesInMebibyte)) <= 0) {
            LOG_ERROR("Not enough space left on device!");
            code = http::Code::InsufficientStorage;
            return ResponseContext{.status = code};
        }

        if (!std::filesystem::exists(filePath)) {
            LOG_DEBUG("Creating file %s", filePath.c_str());
        }
        else {
            LOG_DEBUG("Overwriting file %s", filePath.c_str());
        }

        json11::Json::object response;

        try {
            auto txID = fileOperations.createTransmitIDForFile(filePath, fileSize, fileCrc32);

            code     = http::Code::OK;
            response = json11::Json::object({{json::fs::txID, static_cast<int>(txID)},
                                             {json::fs::chunkSize, static_cast<int>(FileOperations::chunkSize)}});
        }
        catch (std::runtime_error &e) {
            LOG_ERROR("FileOperations exception: %s", e.what());

            code     = http::Code::InternalServerError;
            response = json11::Json::object({{json::common::reason, e.what()}});
        }
        catch (std::exception &e) {
            LOG_ERROR("FileOperations exception: %s", e.what());

            code     = http::Code::BadRequest;
            response = json11::Json::object({{json::common::reason, e.what()}});
        }

        return {code, std::move(response)};
    }

    auto FilesystemHelper::sendFileChunk(Context &context) const -> ResponseContext
    {
        const auto &body   = context.getBody();
        const auto &data   = body[json::fs::data].string_value();
        const auto txID    = body[json::fs::txID].int_value();
        const auto chunkNo = body[json::fs::chunkNo].int_value();

        auto code = http::Code::BadRequest;
        json11::Json::object response;

        if (data.empty()) {
            const auto &errorReason = std::string("Invalid request, Tx ID: ") + std::to_string(txID) +
                                      ", chunk number: " + std::to_string(chunkNo);
            LOG_ERROR("%s", errorReason.c_str());

            response = json11::Json::object({{json::common::reason, errorReason.c_str()}});
            return {code, std::move(response)};
        }

        try {
            fileOperations.sendDataForTransmitID(txID, chunkNo, data);

            code     = http::Code::OK;
            response = json11::Json::object(
                {{json::fs::txID, static_cast<int>(txID)}, {json::fs::chunkNo, static_cast<int>(chunkNo)}});
        }
        catch (std::runtime_error &e) {
            LOG_ERROR("Exception during sending data: %s", e.what());

            code     = http::Code::InternalServerError;
            response = json11::Json::object(
                {{json::fs::txID, static_cast<int>(txID)}, {json::fs::chunkNo, static_cast<int>(chunkNo)}});
        }
        catch (std::exception &e) {
            LOG_ERROR("Exception during sending data: %s", e.what());

            code     = http::Code::BadRequest;
            response = json11::Json::object({{json::common::reason, e.what()}});
        }

        return {code, std::move(response)};
    }

    auto FilesystemHelper::requestFileRemoval(const std::filesystem::path &path) -> bool
    {
        return std::filesystem::remove(path);
    }

    auto FilesystemHelper::requestFileRename(const std::filesystem::path &fileName,
                                             const std::filesystem::path &destFilename) noexcept -> bool
    {
        std::error_code ec;
        std::filesystem::rename(fileName, destFilename, ec);
        if (!ec) {
            LOG_ERROR("Failed to rename file '%s', error: %d", fileName.c_str(), ec.value());
        }
        return !ec;
    }

    auto FilesystemHelper::requestListDir(const std::filesystem::path &path) const -> ResponseContext
    {
        if (!std::filesystem::exists(path)) {
            return ResponseContext{.status = http::Code::NotFound};
        }

        json11::Json::array jsonArr;
        for (const auto &entry : std::filesystem::directory_iterator{path}) {
            jsonArr.push_back(parseFileEntry(entry));
        }

        json11::Json::object response({{path, jsonArr}});
        return {http::Code::OK, std::move(response)};
    }

    auto FilesystemHelper::getFreeSpaceForUserFilesMiB() const -> float
    {
        const auto &userDiskPath = purefs::dir::getUserDiskPath();
        struct statvfs vfstat
        {};

        if (statvfs(userDiskPath.c_str(), &vfstat) < 0) {
            return 0;
        }

        const auto freeBytes = (static_cast<float>(vfstat.f_bfree) * static_cast<float>(vfstat.f_bsize));
        const auto freeMiBs  = freeBytes / bytesInMebibyte;

        return freeMiBs;
    }

    auto FilesystemHelper::parseFileEntry(const std::filesystem::directory_entry &entry) const -> json11::Json
    {
        std::uintmax_t size = 0;
        EntryType type;

        if (entry.is_directory()) {
            type = EntryType::Directory;
        }
        else {
            size = entry.file_size();
        }

        if (entry.is_regular_file()) {
            type = EntryType::RegularFile;
        }
        else if (entry.is_symlink()) {
            type = EntryType::Symlink;
        }
        else {
            type = EntryType::Other;
        }

        return json11::Json::object{{json::fs::path, entry.path().string()},
                                    {json::fs::fileSize, size},
                                    {json::fs::type, static_cast<int>(type)}};
    }
} // namespace sdesktop::endpoints
