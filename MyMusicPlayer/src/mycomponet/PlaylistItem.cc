#include "PlaylistItem.hh"

#include <QCursor>
#include <QPainter>
#include <QPainterPath>

using namespace mycomponent::playlist_item::internal;

namespace {

auto with_alpha(QColor color, qreal alpha) -> QColor {
    color.setAlphaF(alpha);
    return color;
}

}

PlaylistItem::PlaylistItem(QWidget* parent)
    : QAbstractButton(parent) {
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(76);

    QObject::connect(this, &QAbstractButton::toggled, this, [this] { update(); });
}

void PlaylistItem::set_title(const QString& title) noexcept {
    m_title = title;
    update();
}

void PlaylistItem::set_meta(const QString& meta) noexcept {
    m_meta = meta;
    update();
}

void PlaylistItem::set_badge(const QString& badge) noexcept {
    m_badge = badge;
    update();
}

void PlaylistItem::set_color_scheme(const creeper::ColorScheme& scheme) noexcept {
    m_scheme = scheme;
    update();
}

void PlaylistItem::load_theme_manager(creeper::ThemeManager& manager) noexcept {
    manager.append_handler(this, [this](const creeper::ThemeManager& manager) {
        set_color_scheme(manager.color_scheme());
    });
}

void PlaylistItem::set_selected(bool selected) noexcept { setChecked(selected); }

bool PlaylistItem::selected() const noexcept { return isChecked(); }

auto PlaylistItem::sizeHint() const -> QSize { return { 240, 76 }; }

void PlaylistItem::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    const auto selected         = isChecked();
    const auto background       = selected ? m_scheme.primary_container : m_scheme.surface_container_lowest;
    const auto border           = selected ? m_scheme.primary : m_scheme.outline_variant;
    const auto title_color      = selected ? m_scheme.on_primary_container : m_scheme.on_surface;
    const auto meta_color       = selected ? with_alpha(m_scheme.on_primary_container, 0.75)
                                           : m_scheme.on_surface_variant;
    const auto badge_fill       = selected ? m_scheme.primary : m_scheme.secondary_container;
    const auto badge_text_color = selected ? m_scheme.on_primary : m_scheme.on_secondary_container;
    const auto hover_fill       = selected ? with_alpha(m_scheme.primary, 0.10)
                                           : with_alpha(m_scheme.on_surface, 0.05);

    auto painter = QPainter { this };
    painter.setRenderHint(QPainter::Antialiasing, true);

    constexpr auto kOuterRadius = 18.0;
    constexpr auto kBadgeRadius = 12.0;

    const auto outer_rect = rect().adjusted(1, 1, -1, -1);
    const auto badge_rect = QRectF { 12.0, 12.0, 52.0, 52.0 };
    const auto text_left  = int(badge_rect.right()) + 14;
    const auto title_rect = QRect { text_left, 16, width() - text_left - 12, 22 };
    const auto meta_rect  = QRect { text_left, 42, width() - text_left - 12, 18 };

    painter.setPen(QPen { border, 1.2 });
    painter.setBrush(background);
    painter.drawRoundedRect(outer_rect, kOuterRadius, kOuterRadius);

    if (m_hovered) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(hover_fill);
        painter.drawRoundedRect(outer_rect, kOuterRadius, kOuterRadius);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(badge_fill);
    painter.drawRoundedRect(badge_rect, kBadgeRadius, kBadgeRadius);

    auto badge_font = font();
    badge_font.setBold(true);
    painter.setFont(badge_font);
    painter.setPen(badge_text_color);
    painter.drawText(badge_rect, Qt::AlignCenter, m_badge);

    auto title_font = font();
    title_font.setBold(true);
    title_font.setPointSizeF(title_font.pointSizeF() > 0 ? title_font.pointSizeF() + 1.0 : 11.0);
    painter.setFont(title_font);
    painter.setPen(title_color);
    painter.drawText(title_rect, Qt::AlignLeft | Qt::AlignVCenter, m_title);

    auto meta_font = font();
    meta_font.setBold(false);
    meta_font.setPointSizeF(meta_font.pointSizeF() > 0 ? meta_font.pointSizeF() - 1.0 : 9.0);
    painter.setFont(meta_font);
    painter.setPen(meta_color);
    painter.drawText(meta_rect, Qt::AlignLeft | Qt::AlignVCenter, m_meta);
}

void PlaylistItem::enterEvent(creeper::qt::EnterEvent* event) {
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void PlaylistItem::leaveEvent(QEvent* event) {
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}
