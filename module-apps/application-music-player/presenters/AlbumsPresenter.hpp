// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/BasePresenter.hpp>
#include <models/AlbumsModel.hpp>

namespace app::music_player
{
    class AlbumsContract
    {
      public:
        class View
        {
          public:
            virtual ~View() noexcept = default;
        };

        class Presenter : public BasePresenter<AlbumsContract::View>
        {
          public:
            virtual ~Presenter() noexcept = default;

            virtual auto getModel() -> std::shared_ptr<app::music::AlbumsModel> = 0;
            virtual auto createData() -> void                                   = 0;
        };
    };

    class AlbumsPresenter : public AlbumsContract::Presenter
    {
      public:
        AlbumsPresenter(app::ApplicationCommon *app, std::shared_ptr<app::music::AlbumsModel> model);

        auto getModel() -> std::shared_ptr<app::music::AlbumsModel> override;
        auto createData() -> void override;

      private:
        app::ApplicationCommon *application = nullptr;
        std::shared_ptr<app::music::AlbumsModel> model;
    };
} // namespace app::music_player
