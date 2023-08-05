// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ArtistsPresenter.hpp"
#include <data/SongsListWindowData.hpp>
#include "application-music-player/ApplicationMusicPlayer.hpp"

namespace app::music_player
{
    ArtistsPresenter::ArtistsPresenter(app::ApplicationCommon *app, std::shared_ptr<app::music::ArtistsModel> model)
        : application(app), model(std::move(model))
    {}

    auto ArtistsPresenter::getModel() -> std::shared_ptr<app::music::ArtistsModel>
    {
        return model;
    }

    auto ArtistsPresenter::createData() -> void
    {
        model->createData([this](const db::multimedia_files::ArtistWithMetadata &artistWithMetadata) {
            auto switchData = std::make_unique<gui::SongsListArtistData>(artistWithMetadata.artist);
            application->switchWindow(gui::name::window::songs_list, std::move(switchData));
            return true;
        });
    }
} // namespace app::music_player
