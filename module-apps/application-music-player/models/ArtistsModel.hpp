// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "SongsRepository.hpp"
#include <ListItemProvider.hpp>
#include <apps-common/DatabaseModel.hpp>
#include <module-db/Interface/MultimediaFilesRecord.hpp>

namespace app::music
{
    class ArtistsModel : public app::DatabaseModel<db::multimedia_files::ArtistWithMetadata>,
                         public gui::ListItemProvider
    {
      public:
        using OnActivateCallback = std::function<bool(const db::multimedia_files::ArtistWithMetadata &record)>;

        ArtistsModel(app::ApplicationCommon *app, std::shared_ptr<SongsRepository> songsRepository);

        [[nodiscard]] auto requestRecordsCount() -> unsigned override;
        [[nodiscard]] auto getMinimalItemSpaceRequired() const -> unsigned override;
        auto getItem(gui::Order order) -> gui::ListItem * override;
        auto requestRecords(std::uint32_t offset, std::uint32_t limit) -> void override;
        auto createData(OnActivateCallback &&onActivateCallback) -> void;
        auto clearData() -> void;

      private:
        auto updateRecords(std::vector<db::multimedia_files::ArtistWithMetadata> records) -> bool override;
        auto onArtistsListRetrieved(const std::vector<db::multimedia_files::ArtistWithMetadata> &records,
                                    unsigned newRecordsCount) -> bool;

        std::shared_ptr<SongsRepository> songsRepository;
        OnActivateCallback onEnterPressedCallback = nullptr;
    };
} // namespace app::music
