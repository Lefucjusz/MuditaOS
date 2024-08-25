// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

// static lifetime read only cache for (hw) states to not poll
// right now it serves data from:
// - battery
// - gsm SIM tray
// it's not meant to serve as polling interface - rather to serve data

#include <hal/cellular/SIM.hpp>

#include <cstddef>
#include <string>

namespace cpp_freertos
{
    class MutexStandard;
} // namespace cpp_freertos

namespace Store
{
    struct Battery
    {
      public:
        enum class LevelState : std::uint8_t
        {
            Normal,
            Shutdown,
            CriticalCharging,
            CriticalNotCharging
        } levelState = LevelState::Normal;

        enum class State : std::uint8_t
        {
            Discharging,
            Charging,
            ChargingDone,
            PluggedNotCharging,
        } state            = State::Discharging;

        enum class Temperature : std::uint8_t
        {
            Normal,
            TooLow,
            TooHigh,
            Unknown
        } temperature = Temperature::Normal;

        unsigned level = 0;

        /// @brief Returns const reference to Battery instance, used to read battery state
        /// @return const Battery&
        static const Battery &get();

        /// @brief Returns reference to Battery instance, used to change battery state
        /// @return Battery&
        static Battery &modify();

        static void setUpdated();
        static bool takeUpdated();

      private:
        static bool updated;
    };

    enum class RssiBar : std::size_t
    {
        Zero  = 0,
        One   = 1,
        Two   = 2,
        Three = 3,
        Four  = 4,
        NoOfSupportedBars
    };

    struct SignalStrength
    {
        int rssi        = 0;
        int rssidBm     = 0;
        RssiBar rssiBar = RssiBar::Zero;
    };

    enum class Tethering
    {
        Off,
        On
    };

    struct Network
    {
        enum class Status
        {
            NotRegistered,
            RegisteredHomeNetwork,
            Searching,
            RegistrationDenied,
            Unknown,
            RegisteredRoaming
        } status = Status::NotRegistered;

        enum class AccessTechnology
        {
            Gsm   = 0,
            Utran = 2,
            GsmWEgprs,
            UtranWHsdpa,
            UtranWHsupa,
            UtranWHsdpaAndWHsupa,
            EUtran,
            Cdma = 100,
            Wcdma,
            Unknown = 255
        } accessTechnology = AccessTechnology::Unknown;

        inline constexpr bool operator==(const Network &rhs) const
        {
            return this->status == rhs.status && this->accessTechnology == rhs.accessTechnology;
        }

        inline constexpr bool operator!=(const Network &rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct GSM
    {
      public:
        GSM(const GSM &) = delete;
        GSM &operator=(const GSM &) = delete;

        enum class Tray
        {
            OUT,
            IN
        } tray = Tray::IN;
        /// tray - tray actual status which is visible right now on screen
        /// selected - tray selection settings settable sim tray
        enum class SIM
        {
            SIM1 = static_cast<int>(hal::cellular::SimSlot::SIM1),
            SIM2 = static_cast<int>(hal::cellular::SimSlot::SIM2),
            SIM_NEED_PIN,
            SIM_NEED_PUK,
            SIM_LOCKED,
            SIM_FAIL,
            SIM_UNKNOWN,
            NONE
        } sim = SIM::SIM_UNKNOWN;

        enum class SelectedSIM
        {
            SIM1 = static_cast<int>(SIM::SIM1),
            SIM2 = static_cast<int>(SIM::SIM2),
            NONE
        } selected = SelectedSIM::SIM1;

        /// state of modem
        enum class Modem
        {
            OFF,             /// modem is off - it's not working
            ON_INITIALIZING, /// modem is set to on, just started - initialization not done yet
            ON_NEED_SIMFLOW, /// modem is on, initialized, no SIM initialization yet
            ON_INITIALIZED   /// modem is on, and it's fully initialized
        } modem = Modem::OFF;

        void setSignalStrength(const SignalStrength &newSignalStrength);
        [[nodiscard]] SignalStrength getSignalStrength() const;

        void setNetwork(const Network &network);
        [[nodiscard]] Network getNetwork() const;

        void setNetworkOperatorName(const std::string &newNetworkOperatorName);
        [[nodiscard]] std::string getNetworkOperatorName() const;

        void setTethering(const Tethering &newTethering);
        [[nodiscard]] Tethering getTethering() const;

        bool simCardInserted();

        static GSM *get();

      private:
        GSM() = default;
        SignalStrength signalStrength{};
        Network network{};
        std::string networkOperatorName{};
        Tethering tethering{};

        static cpp_freertos::MutexStandard mutex;
    };
} // namespace Store
