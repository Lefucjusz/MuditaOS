// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <application-music-player/presenters/ArtistsPresenter.hpp>
#include <AppWindow.hpp>
#include <TextFixedSize.hpp>

namespace gui
{
    class ListView;
    class Icon;

    class MusicPlayerArtistsWindow : public AppWindow, public app::music_player::ArtistsContract::View
    {
      public:
        MusicPlayerArtistsWindow(app::ApplicationCommon *app,
                                 std::shared_ptr<app::music_player::ArtistsContract::Presenter> presenter);

        void onBeforeShow(ShowMode mode, SwitchData *data) override;

        void buildInterface() override;
        bool onInput(const InputEvent &inputEvent) override;

      private:
        ListView *albumsList = nullptr;
        Icon *emptyListIcon  = nullptr;

        std::shared_ptr<app::music_player::ArtistsContract::Presenter> presenter;
    };
} /* namespace gui */
