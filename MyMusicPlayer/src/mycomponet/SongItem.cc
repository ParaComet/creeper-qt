#include "SongItem.hh"

#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

using namespace mycomponent::song_item::internal;

namespace {

auto with_alpha(QColor color, qreal alpha) -> QColor {
    color.setAlphaF(alpha);
    return color;
}

}

SongItem::SongItem(QWidget* parent)
    : QAbstractButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(72);
}

void SongItem::set_index_text(const QString& text) noexcept {
    m_index_text = text;
    update();
}

void SongItem::set_title(const QString& title) noexcept {
    m_title = title;
    update();
}

void SongItem::set_artist(const QString& artist) noexcept {
    m_artist = artist;
    update();
}

void SongItem::set_album(const QString& album) noexcept {
    m_album = album;
    update();
}

void SongItem::set_duration(const QString& duration) noexcept {
    m_duration = duration;
    update();
}

void SongItem::set_badge(const QString& badge) noexcept {
    m_badge = badge;
    update();
}

void SongItem::set_cover_path(const QString& cover_path) noexcept {
    if (!cover_path.isEmpty() && QFileInfo::exists(cover_path)) {
        m_cover.load(cover_path);
    } else {
        m_cover = QPixmap {};
    }
    update();
}

void SongItem::set_color_scheme(const creeper::ColorScheme& scheme) noexcept {
    m_scheme = scheme;
    update();
}

void SongItem::load_theme_manager(creeper::ThemeManager& manager) noexcept {
    manager.append_handler(this, [this](const creeper::ThemeManager& manager) {
        set_color_scheme(manager.color_scheme());
    });
}

auto SongItem::sizeHint() const -> QSize { return { 720, 72 }; }

void SongItem::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    const auto background = isDown() ? with_alpha(m_scheme.primary, 0.14)
                                     : (m_hovered ? with_alpha(m_scheme.primary, 0.08)
                                                  : m_scheme.surface_container_lowest);
    auto painter = QPainter { this };
    painter.setRenderHint(QPainter::Antialiasing, true);

    const auto outer_rect = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
    const auto badge_rect = QRectF { 52.0, 14.0, 44.0, 44.0 };
    const auto title_left = int(badge_rect.right()) + 16;

    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(outer_rect, 16.0, 16.0);

    painter.setPen(m_scheme.on_surface_variant);
    painter.drawText(QRect { 14, 0, 28, height() }, Qt::AlignCenter, m_index_text);

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_scheme.tertiary_container);
    painter.drawRoundedRect(badge_rect, 12.0, 12.0);

    if (!m_cover.isNull()) {
        auto path = QPainterPath {};
        path.addRoundedRect(badge_rect, 12.0, 12.0);
        painter.save();
        painter.setClipPath(path);
        painter.drawPixmap(badge_rect.toRect(),
            m_cover.scaled(badge_rect.size().toSize(), Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation));
        painter.restore();
    } else {
        auto badge_font = font();
        badge_font.setBold(true);
        painter.setFont(badge_font);
        painter.setPen(m_scheme.on_tertiary_container);
        painter.drawText(badge_rect, Qt::AlignCenter, m_badge);
    }

    auto title_font = font();
    title_font.setBold(true);
    painter.setFont(title_font);
    painter.setPen(m_scheme.on_surface);
    painter.drawText(QRect { title_left, 14, width() - title_left - 180, 22 },
        Qt::AlignLeft | Qt::AlignVCenter, m_title);

    auto meta_font = font();
    meta_font.setBold(false);
    meta_font.setPointSizeF(meta_font.pointSizeF() > 0 ? meta_font.pointSizeF() - 1.0 : 9.0);
    painter.setFont(meta_font);
    painter.setPen(m_scheme.on_surface_variant);
    painter.drawText(QRect { title_left, 40, width() - title_left - 180, 18 },
        Qt::AlignLeft | Qt::AlignVCenter, m_artist);

    painter.drawText(QRect { width() - 180, 0, 110, height() }, Qt::AlignCenter, m_album);
    painter.drawText(QRect { width() - 70, 0, 56, height() }, Qt::AlignCenter, m_duration);
}

void SongItem::enterEvent(creeper::qt::EnterEvent* event) {
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void SongItem::leaveEvent(QEvent* event) {
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}
