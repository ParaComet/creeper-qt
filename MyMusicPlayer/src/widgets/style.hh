#pragma once

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/text.hh"
#include "creeper-qt/widget/widget.hh"

#include <QColor>
#include <QString>

namespace mymusic::style {

inline auto rgba_css(const QColor& color) -> QString { return color.name(QColor::HexArgb); }

inline auto panel_style(const creeper::ColorScheme& scheme) -> QString {
    return QString(
        "#panel {"
        " background: %1;"
        " color: %2;"
        " border: 1px solid %3;"
        " border-radius: 16px;"
        " }"
        "#panel QLabel {"
        " background: transparent;"
        " border: none;"
        " }")
        .arg(rgba_css(scheme.surface_container_low))
        .arg(rgba_css(scheme.on_surface))
        .arg(rgba_css(scheme.outline_variant));
}


inline auto elevated_panel_style(const creeper::ColorScheme& scheme) -> QString {
    return QString(
        "#elevated_panel {"
        " background: %1;"
        " color: %2;"
        " border-top: 1px solid %3;"
        " }"
        "#elevated_panel QLabel {"
        " background: transparent;"
        " border: none;"
        " }")
        .arg(rgba_css(scheme.surface_container_high))
        .arg(rgba_css(scheme.on_surface))
        .arg(rgba_css(scheme.outline_variant));
}

inline auto cover_style(const creeper::ColorScheme& scheme, const QColor& fill) -> QString {
    return QString(
        "#cover_label {"
        " background: %1;"
        " color: %2;"
        " border: 1px solid %3;"
        " border-radius: 14px;"
        " }"
        "#cover_label QLabel {"
        " background: transparent;"
        " border: none;"
        " }")
        .arg(rgba_css(fill))
        .arg(rgba_css(scheme.on_primary_container))
        .arg(rgba_css(scheme.outline_variant));
}

inline auto row_style(const creeper::ColorScheme& scheme, bool selected = false) -> QString {
    const auto background = selected ? scheme.primary_container : scheme.surface_container_lowest;
    const auto foreground = selected ? scheme.on_primary_container : scheme.on_surface;
    const auto border     = selected ? scheme.primary : scheme.outline_variant;

    return QString(
        "#row_panel {"
        " background: %1;"
        " color: %2;"
        " border: 1px solid %3;"
        " border-radius: 18px;"
        " }"
        "#row_panel QLabel {"
        " background: transparent;"
        " border: none;"
        " }")
        .arg(rgba_css(background))
        .arg(rgba_css(foreground))
        .arg(rgba_css(border));
}

inline auto song_row_style(const creeper::ColorScheme& scheme) -> QString {
    return QString(
        "#song_row {"
        " background: %1;"
        " color: %2;"
        " border: 1px solid %3;"
        " border-radius: 16px;"
        " }"
        "#song_row QLabel {"
        " background: transparent;"
        " }")
        .arg(rgba_css(scheme.surface_container_lowest))
        .arg(rgba_css(scheme.on_surface))
        .arg(rgba_css(scheme.outline_variant));
}


inline auto meta_text_color(const creeper::ColorScheme& scheme) -> creeper::text::pro::Color {
    return creeper::text::pro::Color { scheme.on_surface_variant };
}

inline auto title_text_color(const creeper::ColorScheme& scheme,
    bool selected = false) -> creeper::text::pro::Color {
    return creeper::text::pro::Color { selected ? scheme.on_primary_container : scheme.on_surface };
}

inline auto make_cover_label(const creeper::ColorScheme& scheme, const QString& label,
    const QSize& size, const QColor& fill) -> creeper::Widget* {
    namespace lnpro = creeper::linear::pro;

    return new creeper::Widget {
        creeper::widget::pro::FixedSize { size },
        creeper::widget::pro::Apply { [scheme, fill](QWidget& self) {
            self.setObjectName("cover_label");
            self.setStyleSheet(cover_style(scheme, fill));
        } },
        creeper::widget::pro::Layout<creeper::Col> {
            lnpro::Margin { 0 },
            lnpro::Item<creeper::Text> {
                { 0, Qt::AlignCenter },
                creeper::text::pro::Text { label },
                creeper::text::pro::Color { scheme.on_primary_container },
            },
        },
    };
}

}
