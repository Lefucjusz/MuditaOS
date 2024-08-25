// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "module-apps/application-music-player/data/MusicPlayerStyle.hpp"
#include <apps-common/models/SongContext.hpp>
#include <Audio/decoder/Decoder.hpp>
#include <apps-common/ApplicationCommon.hpp>
#include <apps-common/DatabaseModel.hpp>
#include <module-db/Interface/MultimediaFilesRecord.hpp>
#include "SongsRepository.hpp"

namespace app::music
{
    class SongsModel : public app::DatabaseModel<db::multimedia_files::MultimediaFilesRecord>,
                       public gui::ListItemProvider
    {
      public:
        using OnShortReleaseCallback           = std::function<bool(const std::string &fileName)>;
        using OnLongPressCallback              = std::function<void()>;
        using OnSetNavBarTemporaryCallback     = std::function<void(const UTF8 &)>;
        using OnRestoreNavBarTemporaryCallback = std::function<void()>;

        SongsModel(app::ApplicationCommon *app, std::shared_ptr<SongsRepository> songsRepository);

        void createData(OnShortReleaseCallback &&shortReleaseCb,
                        OnLongPressCallback &&longPressCb,
                        OnSetNavBarTemporaryCallback &&navBarTemporaryModeCb,
                        OnRestoreNavBarTemporaryCallback &&navBarRestoreFromTemporaryModeCb);

        [[nodiscard]] auto requestRecordsCount() -> unsigned override;
        [[nodiscard]] auto getMinimalItemSpaceRequired() const -> unsigned override;
        auto getItem(gui::Order order) -> gui::ListItem * override;
        auto requestRecords(std::uint32_t offset, std::uint32_t limit) -> void override;

        auto setCurrentlyViewedAlbum(const db::multimedia_files::Album &album) -> void;
        auto setCurrentlyViewedArtist(const db::multimedia_files::Artist &artist) -> void;
        auto setAllSongsView() -> void;

        auto initRepository() -> void;
        auto updateRepository(const std::string &filePath) -> void;

        [[nodiscard]] auto getNextFilePath(const std::string &filePath) const -> std::string;
        [[nodiscard]] auto getPreviousFilePath(const std::string &filePath) const -> std::string;

        [[nodiscard]] auto isSongPlaying() const noexcept -> bool;
        auto setCurrentSongState(SongState songState) noexcept -> void;
        [[nodiscard]] auto getCurrentFileToken() const noexcept -> std::optional<audio::Token>;
        [[nodiscard]] auto getActivatedRecord() const noexcept
            -> std::optional<db::multimedia_files::MultimediaFilesRecord>;

        [[nodiscard]] auto getCurrentSongContext() const noexcept -> SongContext;
        auto setCurrentSongContext(const SongContext &context) -> void;
        auto clearCurrentSongContext() -> void;

        auto clearData() -> void;

      private:
        auto onMusicListRetrieved(const std::vector<db::multimedia_files::MultimediaFilesRecord> &records,
                                  unsigned repoRecordsCount) -> bool;
        [[nodiscard]] auto updateRecords(std::vector<db::multimedia_files::MultimediaFilesRecord> records)
            -> bool override;

        auto checkIfViewedMatchPlayed() -> bool;

        SongContext songContext;
        std::optional<db::multimedia_files::MultimediaFilesRecord> activatedRecord;

        std::optional<db::multimedia_files::Album> currentlyViewedAlbum;
        std::optional<db::multimedia_files::Artist> currentlyViewedArtist;

        std::shared_ptr<SongsRepository> songsRepository;

        OnShortReleaseCallback shortReleaseCallback                     = nullptr;
        OnLongPressCallback longPressCallback                           = nullptr;
        OnSetNavBarTemporaryCallback navBarTemporaryMode                = nullptr;
        OnRestoreNavBarTemporaryCallback navBarRestoreFromTemporaryMode = nullptr;

        std::uint32_t currentListOffset = 0;
    };
} // namespace app::music
