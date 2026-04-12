#pragma once

#include "creeper-qt/utility/theme/color-scheme.hh"
#include "creeper-qt/utility/theme/theme.hh"

using ThemePack = creeper::theme::ThemePack;

constexpr auto myColorSchemeDark = creeper::ColorScheme {
    .primary              = QColor(108, 232, 221),
    .on_primary           = QColor(0, 53, 50),
    .primary_container    = QColor(0, 78, 74),
    .on_primary_container = QColor(165, 255, 247),

    .secondary              = QColor(176, 203, 201),
    .on_secondary           = QColor(26, 54, 52),
    .secondary_container    = QColor(48, 76, 74),
    .on_secondary_container = QColor(204, 232, 229),

    .tertiary              = QColor(183, 196, 230),
    .on_tertiary           = QColor(33, 47, 79),
    .tertiary_container    = QColor(55, 69, 103),
    .on_tertiary_container = QColor(219, 230, 255),

    .error              = QColor(255, 180, 171),
    .on_error           = QColor(105, 0, 5),
    .error_container    = QColor(147, 0, 10),
    .on_error_container = QColor(255, 218, 214),

    .background         = QColor(57, 197, 187),
    .on_background      = QColor(6, 38, 37),
    .surface            = QColor(19, 42, 43),
    .on_surface         = QColor(228, 242, 241),
    .surface_variant    = QColor(63, 73, 73),
    .on_surface_variant = QColor(191, 202, 201),

    .outline         = QColor(138, 149, 148),
    .outline_variant = QColor(63, 73, 73),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(228, 242, 241),
    .inverse_on_surface = QColor(40, 50, 50),
    .inverse_primary    = QColor(0, 106, 101),

    .surface_container_highest = QColor(70, 93, 92),
    .surface_container_high    = QColor(44, 67, 67),
    .surface_container         = QColor(36, 58, 58),
    .surface_container_low     = QColor(29, 50, 50),
    .surface_container_lowest  = QColor(13, 33, 34),
};

constexpr auto myColorSchemeLight = creeper::ColorScheme {
    .primary              = QColor(0, 106, 101),
    .on_primary           = QColor(255, 255, 255),
    .primary_container    = QColor(137, 255, 246),
    .on_primary_container = QColor(0, 32, 30),

    .secondary              = QColor(74, 99, 97),
    .on_secondary           = QColor(255, 255, 255),
    .secondary_container    = QColor(205, 232, 229),
    .on_secondary_container = QColor(7, 32, 31),

    .tertiary              = QColor(88, 100, 133),
    .on_tertiary           = QColor(255, 255, 255),
    .tertiary_container    = QColor(219, 230, 255),
    .on_tertiary_container = QColor(20, 31, 61),

    .error              = QColor(186, 26, 26),
    .on_error           = QColor(255, 255, 255),
    .error_container    = QColor(255, 218, 214),
    .on_error_container = QColor(65, 0, 2),

    .background         = QColor(57, 197, 187),
    .on_background      = QColor(11, 35, 35),
    .surface            = QColor(247, 255, 253),
    .on_surface         = QColor(22, 29, 29),
    .surface_variant    = QColor(217, 228, 226),
    .on_surface_variant = QColor(63, 73, 73),

    .outline         = QColor(111, 122, 121),
    .outline_variant = QColor(189, 200, 198),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(43, 50, 50),
    .inverse_on_surface = QColor(236, 242, 241),
    .inverse_primary    = QColor(108, 232, 221),

    .surface_container_highest = QColor(209, 237, 234),
    .surface_container_high    = QColor(223, 242, 239),
    .surface_container         = QColor(231, 247, 245),
    .surface_container_low     = QColor(239, 251, 249),
    .surface_container_lowest  = QColor(255, 255, 255),
};

constexpr auto myThemePack = ThemePack {
    .light = myColorSchemeLight,
    .dark = myColorSchemeDark,
};
