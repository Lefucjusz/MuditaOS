// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "FileContext.hpp"

#include <log/log.hpp>
#include <filesystem>
#include <vector>
#include <map>
#include <atomic>

class FileOperations
{
  public:
    using TransferId = std::uint32_t;

    /* Each 3 bytes of binary data is coded as 4 bytes of Base64 */
    static constexpr auto binToBase64Factor = 4U;
    static constexpr auto base64ToBinFactor = 3U;
    static constexpr auto mod3MaxReminder   = 2U;

    /* Make chunk size divisible by 3 * 4 = 12, so that the same size can be used
     * regardless of the direction of data flow. */
    static constexpr auto minimumChunkSize    = base64ToBinFactor * binToBase64Factor;
    static constexpr auto chunkSizeMultiplier = 6U * 1024U;
    static constexpr auto chunkSize           = chunkSizeMultiplier * minimumChunkSize;

    struct EncodedDataWithCrc32
    {
        std::string data;
        std::string crc32;
    };

    FileOperations(const FileOperations &) = delete;
    FileOperations &operator=(const FileOperations &) = delete;

    static FileOperations &getInstance();

    auto createReceiveIDForFile(const std::string &file) -> std::pair<TransferId, std::size_t>;
    auto getDataForReceiveID(TransferId rxID, std::uint32_t chunkNo) -> EncodedDataWithCrc32;

    auto createTransmitIDForFile(const std::string &file, std::size_t size, const std::string &crc32) -> TransferId;
    auto sendDataForTransmitID(TransferId txId, std::uint32_t chunkNo, const std::string &data) -> void;

    auto cleanUpUndeliveredTransfers() -> void;

  private:
    FileOperations()  = default;
    ~FileOperations() = default;

    std::map<TransferId, std::unique_ptr<FileReadContext>> readTransfers;
    std::map<TransferId, std::unique_ptr<FileWriteContext>> writeTransfers;

    std::atomic<TransferId> runningRxId{0};
    std::atomic<TransferId> runningTxId{0};
    std::vector<std::uint8_t> decodedData;

    auto createFileReadContextFor(const std::string &file, std::size_t fileSize, TransferId xfrId) -> void;
    auto createFileWriteContextFor(const std::string &file,
                                   std::size_t fileSize,
                                   const std::string &crc32,
                                   TransferId xfrId) -> void;

    [[nodiscard]] auto encodeDataAsBase64(const std::vector<std::uint8_t> &binaryData) const -> std::string;
    auto decodeDataFromBase64(const std::string &encodedData) -> void;

    [[nodiscard]] auto encodedSize(std::size_t binarySize) const -> std::size_t;
    [[nodiscard]] auto decodedSize(std::size_t encodedSize) const -> std::size_t;

    auto cancelTimedOutReadTransfer() -> void;
    auto cancelTimedOutWriteTransfer() -> void;

    auto freeDecodedDataBuffer() -> void;
};
