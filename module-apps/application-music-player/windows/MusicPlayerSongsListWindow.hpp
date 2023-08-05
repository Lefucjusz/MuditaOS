// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <application-music-player/presenters/SongsPresenter.hpp>
#include <AppWindow.hpp>
#include <TextFixedSize.hpp>

namespace gui
{
    class ListView;
    class Icon;

    class MusicPlayerSongsListWindow : public AppWindow, public app::music_player::SongsContract::View
    {
      public:
        MusicPlayerSongsListWindow(app::ApplicationCommon *app,
                                   std::shared_ptr<app::music_player::SongsContract::Presenter> presenter);

        auto onBeforeShow([[maybe_unused]] ShowMode mode, [[maybe_unused]] SwitchData *data) -> void override;

        auto buildInterface() -> void override;
        auto onInput(const InputEvent &inputEvent) -> bool override;

        auto updateSongsState(std::optional<db::multimedia_files::MultimediaFilesRecord> record, RecordState state)
            -> void override;
        auto updateSongProgress(float progress) -> void override;

        auto refreshWindow() -> void override;
        auto setNavBarTemporaryMode(const std::string &text) -> void override;
        auto restoreFromNavBarTemporaryMode() -> void override;

      private:
        std::shared_ptr<app::music_player::SongsContract::Presenter> presenter;
        ListView *songsList = nullptr;
        Icon *emptyListIcon = nullptr;
    };
} // namespace gui
