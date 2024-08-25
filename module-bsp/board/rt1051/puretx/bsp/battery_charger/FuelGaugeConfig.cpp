#include "FuelGaugeConfig.hpp"
#include <crc32.h>
#include <fstream>

namespace bsp::battery_charger
{
    auto FuelGaugeConfig::loadConfiguration(std::vector<Register> &configData) -> RetCode
    {
        std::ifstream file(configFilePath, std::ios::binary);
        if (!file.is_open()) {
            return RetCode::MissingFileError;
        }

        std::uint16_t regVal;
        for (auto i = 0; i < fuelGaugeRegistersCount; ++i) {
            file.read(reinterpret_cast<char *>(&regVal), sizeof(regVal));
            if (file.eof()) {
                return RetCode::DataSizeError;
            }
            configData.push_back(regVal);
        }

        Crc32 checksumFromFile;
        file.read(reinterpret_cast<char *>(&checksumFromFile), sizeof(checksumFromFile));
        if (!isConfigFileValid(configData, checksumFromFile)) {
            return RetCode::FileCrcError;
        }
        return RetCode::OK;
    }

    auto FuelGaugeConfig::storeConfiguration(const std::vector<Register> &configData) -> RetCode
    {
        std::ofstream file(configFilePath, std::ios::binary);
        if (!file.is_open()) {
            return RetCode::MissingFileError;
        }
        if (configData.size() != fuelGaugeRegistersCount) {
            return RetCode::DataSizeError;
        }

        /* Write registers */
        for (const auto &value : configData) {
            file.write(reinterpret_cast<const char *>(&value), sizeof(value));
        }

        /* Write checksum */
        const auto checksum = computeChecksum(configData);
        file.write(reinterpret_cast<const char *>(&checksum), sizeof(checksum));

        return RetCode::OK;
    }

    auto FuelGaugeConfig::verifyStoredConfiguration(const std::vector<Register> &configData) const -> RetCode
    {
        std::ifstream file(configFilePath, std::ios::binary);
        if (!file.is_open()) {
            return RetCode::MissingFileError;
        }
        if (configData.size() != fuelGaugeRegistersCount) {
            return RetCode::DataSizeError;
        }

        std::uint16_t savedRegValue;
        for (auto i = 0; i < fuelGaugeRegistersCount; ++i) {
            file.read(reinterpret_cast<char *>(&savedRegValue), sizeof(savedRegValue));
            if (savedRegValue != configData[i]) {
                return RetCode::StoreVerificationError;
            }
        }
        return RetCode::OK;
    }

    auto FuelGaugeConfig::computeChecksum(const std::vector<Register> &configData) const noexcept -> Crc32
    {
        CRC32 checksum;
        for (const auto &value : configData) {
            checksum.add(&value, sizeof(value));
        }
        return checksum.getHashValue();
    }

    auto FuelGaugeConfig::getConfigFileSize() const noexcept -> std::uintmax_t
    {
        std::error_code ec;
        return std::filesystem::file_size(configFilePath, ec);
    }

    auto FuelGaugeConfig::isConfigFileValid(const std::vector<Register> &configData,
                                            Crc32 checksumFromFile) const noexcept -> bool
    {
        const auto size = getConfigFileSize();
        if (size == configFileSizeWithoutChecksum) {
            return true;
        }
        if (size == configFileSizeWithChecksum) {
            const auto checksumComputed = computeChecksum(configData);
            return (checksumComputed == checksumFromFile);
        }
        return false;
    }
} // namespace bsp::battery_charger
