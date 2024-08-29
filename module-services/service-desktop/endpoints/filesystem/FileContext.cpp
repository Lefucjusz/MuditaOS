// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <endpoints/filesystem/FileContext.hpp>
#include <log/log.hpp>
#include <utility>
#include <fstream>

FileContext::FileContext(const std::filesystem::path &path, std::size_t size, std::size_t chunkSize, std::size_t offset)
    : path(path), size(size), offset(offset), chunkSize(chunkSize)
{
    if ((size == 0) || (chunkSize == 0)) {
        throw std::invalid_argument("Invalid FileContext arguments");
    }
    computedCrc32.reset();
}

auto FileContext::validateChunkRequest(std::uint32_t chunkNo) const -> bool
{
    return (chunkNo >= firstValidChunkNo) && (chunkNo <= totalChunksInFile()) && (chunkNo == expectedChunkInFile());
}

auto FileContext::reachedEOF() const -> bool
{
    return offset >= size;
}

auto FileContext::advanceFileOffset(std::size_t sizeToAdvance) -> void
{
    offset += sizeToAdvance;
}

auto FileContext::chunksForSize(std::size_t fileSize) const -> std::size_t
{
    return (fileSize + chunkSize - 1) / chunkSize;
}

auto FileContext::expectedChunkInFile() const -> std::size_t
{
    return chunksForSize(offset) + 1;
}

auto FileContext::totalChunksInFile() const -> std::size_t
{
    return chunksForSize(size);
}

auto FileContext::fileHash() const -> std::string
{
    return computedCrc32.getHash();
}

FileReadContext::FileReadContext(const std::filesystem::path &path,
                                 std::size_t size,
                                 std::size_t chunkSize,
                                 std::size_t offset)
    : FileContext(path, size, chunkSize, offset), file(path, std::ios::binary)
{}

auto FileReadContext::read() -> std::vector<std::uint8_t>
{
    if (!file.is_open() || file.fail()) {
        LOG_ERROR("File '%s' open error", path.c_str());
        throw std::runtime_error("File open error");
    }

    const auto bytesToRead = std::min(chunkSize, size - offset);

    std::vector<std::uint8_t> buffer(bytesToRead);

    file.read(reinterpret_cast<char *>(buffer.data()), bytesToRead);
    if (file.bad()) {
        LOG_ERROR("File '%s' read error", path.c_str());
        throw std::runtime_error("File read error");
    }

    computedCrc32.add(buffer.data(), bytesToRead);
    advanceFileOffset(bytesToRead);

    LOG_INFO("Read %zuB", bytesToRead);

    if (reachedEOF()) {
        LOG_INFO("Reached EOF of '%s'", path.c_str());
    }
    return buffer;
}

FileWriteContext::FileWriteContext(const std::filesystem::path &path,
                                   std::size_t size,
                                   std::size_t chunkSize,
                                   const std::string &receivedCrc32,
                                   std::size_t offset)
    : FileContext(path, size, chunkSize, offset), receivedCrc32(receivedCrc32), file(path, std::ios::binary)
{}

auto FileWriteContext::write(const std::vector<std::uint8_t> &data) -> void
{
    if (!file.is_open() || file.fail()) {
        LOG_ERROR("File '%s' open error", path.c_str());
        throw std::runtime_error("File open error");
    }

    const auto bytesToWrite = std::min(chunkSize, size - offset);

    file.write(reinterpret_cast<const char *>(data.data()), bytesToWrite);
    file.flush();
    if (file.bad()) {
        LOG_ERROR("File '%s' write error", path.c_str());
        throw std::runtime_error("File write error");
    }

    computedCrc32.add(data.data(), bytesToWrite);
    advanceFileOffset(bytesToWrite);

    if (reachedEOF()) {
        LOG_INFO("Reached EOF of '%s'", path.c_str());
    }
}

auto FileWriteContext::crc32Matches() const -> bool
{
    const auto &computedHash = fileHash();
    const auto match         = (receivedCrc32 == computedHash);
    LOG_INFO("Received CRC32: %s, computed CRC32: %s, matches: %s",
             receivedCrc32.c_str(),
             computedHash.c_str(),
             match ? "yes" : "no");
    return match;
}

auto FileWriteContext::removeFile() -> void
{
    std::error_code ec;
    std::filesystem::remove(path, ec);
}
