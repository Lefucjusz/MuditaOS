// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <EventStore.hpp>

#include <limits>

class SignalStrength
{
  public:
    explicit SignalStrength(int rssi);
    ~SignalStrength() = default;

    Store::SignalStrength data;

    void setRssi(int rssi);

    [[nodiscard]] bool isValid() const
    {
        return data.rssidBm != rssidBm_invalid;
    };

    /// @return 0 - if invalid
    static int rssiTodBm(int rssi);
    static Store::RssiBar rssidBmToBar(int rssidBm);

  private:
    // <rssi>   0           -113dBm or less
    //          1           -111dBm
    //          2...30      -109dBm...-53dBm
    //          31          -51dBm or greater
    //          99          Not known or not detectable
    //          100         -116dBm or less
    //          101         -115dBm
    //          102..190    -114dBm...-26dBm
    //          191         -25dBm or greater
    //          199         Not known or not detectable
    static constexpr auto rssi_low     = 0;
    static constexpr auto rssi_max     = 31;
    static constexpr auto rssi_low_dBm = -113;
    static constexpr auto rssi_max_dBm = -51;
    static constexpr auto rssi_invalid = 99;
    static constexpr auto rssi_step    = (rssi_low_dBm - rssi_max_dBm) / (rssi_max - rssi_low);

    static constexpr auto rssi_tdscmda_low     = 100;
    static constexpr auto rssi_tdscmda_max     = 191;
    static constexpr auto rssi_tdscmda_invalid = 199;
    static constexpr auto rssi_tdscmda_low_dBm = -116;
    static constexpr auto rssi_tdscmda_max_dBm = -25;
    static constexpr auto rssi_tdscmda_step =
        (rssi_tdscmda_low_dBm - rssi_tdscmda_max_dBm) / (rssi_tdscmda_max - rssi_low);

    static constexpr auto rssidBm_invalid          = 0;
    static constexpr auto rssidBm_four_bar_margin  = -60;
    static constexpr auto rssidBm_three_bar_margin = -75;
    static constexpr auto rssidBm_two_bar_margin   = -90;
    static constexpr auto rssidBm_one_bar_margin   = -100;
};
