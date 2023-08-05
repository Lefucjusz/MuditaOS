// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ArtistsModel.hpp"
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
    ArtistsModel::ArtistsModel(app::ApplicationCommon *app, std::shared_ptr<SongsRepository> songsRepository)
        : DatabaseModel(app), songsRepository(std::move(songsRepository))
    {}

    auto ArtistsModel::requestRecordsCount() -> unsigned
    {
        return recordsCount;
    }

    auto ArtistsModel::getMinimalItemSpaceRequired() const -> unsigned
    {
        return musicPlayerStyle::listItem::h + style::margins::small * 2;
    }

    auto ArtistsModel::requestRecords(std::uint32_t offset, std::uint32_t limit) -> void
    {
        songsRepository->getMusicArtistsWithMetadataList(
            offset, limit, [this](const auto &records, const auto newRecordsCount) {
                return onArtistsListRetrieved(records, newRecordsCount);
            });
    }

    auto ArtistsModel::getItem(gui::Order order) -> gui::ListItem *
    {
        const auto artist = getRecord(order);
        if (artist == nullptr) {
            return nullptr;
        }

        const auto item =
            new gui::FolderItem(artist->artist, createMinutesSecondsString(artist->totalLength), artist->songsCount);

        item->activatedCallback = [this, artist](gui::Item &) {
            if (onEnterPressedCallback == nullptr) {
                return false;
            }
            onEnterPressedCallback(*artist);
            return true;
        };

        return item;
    }

    auto ArtistsModel::createData(const OnActivateCallback &onActivateCallback) -> void
    {
        onEnterPressedCallback = onActivateCallback;
    }

    auto ArtistsModel::clearData() -> void
    {
        if (list != nullptr) {
            list->clear();
        }
    }

    auto ArtistsModel::updateRecords(std::vector<db::multimedia_files::ArtistWithMetadata> records) -> bool
    {
        DatabaseModel::updateRecords(std::move(records));
        list->onProviderDataUpdate();
        return true;
    }

    auto ArtistsModel::onArtistsListRetrieved(const std::vector<db::multimedia_files::ArtistWithMetadata> &records,
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
