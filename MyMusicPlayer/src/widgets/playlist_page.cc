#include "playlist_page.hh"

#include "style.hh"

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/layout/scroll.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/text.hh"

#include <QVBoxLayout>
#include <qwidget.h>

using namespace creeper;
using mycomponent::PlaylistItem;
using mycomponent::SongItem;
namespace lnpro = linear::pro;
namespace pp    = mycomponent::playlist_item::pro;
namespace scrollpro = creeper::scroll::pro;
namespace sp    = mycomponent::song_item::pro;
namespace sty   = mymusic::style;
namespace fc    = filled_card::pro;
namespace model = mymusic::model;

namespace {

auto playlist_cover_path(const model::PlaylistInfo& playlist) -> QString {
    for (const auto& song : playlist.songs) {
        if (!song.cover_path.isEmpty()) {
            return song.cover_path;
        }
    }
    return {};
}

}
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
    const auto& playlists = m_library.playlists();
    const auto cover_path = (index >= 0 && index < static_cast<int>(playlists.size()))
        ? playlist_cover_path(playlists[index])
        : QString {};

    auto* item = new PlaylistItem {
        pp::ThemeManager { m_theme_manager },
        pp::Title { title },
        pp::Meta { meta },
        pp::Badge { badge },
        pp::CoverPath { cover_path },
        pp::Selected { selected },
        pp::Clickable { [this, index] { switch_playlist(index); } },
    };

    m_playlist_items.push_back(item);
    return item;
}

auto PlaylistPage::build_song_row(const QString& source_id, const model::SongInfo& song)
    -> mycomponent::SongItem* {
    return new SongItem {
        sp::ThemeManager { m_theme_manager },
        sp::IndexText { QStringLiteral("%1").arg(song.track_number, 2, 10, QChar('0')) },
        sp::Title { song.title },
        sp::Artist { song.artist },
        sp::Album { song.album },
        sp::Duration { song.duration_text },
        sp::Badge { song.cover_badge },
        sp::CoverPath { song.cover_path },
        sp::Clickable { [this, source_id, song] { emit song_activated(source_id, song.id); } },
    };
}

auto PlaylistPage::build_sidebar() -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();
    const auto& playlists = m_library.playlists();
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 12 },
        },
    };
    auto* list_layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (int index = 0; index < static_cast<int>(playlists.size()); ++index) {
        const auto& playlist = playlists[index];
        list_layout->addWidget(build_sidebar_playlist_item(index, playlist.title, playlist.sidebar_meta,
            playlist.sidebar_badge, index == 0));
    }
    list_layout->addStretch(1);

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
                text::pro::Text { "我的歌单" },
                text::pro::ThemeManager { m_theme_manager },
                widget::pro::Apply { [](Widget& self) {
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
            lnpro::Item<Text> {
                text::pro::Text { "ciallo (∠·ω )⌒★" },
                sty::meta_text_color(scheme),
                
            },
            lnpro::Item<ScrollArea> {
                lnpro::Item<ScrollArea>::LayoutMethod { 1 },
                scrollpro::ThemeManager { m_theme_manager },
                scrollpro::HorizontalScrollBarPolicy { Qt::ScrollBarAlwaysOff },
                scrollpro::Item { list_content },
            },
        },
    };
    return panel;
}

auto PlaylistPage::build_header(const model::PlaylistInfo& playlist) -> QWidget* {
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
            lnpro::Item {
                sty::make_cover_label(m_theme_manager, playlist.badge, QSize { 124, 124 }, playlist.accent_color,
                    playlist_cover_path(playlist)) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 8 },
                lnpro::Item<Text> {
                    text::pro::Text { playlist.title },
                    text::pro::ThemeManager { m_theme_manager },
                
                },
                lnpro::Item<Text> {
                    text::pro::Text { playlist.description },
                    sty::meta_text_color(scheme),
                },
                lnpro::Item<Text> {
                    text::pro::Text { playlist.meta },
                    text::pro::Color { scheme.primary },
                },
            },
        },
    };
}

auto PlaylistPage::build_song_list(const model::PlaylistInfo& playlist) -> QWidget* {
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 10 },
        },
    };
    auto* list_layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (const auto& song : playlist.songs) {
        list_layout->addWidget(build_song_row(playlist.id, song));
    }
    list_layout->addStretch(1);

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
            lnpro::Item<ScrollArea> {
                lnpro::Item<ScrollArea>::LayoutMethod { 1 },
                scrollpro::ThemeManager { m_theme_manager },
                scrollpro::HorizontalScrollBarPolicy { Qt::ScrollBarAlwaysOff },
                scrollpro::Item { list_content },
            },
        },
    };
    return panel;
}

auto PlaylistPage::build_content() -> creeper::Widget* {
    const auto& playlists = m_library.playlists();
    m_playlist_items.clear();
    m_playlist_ids.clear();
    m_header_pages   = new NavHost { nav_host::pro::CurrentIndex { 0 } };
    m_songlist_pages = new NavHost { nav_host::pro::CurrentIndex { 0 } };

    for (const auto& playlist : playlists) {
        m_playlist_ids.push_back(playlist.id);
        m_header_pages->addWidget(build_header(playlist));
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
        if (!m_current_playlist_id.isEmpty()) {
            open_playlist(m_current_playlist_id);
        } else {
            switch_playlist(0);
        }
    }
}

void PlaylistPage::switch_playlist(int index) {
    if (m_header_pages == nullptr || m_songlist_pages == nullptr
        || index < 0 || index >= static_cast<int>(m_playlist_ids.size())) {
        return;
    }

    m_header_pages->setCurrentIndex(index);
    m_songlist_pages->setCurrentIndex(index);
    m_current_playlist_id = m_playlist_ids[index];

    for (int i = 0; i < static_cast<int>(m_playlist_items.size()); ++i) {
        m_playlist_items[i]->set_selected(i == index);
    }
}

void PlaylistPage::open_playlist(const QString& playlist_id) {
    for (int index = 0; index < static_cast<int>(m_playlist_ids.size()); ++index) {
        if (m_playlist_ids[index] == playlist_id) {
            switch_playlist(index);
            return;
        }
    }

    if (!m_playlist_ids.empty()) {
        switch_playlist(0);
    }
}

void PlaylistPage::setup_connections() {
    QObject::connect(&m_library, &LibraryService::library_changed, this,
        [this] { reload_content(); });
}
