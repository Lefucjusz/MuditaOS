// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <ListItem.hpp>
#include <Text.hpp>
#include <Image.hpp>

namespace gui
{
    class FolderItem : public ListItem
    {
      public:
        FolderItem(const std::string &albumName, const std::string &duration, unsigned songsCount);

      private:
        VBox *vBox             = nullptr;
        HBox *firstHBox        = nullptr;
        HBox *secondHBox       = nullptr;
        Label *albumNameText   = nullptr;
        Label *tracksCountText = nullptr;
        Label *durationText    = nullptr;
    };
} // namespace gui
