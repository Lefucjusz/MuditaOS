# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

add_library(application-music-player STATIC)

target_include_directories(application-music-player
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_sources(application-music-player
    PRIVATE
        ApplicationMusicPlayer.cpp
        AudioNotificationsHandler.cpp
        models/SongsModel.cpp
        models/AlbumsModel.cpp
        models/SongsRepository.cpp
        models/SongsCache.cpp
        models/ArtistsModel.cpp
        presenters/SongsPresenter.cpp
        presenters/AlbumsPresenter.cpp
        presenters/ArtistsPresenter.cpp
        widgets/SongItem.cpp
        widgets/FolderItem.cpp
        windows/MusicPlayerMainWindow.cpp
        windows/MusicPlayerAlbumsWindow.cpp
        windows/MusicPlayerSongsListWindow.cpp
        windows/MusicPlayerArtistsWindow.cpp
    PRIVATE
        AudioNotificationsHandler.hpp
        data/MusicPlayerStyle.hpp
        data/SongsListWindowData.hpp
        utils/Utils.hpp
        models/SongsModel.hpp
        models/AlbumsModel.hpp
        models/SongsRepository.hpp
        models/SongsCache.hpp
        models/ArtistsModel.cpp
        presenters/SongsPresenter.hpp
        presenters/AlbumsPresenter.hpp
        presenters/ArtistsPresenter.hpp
        widgets/SongItem.hpp
        widgets/FolderItem.hpp
        windows/MusicPlayerMainWindow.hpp
        windows/MusicPlayerAlbumsWindow.hpp
        windows/MusicPlayerSongsListWindow.hpp
        windows/MusicPlayerArtistsWindow.hpp
    PUBLIC
        include/application-music-player/ApplicationMusicPlayer.hpp
)

target_link_libraries(application-music-player
    PRIVATE
        app
        i18n
        log
        module-gui
        service-audio
        utils-time
    PUBLIC
        module-audio
)

if (${ENABLE_TESTS})
    add_subdirectory(tests)
endif()
