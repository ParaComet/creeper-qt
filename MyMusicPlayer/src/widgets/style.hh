#pragma once

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/widget/image.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/text.hh"
#include "creeper-qt/widget/widget.hh"

#include <QColor>
#include <QFileInfo>
#include <QString>
#include <QVBoxLayout>

#include <cmath>

namespace mymusic::style {

inline auto rgba_css(const QColor& color) -> QString { return color.name(QColor::HexArgb); }

inline auto contrast_text_color(const QColor& fill) -> QColor {
    const auto linear_channel = [](double channel) {
        channel /= 255.0;
        if (channel <= 0.03928) {
            return channel / 12.92;
        }
        return std::pow((channel + 0.055) / 1.055, 2.4);
    };

    const auto luminance = 0.2126 * linear_channel(fill.red())
        + 0.7152 * linear_channel(fill.green())
        + 0.0722 * linear_channel(fill.blue());
    return luminance > 0.42 ? QColor(28, 28, 28) : QColor(250, 250, 250);
}

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
    const auto text_color = contrast_text_color(fill);
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
        .arg(rgba_css(text_color))
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
    auto color = scheme.on_surface_variant;
    if (color.alpha() < 255) {
        color.setAlpha(255);
    }
    if (color.lightness() > scheme.on_surface.lightness()) {
        color = color.darker(118);
    } else {
        color = color.lighter(112);
    }
    return creeper::text::pro::Color { color };
}

inline auto title_text_color(const creeper::ColorScheme& scheme,
    bool selected = false) -> creeper::text::pro::Color {
    return creeper::text::pro::Color { selected ? scheme.on_primary_container : scheme.on_surface };
}

inline auto make_cover_label(creeper::ThemeManager& manager, const QString& label,
    const QSize& size, const QColor& fill, const QString& cover_path = {}) -> QWidget* {
    namespace lnpro  = creeper::linear::pro;
    namespace impro = creeper::image::pro;

    if (!cover_path.isEmpty() && QFileInfo::exists(cover_path)) {
        auto* image = new creeper::Image {
            creeper::widget::pro::FixedSize { size },
            impro::Pixmap { cover_path.toStdString() },
            impro::ContentScale { creeper::ContentScale::CROP },
            impro::Radius { 14.0 },
            impro::BorderWidth { 1.0 },
        };
        manager.append_handler(image, [image](const creeper::ThemeManager& manager) {
            image->set_border_color(manager.color_scheme().outline_variant);
        });
        return image;
    }

    auto* label_text = new creeper::Text {
        creeper::text::pro::Text { label },
    };
    auto* cover = new creeper::Widget {
        creeper::widget::pro::FixedSize { size },
        creeper::widget::pro::Layout<creeper::Col> {
            lnpro::Margin { 0 },
            lnpro::Item {
                { 0, Qt::AlignCenter },
                label_text,
            },
        },
    };
    manager.append_handler(cover, [cover, fill](const creeper::ThemeManager& manager) {
        cover->setObjectName("cover_label");
        cover->setStyleSheet(cover_style(manager.color_scheme(), fill));
    });
    manager.append_handler(label_text, [label_text, fill](const creeper::ThemeManager&) {
        label_text->set_color(contrast_text_color(fill));
    });
    return cover;
}

}
