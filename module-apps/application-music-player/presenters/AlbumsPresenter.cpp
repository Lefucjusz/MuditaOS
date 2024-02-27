// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AlbumsPresenter.hpp"
#include <data/SongsListWindowData.hpp>
#include "application-music-player/ApplicationMusicPlayer.hpp"

namespace app::music_player
{
    AlbumsPresenter::AlbumsPresenter(app::ApplicationCommon *app, std::shared_ptr<app::music::AlbumsModel> model)
        : application(app), model(std::move(model))
    {}

    auto AlbumsPresenter::getModel() -> std::shared_ptr<app::music::AlbumsModel>
    {
        return model;
    }

    auto AlbumsPresenter::createData() -> void
    {
        model->createData([this](const db::multimedia_files::AlbumWithMetadata &albumWithMetadata) {
            const db::multimedia_files::Album album = {.artist = albumWithMetadata.artist,
                                                       .title  = albumWithMetadata.title};
            auto switchData                         = std::make_unique<gui::SongsListAlbumData>(album);
            application->switchWindow(gui::name::window::songs_list, std::move(switchData));
            return true;
        });
    }
} // namespace app::music_player
