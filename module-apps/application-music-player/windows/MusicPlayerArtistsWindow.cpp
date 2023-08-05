// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MusicPlayerArtistsWindow.hpp"
#include <application-music-player/ApplicationMusicPlayer.hpp>
#include <data/MusicPlayerStyle.hpp>
#include <ListView.hpp>
#include <Icon.hpp>

namespace gui
{
    MusicPlayerArtistsWindow::MusicPlayerArtistsWindow(
        app::ApplicationCommon *app, std::shared_ptr<app::music_player::ArtistsContract::Presenter> presenter)
        : AppWindow(app, gui::name::window::artists), presenter(std::move(presenter))
    {
        buildInterface();
    }

    void MusicPlayerArtistsWindow::buildInterface()
    {
        presenter->attach(this);
        presenter->createData();

        AppWindow::buildInterface();

        setTitle(utils::translate("app_music_player_artists"));
        navBar->setText(nav_bar::Side::Right, utils::translate(style::strings::common::back));
        navBar->setText(nav_bar::Side::Center, utils::translate("common_select"));

        albumsList = new gui::ListView(this,
                                       musicPlayerStyle::listWindow::x,
                                       musicPlayerStyle::listWindow::y,
                                       musicPlayerStyle::listWindow::w,
                                       musicPlayerStyle::listWindow::h,
                                       presenter->getModel(),
                                       listview::ScrollBarType::Fixed);
        albumsList->setBoundaries(Boundaries::Continuous);

        emptyListIcon = new gui::Icon(this,
                                      style::window::default_left_margin,
                                      musicPlayerStyle::mainWindow::musicLibraryScreen::emptyLibraryNoteMargin,
                                      style::window::default_body_width,
                                      style::window::default_body_height,
                                      "mp_note",
                                      utils::translate("app_music_player_music_empty_window_notification"),
                                      musicPlayerStyle::common::imageType);
        emptyListIcon->setVisible(false);
        emptyListIcon->setAlignment(Alignment::Horizontal::Center);

        albumsList->emptyListCallback = [this]() {
            emptyListIcon->setVisible(true);
            navBar->setActive(nav_bar::Side::Center, false);
        };

        albumsList->notEmptyListCallback = [this]() {
            emptyListIcon->setVisible(false);
            navBar->setActive(nav_bar::Side::Center, true);
        };

        albumsList->rebuildList(listview::RebuildType::InPlace);
        setFocusItem(albumsList);
    }

    void MusicPlayerArtistsWindow::onBeforeShow([[maybe_unused]] ShowMode mode, [[maybe_unused]] SwitchData *data)
    {}

    bool MusicPlayerArtistsWindow::onInput(const InputEvent &inputEvent)
    {
        return AppWindow::onInput(inputEvent);
    }
} /* namespace gui */
