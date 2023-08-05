// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-gui/gui/Common.hpp>
#include <Style.hpp>

namespace musicPlayerStyle
{
    namespace common
    {
        constexpr gui::ImageTypeSpecifier imageType = gui::ImageTypeSpecifier::W_G;
    }

    namespace mainWindow
    {
        namespace startScreen
        {
            constexpr std::uint32_t noteUpMargin            = 100;
            constexpr std::uint32_t noteSize                = 132;
            constexpr std::uint32_t noteDownMargin          = 28;
            constexpr std::uint32_t descriptionWidth        = 400;
            constexpr std::uint32_t descriptionHeight       = 66;
            constexpr std::uint32_t descriptionBottomMargin = 40;
            constexpr std::uint32_t buttonsBottomMargin     = 40;
        } // namespace startScreen

        namespace trackInfoScreen
        {
            constexpr std::uint32_t topMargin      = 80;
            constexpr std::uint32_t titleWidth     = 400;
            constexpr std::uint32_t titleHeight    = 40;
            constexpr std::uint32_t internalMargin = 16;
            constexpr std::uint32_t albumHeight    = 30;
            constexpr std::uint32_t artistHeight   = 30;
            constexpr std::uint32_t swipeBarMargin = 65;
        } // namespace trackInfoScreen

        namespace musicLibraryScreen
        {
            constexpr std::uint32_t topArrowMargin         = 15;
            constexpr std::uint32_t emptyLibraryNoteMargin = 165;
            constexpr std::uint32_t bottomArrowMargin      = 15;
            constexpr std::uint32_t optionListHeight       = 280;
        } // namespace musicLibraryScreen

        namespace playButtons
        {
            constexpr std::uint32_t width          = 330;
            constexpr std::uint32_t height         = 50;
            constexpr std::uint32_t internalMargin = 10;
        } // namespace playButtons

        namespace lineArrow
        {
            constexpr std::uint32_t textWidth      = 240;
            constexpr std::uint32_t textHeight     = 30;
            constexpr std::uint32_t internalMargin = 12;
        } // namespace lineArrow

        namespace trackInfo
        {
            constexpr std::uint32_t topMargin      = 125;
            constexpr std::uint32_t width          = 430;
            constexpr std::uint32_t height         = 45;
            constexpr std::uint32_t internalMargin = 24;
        } // namespace trackInfo

        namespace trackProgress
        {
            constexpr std::uint32_t topMargin         = 44;
            constexpr std::uint32_t bottomMargin      = 38;
            constexpr std::uint32_t height            = 60;
            constexpr std::uint32_t barWidth          = 419;
            constexpr std::uint32_t barThickness      = 3;
            constexpr std::uint32_t barHeight         = 15;
            constexpr std::uint32_t bottomHeight      = 45;
            constexpr std::uint32_t descriptionWidth  = 40;
            constexpr std::uint32_t descriptionHeight = 24;

        } // namespace trackProgress
    }     // namespace mainWindow

    namespace listWindow
    {
        constexpr std::uint32_t x = style::window::default_left_margin;
        // Magic 1 -> discussed with Design for proper alignment.
        constexpr std::uint32_t y = style::window::default_vertical_pos - 1;
        constexpr std::uint32_t w = style::listview::body_width_with_scroll;
        // Bottom margin need to be added to fit all elements.
        constexpr std::uint32_t h = style::window_height - y - style::nav_bar::height + style::margins::small;
    } // namespace albumsWindow

    namespace listItem
    {
        constexpr std::uint32_t w = style::window::default_body_width;
        constexpr std::uint32_t h = 100;

        constexpr std::uint32_t bold_text_h = 33;
        constexpr std::uint32_t text_h      = 33;

        constexpr std::int32_t topMargin   = 16;
        constexpr std::int32_t leftMargin  = 10;
        constexpr std::int32_t rightMargin = 4;
    } // namespace listItem
} // namespace musicPlayerStyle
