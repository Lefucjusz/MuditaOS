// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AlbumsModel.hpp"
#include "module-apps/application-music-player/data/MusicPlayerStyle.hpp"
#include "application-music-player/widgets/FolderItem.hpp"
#include <ListView.hpp>
#include <time/time_constants.hpp>

namespace
{
    std::string createMinutesSecondsString(std::uint32_t seconds)
    {
        const auto minutes          = seconds / utils::time::secondsInMinute;
        const auto secondsRemainder = seconds % utils::time::secondsInMinute;
        return std::to_string(minutes) + "m " + std::to_string(secondsRemainder) + "s";
    };
} // namespace

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

        const auto item =
            new gui::FolderItem(album->title, createMinutesSecondsString(album->totalLength), album->songsCount);

        item->activatedCallback = [this, album](gui::Item &) {
            if (onEnterPressedCallback == nullptr) {
                return false;
            }
            onEnterPressedCallback(*album);
            return true;
        };

        return item;
    }

    auto AlbumsModel::createData(const OnActivateCallback &onActivateCallback) -> void
    {
        onEnterPressedCallback = onActivateCallback;
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
