#pragma once

#include "creeper-qt/utility/theme/color-scheme.hh"
#include "creeper-qt/utility/theme/theme.hh"

#include <array>

using ThemePack = creeper::theme::ThemePack;

namespace mymusic::theme_preset {

constexpr auto kWhiteMintLight = creeper::ColorScheme {
    .primary              = QColor(47, 111, 102),
    .on_primary           = QColor(255, 255, 255),
    .primary_container    = QColor(215, 239, 234),
    .on_primary_container = QColor(16, 46, 42),

    .secondary              = QColor(92, 111, 104),
    .on_secondary           = QColor(255, 255, 255),
    .secondary_container    = QColor(221, 234, 229),
    .on_secondary_container = QColor(26, 43, 39),

    .tertiary              = QColor(91, 110, 145),
    .on_tertiary           = QColor(255, 255, 255),
    .tertiary_container    = QColor(220, 229, 255),
    .on_tertiary_container = QColor(24, 39, 70),

    .error              = QColor(186, 26, 26),
    .on_error           = QColor(255, 255, 255),
    .error_container    = QColor(255, 218, 214),
    .on_error_container = QColor(65, 0, 2),

    .background         = QColor(250, 251, 252),
    .on_background      = QColor(26, 28, 29),
    .surface            = QColor(255, 255, 255),
    .on_surface         = QColor(26, 28, 29),
    .surface_variant    = QColor(237, 242, 244),
    .on_surface_variant = QColor(94, 102, 106),

    .outline         = QColor(196, 205, 210),
    .outline_variant = QColor(215, 222, 226),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(42, 46, 47),
    .inverse_on_surface = QColor(241, 244, 245),
    .inverse_primary    = QColor(165, 213, 204),

    .surface_container_highest = QColor(230, 236, 238),
    .surface_container_high    = QColor(236, 242, 244),
    .surface_container         = QColor(242, 246, 248),
    .surface_container_low     = QColor(244, 246, 248),
    .surface_container_lowest  = QColor(255, 255, 255),
};

constexpr auto kWhiteMintDark = creeper::ColorScheme {
    .primary              = QColor(165, 213, 204),
    .on_primary           = QColor(13, 56, 50),
    .primary_container    = QColor(36, 80, 73),
    .on_primary_container = QColor(215, 239, 234),

    .secondary              = QColor(191, 206, 200),
    .on_secondary           = QColor(42, 53, 49),
    .secondary_container    = QColor(64, 76, 72),
    .on_secondary_container = QColor(221, 234, 229),

    .tertiary              = QColor(191, 205, 240),
    .on_tertiary           = QColor(43, 55, 84),
    .tertiary_container    = QColor(67, 79, 110),
    .on_tertiary_container = QColor(220, 229, 255),

    .error              = QColor(255, 180, 171),
    .on_error           = QColor(105, 0, 5),
    .error_container    = QColor(147, 0, 10),
    .on_error_container = QColor(255, 218, 214),

    .background         = QColor(19, 23, 24),
    .on_background      = QColor(230, 233, 234),
    .surface            = QColor(24, 28, 29),
    .on_surface         = QColor(230, 233, 234),
    .surface_variant    = QColor(64, 72, 75),
    .on_surface_variant = QColor(189, 197, 201),

    .outline         = QColor(136, 145, 149),
    .outline_variant = QColor(64, 72, 75),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(230, 233, 234),
    .inverse_on_surface = QColor(46, 49, 50),
    .inverse_primary    = QColor(47, 111, 102),

    .surface_container_highest = QColor(53, 57, 58),
    .surface_container_high    = QColor(45, 49, 50),
    .surface_container         = QColor(40, 44, 45),
    .surface_container_low     = QColor(33, 37, 38),
    .surface_container_lowest  = QColor(18, 21, 22),
};

constexpr auto kWhiteMintThemePack = ThemePack {
    .light = kWhiteMintLight,
    .dark = kWhiteMintDark,
};

constexpr auto kWhiteMistLight = creeper::ColorScheme {
    .primary              = QColor(59, 130, 160),
    .on_primary           = QColor(255, 255, 255),
    .primary_container    = QColor(217, 238, 246),
    .on_primary_container = QColor(18, 55, 70),

    .secondary              = QColor(106, 114, 128),
    .on_secondary           = QColor(255, 255, 255),
    .secondary_container    = QColor(231, 235, 240),
    .on_secondary_container = QColor(40, 46, 58),

    .tertiary              = QColor(122, 103, 168),
    .on_tertiary           = QColor(255, 255, 255),
    .tertiary_container    = QColor(236, 228, 255),
    .on_tertiary_container = QColor(51, 38, 84),

    .error              = QColor(186, 26, 26),
    .on_error           = QColor(255, 255, 255),
    .error_container    = QColor(255, 218, 214),
    .on_error_container = QColor(65, 0, 2),

    .background         = QColor(248, 249, 251),
    .on_background      = QColor(26, 28, 31),
    .surface            = QColor(255, 255, 255),
    .on_surface         = QColor(26, 28, 31),
    .surface_variant    = QColor(236, 239, 242),
    .on_surface_variant = QColor(98, 104, 112),

    .outline         = QColor(201, 208, 214),
    .outline_variant = QColor(224, 229, 233),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(42, 45, 49),
    .inverse_on_surface = QColor(240, 243, 247),
    .inverse_primary    = QColor(158, 210, 231),

    .surface_container_highest = QColor(231, 235, 238),
    .surface_container_high    = QColor(236, 240, 243),
    .surface_container         = QColor(242, 245, 247),
    .surface_container_low     = QColor(243, 245, 247),
    .surface_container_lowest  = QColor(255, 255, 255),
};

constexpr auto kWhiteMistDark = creeper::ColorScheme {
    .primary              = QColor(158, 210, 231),
    .on_primary           = QColor(19, 63, 79),
    .primary_container    = QColor(41, 87, 106),
    .on_primary_container = QColor(217, 238, 246),

    .secondary              = QColor(197, 201, 212),
    .on_secondary           = QColor(48, 54, 63),
    .secondary_container    = QColor(70, 76, 86),
    .on_secondary_container = QColor(231, 235, 240),

    .tertiary              = QColor(210, 193, 245),
    .on_tertiary           = QColor(58, 45, 91),
    .tertiary_container    = QColor(81, 65, 117),
    .on_tertiary_container = QColor(236, 228, 255),

    .error              = QColor(255, 180, 171),
    .on_error           = QColor(105, 0, 5),
    .error_container    = QColor(147, 0, 10),
    .on_error_container = QColor(255, 218, 214),

    .background         = QColor(18, 21, 24),
    .on_background      = QColor(229, 232, 235),
    .surface            = QColor(22, 25, 28),
    .on_surface         = QColor(229, 232, 235),
    .surface_variant    = QColor(66, 72, 78),
    .on_surface_variant = QColor(191, 196, 201),

    .outline         = QColor(138, 144, 150),
    .outline_variant = QColor(66, 72, 78),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(229, 232, 235),
    .inverse_on_surface = QColor(43, 46, 49),
    .inverse_primary    = QColor(59, 130, 160),

    .surface_container_highest = QColor(51, 54, 58),
    .surface_container_high    = QColor(44, 47, 51),
    .surface_container         = QColor(38, 41, 45),
    .surface_container_low     = QColor(31, 35, 38),
    .surface_container_lowest  = QColor(17, 19, 22),
};

constexpr auto kWhiteMistThemePack = ThemePack {
    .light = kWhiteMistLight,
    .dark = kWhiteMistDark,
};

constexpr auto kWarmPaperLight = creeper::ColorScheme {
    .primary              = QColor(138, 90, 68),
    .on_primary           = QColor(255, 255, 255),
    .primary_container    = QColor(243, 224, 214),
    .on_primary_container = QColor(61, 34, 23),

    .secondary              = QColor(116, 102, 92),
    .on_secondary           = QColor(255, 255, 255),
    .secondary_container    = QColor(238, 227, 218),
    .on_secondary_container = QColor(47, 38, 31),

    .tertiary              = QColor(108, 122, 82),
    .on_tertiary           = QColor(255, 255, 255),
    .tertiary_container    = QColor(228, 237, 211),
    .on_tertiary_container = QColor(40, 51, 24),

    .error              = QColor(186, 26, 26),
    .on_error           = QColor(255, 255, 255),
    .error_container    = QColor(255, 218, 214),
    .on_error_container = QColor(65, 0, 2),

    .background         = QColor(252, 250, 247),
    .on_background      = QColor(30, 27, 25),
    .surface            = QColor(255, 255, 255),
    .on_surface         = QColor(30, 27, 25),
    .surface_variant    = QColor(240, 234, 227),
    .on_surface_variant = QColor(108, 99, 92),

    .outline         = QColor(201, 191, 183),
    .outline_variant = QColor(226, 219, 213),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(48, 44, 41),
    .inverse_on_surface = QColor(244, 239, 236),
    .inverse_primary    = QColor(224, 189, 171),

    .surface_container_highest = QColor(236, 230, 224),
    .surface_container_high    = QColor(242, 236, 231),
    .surface_container         = QColor(247, 243, 238),
    .surface_container_low     = QColor(249, 246, 243),
    .surface_container_lowest  = QColor(255, 255, 255),
};

constexpr auto kWarmPaperDark = creeper::ColorScheme {
    .primary              = QColor(224, 189, 171),
    .on_primary           = QColor(76, 43, 30),
    .primary_container    = QColor(107, 66, 48),
    .on_primary_container = QColor(243, 224, 214),

    .secondary              = QColor(211, 198, 188),
    .on_secondary           = QColor(56, 46, 40),
    .secondary_container    = QColor(79, 69, 61),
    .on_secondary_container = QColor(238, 227, 218),

    .tertiary              = QColor(201, 213, 181),
    .on_tertiary           = QColor(50, 61, 34),
    .tertiary_container    = QColor(73, 85, 54),
    .on_tertiary_container = QColor(228, 237, 211),

    .error              = QColor(255, 180, 171),
    .on_error           = QColor(105, 0, 5),
    .error_container    = QColor(147, 0, 10),
    .on_error_container = QColor(255, 218, 214),

    .background         = QColor(23, 20, 18),
    .on_background      = QColor(236, 231, 227),
    .surface            = QColor(28, 25, 23),
    .on_surface         = QColor(236, 231, 227),
    .surface_variant    = QColor(77, 69, 63),
    .on_surface_variant = QColor(208, 199, 192),

    .outline         = QColor(152, 143, 136),
    .outline_variant = QColor(77, 69, 63),
    .shadow          = QColor(0, 0, 0),
    .scrim           = QColor(0, 0, 0),

    .inverse_surface    = QColor(236, 231, 227),
    .inverse_on_surface = QColor(49, 45, 42),
    .inverse_primary    = QColor(138, 90, 68),

    .surface_container_highest = QColor(58, 54, 50),
    .surface_container_high    = QColor(50, 46, 43),
    .surface_container         = QColor(43, 39, 36),
    .surface_container_low     = QColor(35, 31, 29),
    .surface_container_lowest  = QColor(20, 17, 16),
};

constexpr auto kWarmPaperThemePack = ThemePack {
    .light = kWarmPaperLight,
    .dark = kWarmPaperDark,
};

constexpr auto kThemePacks = std::array {
    kWhiteMintThemePack,
    kWhiteMistThemePack,
    kWarmPaperThemePack,
};

}

constexpr auto myThemePack = mymusic::theme_preset::kWhiteMintThemePack;
