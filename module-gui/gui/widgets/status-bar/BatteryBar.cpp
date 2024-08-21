// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "BatteryBar.hpp"
#include "Style.hpp"
#include <Image.hpp>

namespace gui::status_bar
{
    namespace
    {
        constexpr auto batteryLow           = "battery_low";
        constexpr auto batteryCharging      = "battery_charging";
        constexpr auto batteryChargingReady = "battery_charging_ready";
        constexpr auto battery1             = "battery_1";
        constexpr auto battery2             = "battery_2";
        constexpr auto battery3             = "battery_3";
        constexpr auto battery4             = "battery_4";
        constexpr auto battery5             = "battery_5";

        constexpr auto level1Threshold = 15;
        constexpr auto level2Threshold = 30;
        constexpr auto level3Threshold = 50;
        constexpr auto level4Threshold = 70;
        constexpr auto level5Threshold = 85;
    } // namespace

    BatteryBar::BatteryBar(Item *parent, std::uint32_t x, std::uint32_t y, std::uint32_t w, std::uint32_t h)
        : BatteryBase(parent, x, y, w, h)
    {
        img = new Image(this, battery1, style::status_bar::imageTypeSpecifier);

        setMinimumSize(img->getWidth(), style::status_bar::height);
    }

    void BatteryBar::showBatteryLevel(std::uint32_t percentage)
    {
        if (percentage < level1Threshold) {
            img->set(batteryLow, style::status_bar::imageTypeSpecifier);
        }
        else if (percentage < level2Threshold) {
            img->set(battery1, style::status_bar::imageTypeSpecifier);
        }
        else if (percentage < level3Threshold) {
            img->set(battery2, style::status_bar::imageTypeSpecifier);
        }
        else if (percentage < level4Threshold) {
            img->set(battery3, style::status_bar::imageTypeSpecifier);
        }
        else if (percentage < level5Threshold) {
            img->set(battery4, style::status_bar::imageTypeSpecifier);
        }
        else {
            img->set(battery5, style::status_bar::imageTypeSpecifier);
        }
    }

    void BatteryBar::showBatteryCharging()
    {
        img->set(batteryCharging, style::status_bar::imageTypeSpecifier);
    }

    void BatteryBar::showBatteryChargingDone()
    {
        img->set(batteryChargingReady, style::status_bar::imageTypeSpecifier);
    }
} // namespace gui::status_bar
