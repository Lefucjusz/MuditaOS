// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "FolderItem.hpp"
#include "application-music-player/data/MusicPlayerStyle.hpp"
#include <gui/widgets/text/Label.hpp>
#include <i18n/i18n.hpp>

namespace gui
{
    using namespace musicPlayerStyle;

    FolderItem::FolderItem(const std::string &albumName, const std::string &duration, unsigned songsCount)
    {
        setMinimumSize(listItem::w, listItem::h);
        setMargins(Margins(0, style::margins::small, 0, style::margins::small));

        vBox = new VBox(this, 0, 0, 0, 0);
        vBox->setEdges(RectangleEdge::None);

        firstHBox = new HBox(vBox, 0, 0, 0, 0);
        firstHBox->setMinimumSize(listItem::w, listItem::bold_text_h);
        firstHBox->setMargins(Margins(0, listItem::topMargin, 0, 0));
        firstHBox->setReverseOrder(true);
        firstHBox->setEdges(RectangleEdge::None);

        secondHBox = new HBox(vBox, 0, 0, 0, 0);
        secondHBox->setMinimumSize(listItem::w, listItem::text_h);
        secondHBox->setMargins(Margins(0, listItem::topMargin, 0, 0));
        secondHBox->setReverseOrder(true);
        secondHBox->setEdges(RectangleEdge::None);

        albumNameText = new Label(firstHBox, 0, 0, 0, 0);
        albumNameText->setMinimumHeight(listItem::bold_text_h);
        albumNameText->setMaximumWidth(listItem::w);
        albumNameText->setMargins(Margins(listItem::leftMargin, 0, 0, 0));
        albumNameText->setEdges(RectangleEdge::None);
        albumNameText->setFont(style::window::font::bigbold);
        albumNameText->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        albumNameText->setText(albumName);

        durationText = new Label(secondHBox, 0, 0, 0, 0);
        durationText->setMinimumWidthToFitText(duration);
        durationText->setMinimumHeight(listItem::text_h);
        durationText->setMargins(Margins(0, 0, listItem::rightMargin, 0));
        durationText->setEdges(RectangleEdge::None);
        durationText->setFont(style::window::font::verysmall);
        durationText->setAlignment(Alignment(gui::Alignment::Horizontal::Right, gui::Alignment::Vertical::Center));
        durationText->setEditMode(EditMode::Browse);
        durationText->setText(duration);

        tracksCountText = new Label(secondHBox, 0, 0, 0, 0);
        tracksCountText->setMinimumWidthToFitText(duration);
        tracksCountText->setMinimumHeight(listItem::text_h);
        tracksCountText->setMaximumWidth(listItem::w);
        tracksCountText->setMargins(Margins(listItem::leftMargin, 0, 0, 0));
        tracksCountText->setEdges(RectangleEdge::None);
        tracksCountText->setFont(style::window::font::medium);
        tracksCountText->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        tracksCountText->setText(utils::translate("app_music_player_tracks") + ": " + std::to_string(songsCount));

        dimensionChangedCallback = [&]([[maybe_unused]] gui::Item &item, const BoundingBox &newDim) -> bool {
            vBox->setArea({0, 0, newDim.w, newDim.h});
            return true;
        };
    }
} // namespace gui
