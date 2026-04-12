#include "playlist_page.hh"

#include "style.hh"

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/text.hh"

#include <QVBoxLayout>
#include <qwidget.h>

using namespace creeper;
using mycomponent::PlaylistItem;
namespace lnpro = linear::pro;
namespace pp    = mycomponent::playlist_item::pro;
namespace sty   = mymusic::style;
namespace fc    = filled_card::pro;
namespace model = mymusic::model;
PlaylistPage::PlaylistPage(ThemeManager& manager, LibraryService& library)
    : m_theme_manager(manager)
    , m_library(library) {
    m_root_layout = new QVBoxLayout {};
    m_root_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_root_layout);
    reload_content();
    setup_connections();
}

auto PlaylistPage::build_sidebar_playlist_item(int index, const QString& title, const QString& meta,
    const QString& badge, bool selected) -> mycomponent::PlaylistItem* {
    auto* item = new PlaylistItem {
        pp::ThemeManager { m_theme_manager },
        pp::Title { title },
        pp::Meta { meta },
        pp::Badge { badge },
        pp::Selected { selected },
        pp::Clickable { [this, index] { switch_playlist(index); } },
    };

    m_playlist_items.push_back(item);
    return item;
}

auto PlaylistPage::build_song_row(const QString& index, const QString& title, const QString& artist,
    const QString& album, const QString& duration, const QString& badge) -> creeper::Widget* {
    const auto scheme = m_theme_manager.color_scheme();

    return new creeper::Widget {
        widget::pro::Apply { [scheme](QWidget& self) {
            self.setObjectName("song_row");
            self.setStyleSheet(sty::song_row_style(scheme));
        } },
        widget::pro::Layout<Row> {
            lnpro::ContentsMargin { {14, 12, 14, 12} },
            lnpro::Spacing { 12 },
            lnpro::Item<Text> {
                text::pro::Text { index },
                text::pro::Color { scheme.on_surface_variant },
            },
            lnpro::Item { sty::make_cover_label(scheme, badge, QSize { 44, 44 }, scheme.tertiary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 2 },
                lnpro::Item<Text> {
                    text::pro::Text { title },
                    text::pro::ThemeManager { m_theme_manager },
                },
                lnpro::Item<Text> {
                    text::pro::Text { artist },
                    sty::meta_text_color(scheme),
                },
            },
            lnpro::Item<Text> {
                text::pro::Text { album },
                text::pro::Color { scheme.on_surface_variant },
            },
            lnpro::Item<Text> {
                text::pro::Text { duration },
                text::pro::Color { scheme.on_surface_variant },
            },
        },
    };
}

auto PlaylistPage::build_sidebar() -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();
    const auto& playlists = m_library.playlists();

    auto* panel = new FilledCard {
        // fc::Apply { [scheme](QWidget& self) {
        //     self.setObjectName("panel");
        //     self.setStyleSheet(sty::panel_style(scheme));
        // } },
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::FixedWidth { 290 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { {18, 20, 18, 20} },
            lnpro::Spacing { 14 },
            lnpro::Item<Text> {
                text::pro::Text { "你的歌单" },
                text::pro::ThemeManager { m_theme_manager },
                widget::pro::Apply { [](Widget& self) {
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
            lnpro::Item<Text> {
                text::pro::Text { "左侧像导航，右侧像内容详情，这样会更像播放器。" },
                sty::meta_text_color(scheme),
                
            },
        },
    };

    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    for (int index = 0; index < static_cast<int>(playlists.size()); ++index) {
        const auto& playlist = playlists[index];
        layout->addWidget(build_sidebar_playlist_item(index, playlist.title, playlist.sidebar_meta,
            playlist.sidebar_badge, index == 0));
    }
    layout->addStretch(1);
    return panel;
}

auto PlaylistPage::build_header(const QString& title, const QString& description,
    const QString& meta, const QString& badge, const QColor& badge_color) -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();

    return new FilledCard {
        // fc::Apply { [scheme](QWidget& self) {
        //     self.setObjectName("panel");
        //     self.setStyleSheet(sty::panel_style(scheme));
        // } },
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { {22, 22, 22, 22} },
            lnpro::Spacing { 18 },
            lnpro::Item { sty::make_cover_label(scheme, badge, QSize { 124, 124 }, badge_color) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 8 },
                lnpro::Item<Text> {
                    text::pro::Text { title },
                    text::pro::ThemeManager { m_theme_manager },
                
                },
                lnpro::Item<Text> {
                    text::pro::Text { description },
                    sty::meta_text_color(scheme),
                },
                lnpro::Item<Text> {
                    text::pro::Text { meta },
                    text::pro::Color { scheme.primary },
                },
            },
        },
    };
}

auto PlaylistPage::build_song_list(const model::PlaylistInfo& playlist) -> QWidget* {
    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { {18, 18, 18, 18} },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::Text { QStringLiteral("%1 · 歌曲列表").arg(playlist.title) },
                text::pro::ThemeManager { m_theme_manager },
            },
        },
    };

    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    for (const auto& song : playlist.songs) {
        layout->addWidget(build_song_row(QStringLiteral("%1").arg(song.track_number, 2, 10, QChar('0')),
            song.title, song.artist, song.album, song.duration_text, song.cover_badge));
    }
    layout->addStretch(1);
    return panel;
}

auto PlaylistPage::build_content() -> creeper::Widget* {
    const auto& playlists = m_library.playlists();
    m_playlist_items.clear();
    m_header_pages   = new NavHost { nav_host::pro::CurrentIndex { 0 } };
    m_songlist_pages = new NavHost { nav_host::pro::CurrentIndex { 0 } };

    for (const auto& playlist : playlists) {
        m_header_pages->addWidget(build_header(playlist.title, playlist.description, playlist.meta,
            playlist.badge, playlist.accent_color));
        m_songlist_pages->addWidget(build_song_list(playlist));
    }

    auto* header_host = new creeper::Widget {};
    header_host->setLayout(m_header_pages);

    auto* song_host = new creeper::Widget {};
    song_host->setLayout(m_songlist_pages);

    return new creeper::Widget {
        widget::pro::Layout<Row> {
            lnpro::ContentsMargin { { 0, 0, 0, 0} },
            lnpro::Spacing { 8 }, // 侧边栏和内容之间的间距
            lnpro::Item { build_sidebar() },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 8 }, // header和歌曲列表之间的间距
                lnpro::Item { header_host },
                lnpro::Item<creeper::Widget> {
                    lnpro::Item<creeper::Widget>::LayoutMethod { 1 },
                    song_host,
                },
            },
        },
    };
}

void PlaylistPage::reload_content() {
    if (m_root_layout == nullptr) {
        return;
    }
    if (m_content_widget != nullptr) {
        m_root_layout->removeWidget(m_content_widget);
        m_content_widget->deleteLater();
    }

    m_content_widget = build_content();
    m_root_layout->addWidget(m_content_widget);
    m_theme_manager.apply_theme();

    if (!m_library.playlists().empty()) {
        switch_playlist(0);
    }
}

void PlaylistPage::switch_playlist(int index) {
    if (m_header_pages == nullptr || m_songlist_pages == nullptr) {
        return;
    }

    m_header_pages->setCurrentIndex(index);
    m_songlist_pages->setCurrentIndex(index);

    for (int i = 0; i < static_cast<int>(m_playlist_items.size()); ++i) {
        m_playlist_items[i]->set_selected(i == index);
    }
}

void PlaylistPage::setup_connections() {
    QObject::connect(&m_library, &LibraryService::library_changed, this,
        [this] { reload_content(); });
}
