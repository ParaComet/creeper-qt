#include "all_music_page.hh"

#include "style.hh"

#include "creeper-qt/layout/group.hh"
#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/layout/mutual-exclusion-group.hh"
#include "creeper-qt/layout/scroll.hh"
#include "creeper-qt/utility/material-icon.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/text.hh"

#include <QVBoxLayout>
#include <algorithm>

using namespace creeper;
using mycomponent::PlaylistItem;
using mycomponent::SongItem;
namespace fc    = filled_card::pro;
namespace ib    = icon_button::pro;
namespace lnpro = linear::pro;
namespace pp    = mycomponent::playlist_item::pro;
namespace scrollpro = creeper::scroll::pro;
namespace sg    = select_group::pro;
namespace sp    = mycomponent::song_item::pro;
namespace sty   = mymusic::style;
namespace model = mymusic::model;

AllMusicPage::AllMusicPage(ThemeManager& manager, LibraryService& library)
    : m_theme_manager(manager)
    , m_library(library)
{
    m_root_layout = new QVBoxLayout {};
    m_root_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_root_layout);
    reload_content();
    QObject::connect(&m_library, &LibraryService::library_changed, this,
        [this] { reload_content(); });
}

auto AllMusicPage::build_content() -> creeper::Widget* {
    if (m_detail_mode != DetailMode::None) {
        auto* detail = new creeper::Widget {};
        auto* detail_layout = new QVBoxLayout {};
        detail_layout->setContentsMargins(0, 0, 0, 0);
        detail_layout->setSpacing(8);
        detail_layout->addWidget(build_detail_header());
        detail_layout->addWidget(build_detail_song_list(), 1);
        detail->setLayout(detail_layout);
        return detail;
    }

    m_views = new NavHost { nav_host::pro::CurrentIndex { 0 } };
    m_views->addWidget(build_songs_view());
    m_views->addWidget(build_artists_view());
    m_views->addWidget(build_albums_view());
    m_views->setCurrentIndex(m_current_view_index);

    auto* view_host = new creeper::Widget {};
    view_host->setLayout(m_views);

    return new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::ContentsMargin { { 0, 0, 0, 0 } },
            lnpro::Spacing { 8 },
            lnpro::Item { build_header_card() },
            lnpro::Item { build_view_switch() },
            lnpro::Item<Widget> {
                lnpro::Item<Widget>::LayoutMethod { 1 },
                view_host,
            },
        },
    };
}

void AllMusicPage::reload_content() {
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
}

auto AllMusicPage::build_header_card() -> QWidget* {
    const auto subtitle = QStringLiteral("%1 首歌曲 · %2 位歌手 · %3 张专辑")
                              .arg(m_library.songs().size())
                              .arg(m_library.artists().size())
                              .arg(m_library.albums().size());

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 22, 20, 22, 20 } },
            lnpro::Spacing { 18 },
            lnpro::Item { sty::make_cover_label(m_theme_manager, "LIB", QSize { 88, 88 },
                m_theme_manager.color_scheme().primary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 6 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { "全部音乐" },
                },
                lnpro::Item<Text> {
                    text::pro::Text { subtitle },
                    sty::meta_text_color(m_theme_manager.color_scheme()),
                },
                lnpro::Item<Text> {
                    text::pro::Text { "快速浏览音乐库，提供简单分类功能～" },
                    sty::meta_text_color(m_theme_manager.color_scheme()),
                },
            },
        },
    };
}

auto AllMusicPage::build_view_switch() -> QWidget* {
    const auto button_features = std::tuple {
        ib::ThemeManager { m_theme_manager },
        ib::Font { material::kRoundSmallFont },
        ib::FixedSize { 44, 44 },
        ib::ColorStandard,
        ib::ShapeRound,
        ib::TypesToggleUnselected,
    };

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelLow,
        fc::Radius { 20 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 14, 10, 14, 10 } },
            lnpro::Item<SelectGroup<Row, IconButton>> {
                { 0, Qt::AlignLeft | Qt::AlignVCenter },
                lnpro::Margin { 0 },
                lnpro::Spacing { 12 },
                sg::Compose {
                    std::array {
                        std::pair { 0, QStringLiteral("library_music") },
                        std::pair { 1, QStringLiteral("person") },
                        std::pair { 2, QStringLiteral("album") },
                    },
                    [this, button_features](int index, const QString& icon_name) {
                        return new IconButton {
                            button_features,
                            index == m_current_view_index ? ib::TypesToggleSelected
                                                          : ib::TypesToggleUnselected,
                            ib::ColorFilled,
                            ib::FontIcon { icon_name },
                            ib::Clickable { [this, index] { switch_view(index); } },
                        };
                    },
                    Qt::AlignVCenter,
                },
                sg::SignalInjection { &IconButton::clicked },
            },
            lnpro::Stretch { 1 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "歌曲 / 歌手 / 专辑" },
                sty::meta_text_color(m_theme_manager.color_scheme()),
            },
        },
    };
}

auto AllMusicPage::build_detail_header() -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();

    QString title;
    QString description;
    QString meta;
    QString badge;
    QString cover_path;
    QColor accent = scheme.primary_container;
    bool liked = false;

    if (m_detail_mode == DetailMode::Artist) {
        title = m_detail_artist.name;
        description = m_detail_artist.subtitle;
        meta = QStringLiteral("%1 首歌 · %2 张专辑").arg(m_detail_artist.song_count).arg(m_detail_artist.album_count);
        badge = m_detail_artist.cover_badge;
        cover_path = m_detail_artist.cover_path;
        accent = scheme.secondary_container;
        liked = m_library.source_entity_liked(m_detail_source_id);
    } else {
        title = m_detail_album.title;
        description = QStringLiteral("%1 · %2").arg(m_detail_album.artist, m_detail_album.subtitle);
        meta = QStringLiteral("%1 首歌").arg(m_detail_album.track_count);
        badge = m_detail_album.cover_badge;
        cover_path = m_detail_album.cover_path;
        accent = scheme.primary_container;
        liked = m_library.source_entity_liked(m_detail_source_id);
    }

    if (liked) {
        description = description.isEmpty()
            ? QStringLiteral("已收藏")
            : QStringLiteral("已收藏 · %1").arg(description);
    }

    auto* action_bar = new creeper::Widget {};
    auto* action_layout = new QHBoxLayout {};
    action_layout->setContentsMargins(0, 0, 0, 0);
    action_layout->setSpacing(10);
    action_bar->setLayout(action_layout);

    const auto button_features = std::tuple {
        ib::ThemeManager { m_theme_manager },
        ib::Font { material::kRoundSmallFont },
        ib::ShapeRound,
        ib::FixedSize { 42, 42 },
        ib::ColorStandard,
    };

    auto* back_button = new IconButton {
        button_features,
        ib::FontIcon { QStringLiteral("arrow_back") },
        ib::Clickable { [this] { leave_detail(); } },
    };
    back_button->setToolTip(QStringLiteral("返回全部音乐"));
    action_layout->addWidget(back_button);

    auto* play_all_button = new IconButton {
        button_features,
        ib::ColorFilled,
        ib::FontIcon { QStringLiteral("play_arrow") },
        ib::Clickable { [this] { emit play_all_requested(m_detail_source_id); } },
    };
    play_all_button->setToolTip(QStringLiteral("播放全部"));
    action_layout->addWidget(play_all_button);

    auto* enqueue_button = new IconButton {
        button_features,
        ib::FontIcon { QStringLiteral("queue_music") },
        ib::Clickable { [this] { emit enqueue_requested(m_detail_source_id); } },
    };
    enqueue_button->setToolTip(QStringLiteral("加入队列"));
    action_layout->addWidget(enqueue_button);

    auto* favorite_button = new IconButton {
        button_features,
        liked ? ib::TypesToggleSelected : ib::TypesToggleUnselected,
        ib::ColorFilled,
        ib::FontIcon { material::icon::kFavorite },
        ib::Clickable { [this](IconButton& self) {
            emit entity_favorite_requested(m_detail_source_id, self.selected());
        } },
    };
    favorite_button->set_selected(liked);
    favorite_button->setToolTip(QStringLiteral("收藏"));
    action_layout->addWidget(favorite_button);

    auto* reveal_button = new IconButton {
        button_features,
        ib::FontIcon { QStringLiteral("folder_open") },
        ib::Clickable { [this] { emit reveal_requested(m_detail_source_id); } },
    };
    reveal_button->setToolTip(QStringLiteral("在文件夹中显示"));
    action_layout->addWidget(reveal_button);
    action_layout->addStretch(1);

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 22, 22, 22, 22 } },
            lnpro::Spacing { 18 },
            lnpro::Item { sty::make_cover_label(m_theme_manager, badge, QSize { 124, 124 }, accent, cover_path) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 8 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { title },
                },
                lnpro::Item<Text> {
                    text::pro::Text { description },
                    sty::meta_text_color(scheme),
                },
                lnpro::Item<Text> {
                    text::pro::Text { meta },
                    text::pro::Color { scheme.primary },
                },
                lnpro::Item { action_bar },
            },
        },
    };
}

auto AllMusicPage::build_detail_song_list() -> QWidget* {
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 10 },
        },
    };
    auto* list_layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (const auto& song : m_library.songs_for_source(m_detail_source_id)) {
        list_layout->addWidget(build_song_row(m_detail_source_id, song));
    }
    list_layout->addStretch(1);

    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "歌曲列表" },
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

auto AllMusicPage::build_songs_view() -> QWidget* {
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 10 },
        },
    };
    auto* list_layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (const auto& song : m_library.songs()) {
        list_layout->addWidget(build_song_row(QStringLiteral("all-music"), song));
    }
    list_layout->addStretch(1);

    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "歌曲列表" },
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

auto AllMusicPage::build_artists_view() -> QWidget* {
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 10 },
        },
    };
    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "歌手视图" },
            },
        },
    };

    auto artists = m_library.artists();
    std::sort(artists.begin(), artists.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.liked != rhs.liked) {
            return lhs.liked > rhs.liked;
        }
        if (lhs.song_count != rhs.song_count) {
            return lhs.song_count > rhs.song_count;
        }
        return lhs.name.localeAwareCompare(rhs.name) < 0;
    });

    auto* layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (const auto& artist : artists) {
        layout->addWidget(build_artist_card(artist));
    }
    layout->addStretch(1);

    auto* panel_layout = qobject_cast<QVBoxLayout*>(panel->layout());
    panel_layout->addWidget(new ScrollArea {
        scrollpro::ThemeManager { m_theme_manager },
        scrollpro::HorizontalScrollBarPolicy { Qt::ScrollBarAlwaysOff },
        scrollpro::Item { list_content },
    });
    return panel;
}

auto AllMusicPage::build_albums_view() -> QWidget* {
    auto* list_content = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 10 },
        },
    };
    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "专辑视图" },
            },
        },
    };

    auto albums = m_library.albums();
    std::sort(albums.begin(), albums.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.liked != rhs.liked) {
            return lhs.liked > rhs.liked;
        }
        if (lhs.track_count != rhs.track_count) {
            return lhs.track_count > rhs.track_count;
        }
        return lhs.title.localeAwareCompare(rhs.title) < 0;
    });

    auto* layout = qobject_cast<QVBoxLayout*>(list_content->layout());
    for (const auto& album : albums) {
        layout->addWidget(build_album_card(album));
    }
    layout->addStretch(1);

    auto* panel_layout = qobject_cast<QVBoxLayout*>(panel->layout());
    panel_layout->addWidget(new ScrollArea {
        scrollpro::ThemeManager { m_theme_manager },
        scrollpro::HorizontalScrollBarPolicy { Qt::ScrollBarAlwaysOff },
        scrollpro::Item { list_content },
    });
    return panel;
}

auto AllMusicPage::build_song_row(const QString& source_id, const model::SongInfo& song)
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

auto AllMusicPage::build_artist_card(const model::ArtistInfo& artist) -> QWidget* {
    const auto meta = QStringLiteral("%1%2 · %3 首歌 · %4 张专辑")
                          .arg(artist.liked ? QStringLiteral("已收藏 · ") : QString())
                          .arg(artist.subtitle)
                          .arg(artist.song_count)
                          .arg(artist.album_count);

    return new PlaylistItem {
        pp::ThemeManager { m_theme_manager },
        pp::Title { artist.name },
        pp::Meta { meta },
        pp::Badge { artist.cover_badge },
        pp::CoverPath { artist.cover_path },
        pp::Marked { artist.liked },
        pp::Clickable { [this, artist] { open_artist_detail(artist); } },
    };
}

auto AllMusicPage::build_album_card(const model::AlbumInfo& album) -> QWidget* {
    const auto meta = QStringLiteral("%1%2 · %3 首歌 · %4")
                          .arg(album.liked ? QStringLiteral("已收藏 · ") : QString())
                          .arg(album.artist)
                          .arg(album.track_count)
                          .arg(album.subtitle);

    return new PlaylistItem {
        pp::ThemeManager { m_theme_manager },
        pp::Title { album.title },
        pp::Meta { meta },
        pp::Badge { album.cover_badge },
        pp::CoverPath { album.cover_path },
        pp::Marked { album.liked },
        pp::Clickable { [this, album] { open_album_detail(album); } },
    };
}

void AllMusicPage::switch_view(int index) {
    m_current_view_index = index;
    if (m_views == nullptr) {
        return;
    }
    m_views->setCurrentIndex(index);
}

void AllMusicPage::open_artist_detail(const model::ArtistInfo& artist) {
    m_detail_origin_view_index = 1;
    m_detail_mode = DetailMode::Artist;
    m_detail_artist = artist;
    m_detail_source_id = artist.id;
    reload_content();
}

void AllMusicPage::open_album_detail(const model::AlbumInfo& album) {
    m_detail_origin_view_index = 2;
    m_detail_mode = DetailMode::Album;
    m_detail_album = album;
    m_detail_source_id = album.id;
    reload_content();
}

void AllMusicPage::leave_detail() {
    m_detail_mode = DetailMode::None;
    m_detail_source_id.clear();
    m_current_view_index = m_detail_origin_view_index;
    reload_content();
}
