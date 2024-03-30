// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AlbumsModel.hpp"
#include "data/MusicPlayerStyle.hpp"
#include "widgets/FolderItem.hpp"
#include "utils/Utils.hpp"
#include <ListView.hpp>

namespace app::music
{
    AlbumsModel::AlbumsModel(app::ApplicationCommon *app, std::shared_ptr<SongsRepository> songsRepository)
        : DatabaseModel(app), songsRepository(std::move(songsRepository))
    {}

    auto AlbumsModel::requestRecordsCount() -> unsigned
    {
        return recordsCount;
    }

    auto AlbumsModel::getMinimalItemSpaceRequired() const -> unsigned
    {
        return musicPlayerStyle::listItem::h + style::margins::small * 2;
    }

    auto AlbumsModel::requestRecords(std::uint32_t offset, std::uint32_t limit) -> void
    {
        songsRepository->getMusicAlbumsWithMetadataList(
            offset, limit, [this](const auto &records, const auto newRecordsCount) {
                return onAlbumsListRetrieved(records, newRecordsCount);
            });
    }

    auto AlbumsModel::getItem(gui::Order order) -> gui::ListItem *
    {
        const auto album = getRecord(order);
        if (album == nullptr) {
            return nullptr;
        }

        const auto &albumTitle =
            album->title.empty() ? utils::translate("app_music_player_unknown_album") : album->title;
        const auto item =
            new gui::FolderItem(albumTitle, utils::createMinutesSecondsString(album->totalLength), album->songsCount);

        item->activatedCallback = [this, album](gui::Item &) {
            if (onEnterPressedCallback == nullptr) {
                return false;
            }
            onEnterPressedCallback(*album);
            return true;
        };

        return item;
    }

    auto AlbumsModel::createData(OnActivateCallback &&onActivateCallback) -> void
    {
        onEnterPressedCallback = std::move(onActivateCallback);
    }

    auto AlbumsModel::clearData() -> void
    {
        if (list != nullptr) {
            list->clear();
        }
    }

    auto AlbumsModel::updateRecords(std::vector<db::multimedia_files::AlbumWithMetadata> records) -> bool
    {
        DatabaseModel::updateRecords(std::move(records));
        list->onProviderDataUpdate();
        return true;
    }

    auto AlbumsModel::onAlbumsListRetrieved(const std::vector<db::multimedia_files::AlbumWithMetadata> &records,
                                            unsigned newRecordsCount) -> bool
    {
        if (list != nullptr && recordsCount != newRecordsCount) {
            recordsCount = newRecordsCount;
            list->reSendLastRebuildRequest();
            return false;
        }
        return updateRecords(records);
    }
} // namespace app::music
