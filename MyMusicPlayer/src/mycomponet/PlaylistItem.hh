#pragma once

#include "creeper-qt/utility/qt_wrapper/enter-event.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/utility/wrapper/common.hh"
#include "creeper-qt/utility/wrapper/property.hh"
#include "creeper-qt/utility/wrapper/widget.hh"

#include <QAbstractButton>
#include <QPixmap>
#include <QString>

namespace mycomponent::playlist_item::internal {

class PlaylistItem : public QAbstractButton {
public:
    explicit PlaylistItem(QWidget* parent = nullptr);

    void set_title(const QString& title) noexcept;
    void set_meta(const QString& meta) noexcept;
    void set_badge(const QString& badge) noexcept;
    void set_cover_path(const QString& cover_path) noexcept;
    void set_marked(bool marked) noexcept;

    void set_color_scheme(const creeper::ColorScheme& scheme) noexcept;
    void load_theme_manager(creeper::ThemeManager& manager) noexcept;

    void set_selected(bool selected) noexcept;
    bool selected() const noexcept;

    auto sizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(creeper::qt::EnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_title;
    QString m_meta;
    QString m_badge;
    QPixmap m_cover;
    creeper::ColorScheme m_scheme {};
    bool m_hovered = false;
    bool m_marked = false;
};

}

namespace mycomponent::playlist_item::pro {

using Token = creeper::common::Token<internal::PlaylistItem>;

using Title =
    creeper::DerivedProp<Token, QString, [](auto& self, const QString& value) { self.set_title(value); }>;
using Meta =
    creeper::DerivedProp<Token, QString, [](auto& self, const QString& value) { self.set_meta(value); }>;
using Badge =
    creeper::DerivedProp<Token, QString, [](auto& self, const QString& value) { self.set_badge(value); }>;
using CoverPath = creeper::DerivedProp<Token, QString,
    [](auto& self, const QString& value) { self.set_cover_path(value); }>;
using Marked =
    creeper::SetterProp<Token, bool, [](auto& self, bool value) { self.set_marked(value); }>;
using Selected =
    creeper::SetterProp<Token, bool, [](auto& self, bool value) { self.set_selected(value); }>;

template <typename Callback>
using Clickable = creeper::common::pro::Clickable<Callback, Token>;

template <class T>
concept trait = std::derived_from<T, Token>;

CREEPER_DEFINE_CHECKER(trait);

using namespace creeper::widget::pro;
using namespace creeper::theme::pro;

}

namespace mycomponent {

using PlaylistItem = creeper::Declarative<playlist_item::internal::PlaylistItem,
    creeper::CheckerOr<playlist_item::pro::checker, creeper::widget::pro::checker,
        creeper::theme::pro::checker>>;

}
