// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <crc32.h>
#include <filesystem>
#include <vector>
#include <map>
#include <atomic>
#include <fstream>

class FileContext
{
  public:
    FileContext(const std::filesystem::path &path, std::size_t size, std::size_t chunkSize, std::size_t offset = 0);

    virtual ~FileContext() = default;

    [[nodiscard]] auto validateChunkRequest(std::uint32_t chunkNo) const -> bool;
    [[nodiscard]] auto reachedEOF() const -> bool;

    auto advanceFileOffset(std::size_t sizeToAdvance) -> void;

    [[nodiscard]] auto chunksForSize(std::size_t size) const -> std::size_t;
    [[nodiscard]] auto expectedChunkInFile() const -> std::size_t;
    [[nodiscard]] auto totalChunksInFile() const -> std::size_t;

    [[nodiscard]] auto fileHash() const -> std::string;

  protected:
    static constexpr auto firstValidChunkNo = 1;

    std::filesystem::path path;
    std::size_t size;
    std::size_t offset;
    std::size_t chunkSize;
    CRC32 computedCrc32;
};

class FileReadContext : public FileContext
{
  public:
    FileReadContext(const std::filesystem::path &path, std::size_t size, std::size_t chunkSize, std::size_t offset = 0);

    ~FileReadContext() override = default;

    auto read() -> std::vector<std::uint8_t>;

  private:
    std::ifstream file;
};

class FileWriteContext : public FileContext
{
  public:
    FileWriteContext(const std::filesystem::path &path,
                     std::size_t size,
                     std::size_t chunkSize,
                     const std::string &receivedCrc32,
                     std::size_t offset = 0);

    ~FileWriteContext() override = default;

    auto write(const std::vector<std::uint8_t> &data) -> void;
    auto crc32Matches() const -> bool;
    auto removeFile() -> void;

  private:
    std::string receivedCrc32;
    std::ofstream file;
};
