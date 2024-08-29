// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <endpoints/filesystem/FileOperations.hpp>
#include <base64.h>
#include <log/log.hpp>

FileOperations &FileOperations::getInstance()
{
    static FileOperations instance;
    return instance;
}

auto FileOperations::createReceiveIDForFile(const std::string &file) -> std::pair<TransferId, std::size_t>
{
    cancelTimedOutReadTransfer();

    const auto rxID = ++runningRxId;
    const auto size = std::filesystem::file_size(file);
    if (size == 0) {
        LOG_ERROR("File '%s' is empty", file.c_str());
        throw std::runtime_error("File size error");
    }

    LOG_INFO("Creating Rx ID %" PRIu32, rxID);
    createFileReadContextFor(file, size, rxID);

    return {rxID, size};
}

void FileOperations::cancelTimedOutReadTransfer()
{
    if (runningRxId == 0) {
        return;
    }

    const auto timedOutXfer = runningRxId.load();
    const auto fileCtxEntry = readTransfers.find(timedOutXfer);
    if (fileCtxEntry == readTransfers.end()) {
        LOG_INFO("No timed out read transfers");
        return;
    }

    LOG_INFO("Cancelling timed out Rx ID %" PRIu32, timedOutXfer);
    fileCtxEntry->second.reset();
    readTransfers.erase(timedOutXfer);
}

void FileOperations::cancelTimedOutWriteTransfer()
{
    if (runningTxId == 0) {
        return;
    }

    const auto timedOutXfer = runningTxId.load();
    const auto fileCtxEntry = writeTransfers.find(timedOutXfer);
    if (fileCtxEntry == writeTransfers.end()) {
        LOG_INFO("No timed out write transfers");
        return;
    }

    LOG_INFO("Cancelling timed out Tx ID %" PRIu32, timedOutXfer);
    fileCtxEntry->second->removeFile();
    fileCtxEntry->second.reset();
    writeTransfers.erase(timedOutXfer);
}

auto FileOperations::createFileReadContextFor(const std::string &file, std::size_t fileSize, TransferId xfrId) -> void
{
    auto context = std::make_unique<FileReadContext>(file, fileSize, chunkSize);
    readTransfers.insert({xfrId, std::move(context)});
}

auto FileOperations::createFileWriteContextFor(const std::string &file,
                                               std::size_t fileSize,
                                               const std::string &crc32,
                                               TransferId xfrId) -> void
{
    auto context = std::make_unique<FileWriteContext>(file, fileSize, chunkSize, crc32);
    writeTransfers.insert({xfrId, std::move(context)});
}

auto FileOperations::encodedSize(std::size_t binarySize) const -> std::size_t
{
    /* 3 bytes of binary data is converted into 4 bytes of (printable) text.
     * One more byte is used for null termination. */
    return ((binarySize + mod3MaxReminder) / base64ToBinFactor * binToBase64Factor) + 1;
}

auto FileOperations::decodedSize(std::size_t encodedSize) const -> std::size_t
{
    /* 4 bytes of encoded data is converted back into 3 bytes of binary */
    return (((encodedSize + base64ToBinFactor) / binToBase64Factor) * base64ToBinFactor) + 1;
}

auto FileOperations::encodeDataAsBase64(const std::vector<std::uint8_t> &binaryData) const -> std::string
{
    const auto encodedDataSize = encodedSize(binaryData.size());

    std::string encodedData(encodedDataSize, '\0');
    bintob64(encodedData.data(), binaryData.data(), binaryData.size());
    return encodedData;
}

auto FileOperations::decodeDataFromBase64(const std::string &encodedData) -> void
{
    const auto decodedDataSize = decodedSize(encodedData.length());
    if (decodedData.size() < decodedDataSize) {
        decodedData.resize(decodedDataSize, 0);
    }
    b64tobin(decodedData.data(), encodedData.data());
}

auto FileOperations::getDataForReceiveID(TransferId rxID, std::uint32_t chunkNo) -> EncodedDataWithCrc32
{
    const auto fileCtxEntry = readTransfers.find(rxID);
    if (fileCtxEntry == readTransfers.end()) {
        LOG_ERROR("Invalid Rx ID %" PRIu32, rxID);
        return {};
    }

    const auto chunksTotal = fileCtxEntry->second->totalChunksInFile();
    const auto progress    = 100 * chunkNo / chunksTotal;
    LOG_INFO("Getting chunk %" PRIu32 "/%zu (%" PRIu32 "%%) for Rx ID %" PRIu32, chunkNo, chunksTotal, progress, rxID);

    auto fileCtx = fileCtxEntry->second.get();
    if (fileCtx == nullptr) {
        LOG_ERROR("Invalid file context for Rx ID %" PRIu32, rxID);
        return {};
    }

    if (!fileCtx->validateChunkRequest(chunkNo)) {
        LOG_ERROR("Invalid chunk number %" PRIu32, chunkNo);
        return {};
    }

    const auto &data = fileCtx->read();
    if (data.empty()) {
        LOG_ERROR("File read error");
        return {};
    }

    std::string computedCrc32;
    if (fileCtx->reachedEOF()) {
        LOG_INFO("Reached EOF for Rx ID %" PRIu32, rxID);
        computedCrc32 = fileCtx->fileHash();
        writeTransfers.erase(rxID);
    }

    return {std::move(encodeDataAsBase64(data)), std::move(computedCrc32)};
}

auto FileOperations::createTransmitIDForFile(const std::string &file, std::size_t size, const std::string &crc32)
    -> TransferId
{
    cancelTimedOutWriteTransfer();
    const auto txID = ++runningTxId;

    LOG_INFO("Creating Tx ID %" PRIu32, txID);

    createFileWriteContextFor(file, size, crc32, txID);

    return txID;
}

auto FileOperations::sendDataForTransmitID(TransferId txID, std::uint32_t chunkNo, const std::string &data) -> void
{

    const auto fileCtxEntry = writeTransfers.find(txID);
    if (fileCtxEntry == writeTransfers.end()) {
        LOG_ERROR("Invalid Tx ID %" PRIu32, txID);
        throw std::runtime_error("Invalid Tx ID");
    }

    const auto chunksTotal = fileCtxEntry->second->totalChunksInFile();
    const auto progress    = 100 * chunkNo / chunksTotal;
    LOG_INFO(
        "Transmitting chunk %" PRIu32 "/%zu (%" PRIu32 "%%) for Tx ID %" PRIu32, chunkNo, chunksTotal, progress, txID);

    auto fileCtx = fileCtxEntry->second.get();
    if (fileCtx == nullptr) {
        LOG_ERROR("Invalid file context for Tx ID %" PRIu32, txID);
        throw std::runtime_error("Invalid file context for Tx ID");
    }

    if (!fileCtx->validateChunkRequest(chunkNo)) {
        LOG_ERROR("Invalid chunk number %" PRIu32, chunkNo);
        throw std::runtime_error("Invalid chunk number");
    }

    decodeDataFromBase64(data);
    fileCtx->write(decodedData);

    if (fileCtx->reachedEOF()) {
        LOG_INFO("Reached EOF for txID %" PRIu32, txID);

        if (fileCtx->crc32Matches()) {
            writeTransfers.erase(txID);
            freeDecodedDataBuffer();
        }
        else {
            fileCtx->removeFile();
            writeTransfers.erase(txID);
            freeDecodedDataBuffer();

            LOG_ERROR("File CRC32 mismatch");
            throw std::runtime_error("File CRC32 mismatch");
        }
    }
}

auto FileOperations::cleanUpUndeliveredTransfers() -> void
{
    if (writeTransfers.empty()) {
        return;
    }

    LOG_INFO("Cleaning up undelivered transfers");
    for (auto &wt : writeTransfers) {
        wt.second->removeFile();
        wt.second.reset();
        writeTransfers.erase(wt.first);
    }
}

auto FileOperations::freeDecodedDataBuffer() -> void
{
    decodedData.clear();
    decodedData.shrink_to_fit();
}
