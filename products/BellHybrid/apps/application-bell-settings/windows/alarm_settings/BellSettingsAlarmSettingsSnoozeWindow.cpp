// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include "BellSettingsAlarmSettingsSnoozeWindow.hpp"

#include "BellSettingsAlarmSettingsMenuWindow.hpp"
#include "models/alarm_settings/SnoozeListItemProvider.hpp"
#include "presenter/alarm_settings/SnoozePresenter.hpp"

#include <apps-common/ApplicationCommon.hpp>
#include <common/windows/BellFinishedWindow.hpp>
#include <popups/data/AudioErrorParams.hpp>
#include <service-appmgr/Controller.hpp>
#include <gui/input/InputEvent.hpp>
#include <widgets/SideListView.hpp>

namespace gui
{

    BellSettingsAlarmSettingsSnoozeWindow::BellSettingsAlarmSettingsSnoozeWindow(
        app::ApplicationCommon *app, std::unique_ptr<app::bell_settings::SnoozePresenter> presenter)
        : AppWindow(app, name), presenter{std::move(presenter)}
    {
        this->presenter->attach(this);
        buildInterface();
    }

    void BellSettingsAlarmSettingsSnoozeWindow::buildInterface()
    {
        AppWindow::buildInterface();
        statusBar->setVisible(false);
        header->setTitleVisibility(false);
        navBar->setVisible(false);

        listView = new SideListView(
            this, 0U, 0U, this->getWidth(), this->getHeight(), presenter->getPagesProvider(), PageBarType::None);
        listView->setEdges(RectangleEdge::None);

        listView->rebuildList(listview::RebuildType::Full);

        presenter->loadData();
    }

    void BellSettingsAlarmSettingsSnoozeWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        setFocusItem(listView);
    }

    bool BellSettingsAlarmSettingsSnoozeWindow::onInput(const InputEvent &inputEvent)
    {
        if (listView->onInput(inputEvent)) {
            return true;
        }
        if (inputEvent.isShortRelease(KeyCode::KEY_ENTER)) {
            exit();
            return true;
        }

        if (inputEvent.isShortRelease(KeyCode::KEY_RF)) {
            presenter->exitWithoutSave();
        }

        return AppWindow::onInput(inputEvent);
    }

    void BellSettingsAlarmSettingsSnoozeWindow::rebuild()
    {
        erase();
        buildInterface();
    }

    void BellSettingsAlarmSettingsSnoozeWindow::exit()
    {
        presenter->saveData();
        application->switchWindow(
            window::bell_finished::defaultName,
            BellFinishedWindowData::Factory::create("circle_success_big", BellSettingsAlarmSettingsMenuWindow::name));
    }

    void BellSettingsAlarmSettingsSnoozeWindow::deepRefresh()
    {
        getApplication()->render(gui::RefreshModes::GUI_REFRESH_DEEP);
    }

    void BellSettingsAlarmSettingsSnoozeWindow::onClose(Window::CloseReason reason)
    {
        if (reason != CloseReason::Popup) {
            presenter->eraseProviderData();
        }
    }

    void BellSettingsAlarmSettingsSnoozeWindow::handleError()
    {
        auto switchData = std::make_unique<AudioErrorParams>(AudioErrorType::UnsupportedMediaType);
        app::manager::Controller::sendAction(application, app::manager::actions::ShowPopup, std::move(switchData));
    }

    void BellSettingsAlarmSettingsSnoozeWindow::handleDeletedFile()
    {
        auto switchData = std::make_unique<AudioErrorParams>(AudioErrorType::FileDeleted);
        app::manager::Controller::sendAction(application, app::manager::actions::ShowPopup, std::move(switchData));
    }
} // namespace gui
