#pragma once

#include "battery_charger.hpp"
#include <purefs/filesystem_paths.hpp>
#include <vector>

namespace bsp::battery_charger
{
    class FuelGaugeConfig
    {
      public:
        enum class RetCode
        {
            OK,
            MissingFileError,
            DataSizeError,
            FileCrcError,
            ReadingRegisterError,
            WritingRegisterError,
            StoreVerificationError,
            ConfigFuelGaugeModelError
        };

        auto loadConfiguration(std::vector<Register> &configData) -> RetCode;
        auto storeConfiguration(const std::vector<Register> &configData) -> RetCode;
        [[nodiscard]] auto verifyStoredConfiguration(const std::vector<Register> &configData) const -> RetCode;

      private:
        using Crc32 = std::uint32_t;

        static constexpr auto configFileSizeWithoutChecksum = fuelGaugeRegistersCount * sizeof(Register);
        static constexpr auto configFileSizeWithChecksum    = configFileSizeWithoutChecksum + sizeof(Crc32);

        const std::filesystem::path configFilePath = purefs::dir::getSystemVarDirPath() / "batteryFuelGaugeConfig.cfg";

        [[nodiscard]] auto computeChecksum(const std::vector<Register> &configData) const noexcept -> Crc32;
        [[nodiscard]] auto getConfigFileSize() const noexcept -> std::uintmax_t;
        [[nodiscard]] auto isConfigFileValid(const std::vector<Register> &configData,
                                             Crc32 checksumFromFile) const noexcept -> bool;
    };
} // namespace bsp::battery_charger
