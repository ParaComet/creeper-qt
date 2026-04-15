#pragma once

#include "creeper-qt/utility/qt_wrapper/enter-event.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/utility/wrapper/common.hh"
#include "creeper-qt/utility/wrapper/property.hh"
#include "creeper-qt/utility/wrapper/widget.hh"

#include <QAbstractButton>
#include <QPixmap>
#include <QString>

namespace mycomponent::song_item::internal {

class SongItem : public QAbstractButton {
public:
    explicit SongItem(QWidget* parent = nullptr);

    void set_index_text(const QString& text) noexcept;
    void set_title(const QString& title) noexcept;
    void set_artist(const QString& artist) noexcept;
    void set_album(const QString& album) noexcept;
    void set_duration(const QString& duration) noexcept;
    void set_badge(const QString& badge) noexcept;
    void set_cover_path(const QString& cover_path) noexcept;

    void set_color_scheme(const creeper::ColorScheme& scheme) noexcept;
    void load_theme_manager(creeper::ThemeManager& manager) noexcept;

    auto sizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(creeper::qt::EnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_index_text;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_duration;
    QString m_badge;
    QPixmap m_cover;
    creeper::ColorScheme m_scheme {};
    bool m_hovered = false;
};

}

namespace mycomponent::song_item::pro {

using Token = creeper::common::Token<internal::SongItem>;

using IndexText = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_index_text(value); }>;
using Title = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_title(value); }>;
using Artist = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_artist(value); }>;
using Album = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_album(value); }>;
using Duration = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_duration(value); }>;
using Badge = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_badge(value); }>;
using CoverPath = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_cover_path(value); }>;

template <typename Callback>
using Clickable = creeper::common::pro::Clickable<Callback, Token>;

template <class T>
concept trait = std::derived_from<T, Token>;

CREEPER_DEFINE_CHECKER(trait);

using namespace creeper::widget::pro;
using namespace creeper::theme::pro;

}

namespace mycomponent {

using SongItem = creeper::Declarative<song_item::internal::SongItem,
    creeper::CheckerOr<song_item::pro::checker, creeper::widget::pro::checker,
        creeper::theme::pro::checker>>;

}
