// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/BasePresenter.hpp>
#include <models/ArtistsModel.hpp>

namespace app::music_player
{
    class ArtistsContract
    {
      public:
        class View
        {
          public:
            virtual ~View() noexcept = default;
        };

        class Presenter : public BasePresenter<ArtistsContract::View>
        {
          public:
            virtual ~Presenter() noexcept = default;

            virtual auto getModel() -> std::shared_ptr<app::music::ArtistsModel> = 0;
            virtual auto createData() -> void                                    = 0;
        };
    };

    class ArtistsPresenter : public ArtistsContract::Presenter
    {
      public:
        ArtistsPresenter(app::ApplicationCommon *app, std::shared_ptr<app::music::ArtistsModel> model);

        auto getModel() -> std::shared_ptr<app::music::ArtistsModel> override;
        auto createData() -> void override;

      private:
        app::ApplicationCommon *application = nullptr;
        std::shared_ptr<app::music::ArtistsModel> model;
    };
} // namespace app::music_player
