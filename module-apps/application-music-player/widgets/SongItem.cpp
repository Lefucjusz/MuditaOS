// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "module-apps/application-music-player/widgets/SongItem.hpp"
#include <gui/widgets/text/Label.hpp>
#include <i18n/i18n.hpp>

namespace gui
{

    using namespace musicPlayerStyle;

    SongItem::SongItem(const std::string &authorName,
                       const std::string &songName,
                       const std::string &duration,
                       const std::function<void(const UTF8 &)> &setBtBarCallback,
                       const std::function<void()> &restoreBtBarCallback)
        : navBarTemporaryMode(setBtBarCallback), navBarRestoreFromTemporaryMode(restoreBtBarCallback)
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

        durationText = new TextFixedSize(firstHBox, 0, 0, 0, 0);
        durationText->setMinimumWidthToFitText(duration);
        durationText->setMinimumHeight(listItem::text_h);
        durationText->setMargins(Margins(0, 0, listItem::rightMargin, 0));
        durationText->setEdges(RectangleEdge::None);
        durationText->drawUnderline(false);
        durationText->setFont(style::window::font::verysmall);
        durationText->setAlignment(Alignment(gui::Alignment::Horizontal::Right, gui::Alignment::Vertical::Center));
        durationText->setEditMode(EditMode::Browse);
        durationText->setText(duration);

        songText = new Label(firstHBox, 0, 0, 0, 0);
        songText->setMinimumHeight(listItem::bold_text_h);
        songText->setMaximumWidth(listItem::w);
        songText->setMargins(Margins(listItem::leftMargin, 0, 0, 0));
        songText->setEdges(RectangleEdge::None);
        songText->setFont(style::window::font::bigbold);
        songText->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        songText->setText(songName);

        playedSong = new Image(secondHBox, 0, 0, "");
        playedSong->setAlignment(Alignment(gui::Alignment::Horizontal::Right, gui::Alignment::Vertical::Center));
        playedSong->setVisible(false);

        authorText = new Label(secondHBox, 0, 0, 0, 0);
        authorText->setMinimumHeight(listItem::text_h);
        authorText->setMaximumWidth(listItem::w);
        authorText->setMargins(Margins(listItem::leftMargin, 0, 0, 0));
        authorText->setEdges(RectangleEdge::None);
        authorText->setFont(style::window::font::medium);
        authorText->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        authorText->setText(authorName);

        dimensionChangedCallback = [&]([[maybe_unused]] gui::Item &item, const BoundingBox &newDim) -> bool {
            vBox->setArea({0, 0, newDim.w, newDim.h});
            return true;
        };

        focusChangedCallback = [&](gui::Item &item) {
            if (item.focus) {
                std::string bottomBarText;
                switch (itemState) {
                case ItemState::Playing:
                    bottomBarText = utils::translate("common_pause");
                    break;
                case ItemState::Paused:
                    bottomBarText = utils::translate("common_resume");
                    break;
                case ItemState::None:
                    bottomBarText = utils::translate("common_play");
                    break;
                }
                if (navBarTemporaryMode != nullptr) {
                    navBarTemporaryMode(bottomBarText);
                }
            }
            else {
                setFocusItem(nullptr);
                if (navBarRestoreFromTemporaryMode != nullptr) {
                    navBarRestoreFromTemporaryMode();
                }
            }
            setState(itemState);
            return true;
        };
    }

    void SongItem::setState(ItemState state)
    {
        itemState = state;
        switch (state) {
        case ItemState::Paused:
            playedSong->set("mp_nowplaying_paused_list", ImageTypeSpecifier::W_M);
            playedSong->setVisible(true);
            break;
        case ItemState::Playing:
            playedSong->set("mp_nowplaying_play_list", ImageTypeSpecifier::W_M);
            playedSong->setVisible(true);
            break;
        case ItemState::None:
            playedSong->set("");
            playedSong->setVisible(false);
            break;
        }
        secondHBox->resizeItems();
    }
} /* namespace gui */
