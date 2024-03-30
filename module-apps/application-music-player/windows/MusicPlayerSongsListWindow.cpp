// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MusicPlayerSongsListWindow.hpp"
#include <application-music-player/ApplicationMusicPlayer.hpp>
#include <data/MusicPlayerStyle.hpp>
#include <data/SongsListWindowData.hpp>

#include <Style.hpp>
#include <i18n/i18n.hpp>
#include <gui/widgets/ListView.hpp>
#include <gui/widgets/Icon.hpp>

namespace gui
{
    MusicPlayerSongsListWindow::MusicPlayerSongsListWindow(
        app::ApplicationCommon *app, std::shared_ptr<app::music_player::SongsContract::Presenter> presenter)
        : AppWindow(app, gui::name::window::songs_list), presenter(std::move(presenter))
    {
        buildInterface();
    }

    auto MusicPlayerSongsListWindow::buildInterface() -> void
    {
        presenter->attach(this);
        AppWindow::buildInterface();

        navBar->setText(nav_bar::Side::Right, utils::translate(style::strings::common::back));

        songsList = new gui::ListView(this,
                                      musicPlayerStyle::listWindow::x,
                                      musicPlayerStyle::listWindow::y,
                                      musicPlayerStyle::listWindow::w,
                                      musicPlayerStyle::listWindow::h,
                                      presenter->getModel(),
                                      listview::ScrollBarType::Fixed);
        songsList->setBoundaries(Boundaries::Continuous);

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

        songsList->emptyListCallback    = [this]() { emptyListIcon->setVisible(true); };
        songsList->notEmptyListCallback = [this]() { emptyListIcon->setVisible(false); };

        setFocusItem(songsList);
    }

    auto MusicPlayerSongsListWindow::onBeforeShow([[maybe_unused]] ShowMode mode, SwitchData *data) -> void
    {
//        presenter->attach(this);

        if (mode == ShowMode::GUI_SHOW_RETURN) {
            return;
        }

        std::string title;
        if (const auto albumData = dynamic_cast<SongsListAlbumData *>(data); albumData != nullptr) {
            const auto &album = albumData->getAlbum();
            title             = album.title.empty() ? utils::translate("app_music_player_unknown_album") : album.title;
            presenter->setCurrentlyViewedAlbum(album);
        }
        else if (const auto artistData = dynamic_cast<SongsListArtistData *>(data); artistData != nullptr) {
            const auto &artist = artistData->getArtist();
            title              = artist.empty() ? utils::translate("app_music_player_unknown_artist") : artist;
            presenter->setCurrentlyViewedArtist(artist);
        }
        else {
            title = utils::translate("app_music_player_all_songs");
            presenter->setAllSongsView();
        }

        setTitle(title);
        presenter->createData();
    }

    auto MusicPlayerSongsListWindow::updateSongsState(
        [[maybe_unused]] std::optional<db::multimedia_files::MultimediaFilesRecord> record,
        [[maybe_unused]] RecordState state) -> void
    {
        songsList->rebuildList(gui::listview::RebuildType::InPlace);
    }

    auto MusicPlayerSongsListWindow::updateSongProgress([[maybe_unused]] float progress) -> void
    {}

    auto MusicPlayerSongsListWindow::refreshWindow() -> void
    {
        application->refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
    }

    auto MusicPlayerSongsListWindow::setNavBarTemporaryMode(const std::string &text) -> void
    {
        navBarTemporaryMode(text, nav_bar::Side::Center, false);
    }

    auto MusicPlayerSongsListWindow::restoreFromNavBarTemporaryMode() -> void
    {
        navBarRestoreFromTemporaryMode();
    }

    auto MusicPlayerSongsListWindow::onInput(const InputEvent &inputEvent) -> bool
    {
        return AppWindow::onInput(inputEvent);
    }
} // namespace gui
