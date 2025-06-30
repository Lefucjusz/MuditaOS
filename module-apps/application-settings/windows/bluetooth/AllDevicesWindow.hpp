// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
#pragma once

#include <application-settings/windows/BaseSettingsWindow.hpp>
#include <application-settings/models/bluetooth/BluetoothSettingsModel.hpp>

namespace gui
{
    class Image;

    class AllDevicesWindow : public BaseSettingsWindow
    {
      public:
        AllDevicesWindow(app::ApplicationCommon *app, std::shared_ptr<BluetoothSettingsModel> bluetoothSettingsModel);

      private:
        auto buildInterface() -> void override;
        auto buildOptionsList() -> std::list<Option> override;
        auto onBeforeShow(ShowMode mode, SwitchData *data) -> void override;
        auto onInput(const InputEvent &inputEvent) -> bool override;
        [[nodiscard]] auto getTextOnCenter(const DeviceState &state) const -> UTF8;
        [[nodiscard]] auto getTextOnRight(const DeviceState &state) const -> UTF8;
        [[nodiscard]] auto getRightItem(const DeviceState &state) const -> option::SettingRightItem;
        auto handleDeviceAction(const Devicei &device) -> bool;

        std::shared_ptr<BluetoothSettingsModel> bluetoothSettingsModel{};
        OptionWindowDestroyer rai_destroyer = OptionWindowDestroyer(*this);
    };
} // namespace gui
