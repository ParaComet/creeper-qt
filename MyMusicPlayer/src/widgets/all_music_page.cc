#include "all_music_page.hh"

#include "style.hh"

#include "creeper-qt/layout/group.hh"
#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/layout/mutual-exclusion-group.hh"
#include "creeper-qt/utility/material-icon.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/text.hh"

#include <QVBoxLayout>

using namespace creeper;
namespace fc    = filled_card::pro;
namespace ib    = icon_button::pro;
namespace lnpro = linear::pro;
namespace sg    = select_group::pro;
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
    m_views = new NavHost { nav_host::pro::CurrentIndex { 0 } };
    m_views->addWidget(build_songs_view());
    m_views->addWidget(build_artists_view());
    m_views->addWidget(build_albums_view());

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
            lnpro::Item { sty::make_cover_label(m_theme_manager.color_scheme(), "LIB", QSize { 88, 88 },
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
                    text::pro::Text { "这里适合做资料库浏览、排序和快速播放入口。" },
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
                            index == 0 ? ib::TypesToggleSelected : ib::TypesToggleUnselected,
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

auto AllMusicPage::build_songs_view() -> QWidget* {
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
        },
    };

    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    for (const auto& song : m_library.songs()) {
        layout->addWidget(build_song_row(song));
    }
    layout->addStretch(1);
    return panel;
}

auto AllMusicPage::build_artists_view() -> QWidget* {
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

    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    for (const auto& artist : m_library.artists()) {
        layout->addWidget(build_artist_card(artist));
    }
    layout->addStretch(1);
    return panel;
}

auto AllMusicPage::build_albums_view() -> QWidget* {
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

    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    for (const auto& album : m_library.albums()) {
        layout->addWidget(build_album_card(album));
    }
    layout->addStretch(1);
    return panel;
}

auto AllMusicPage::build_song_row(const model::SongInfo& song) -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();

    return new creeper::Widget {
        widget::pro::Apply { [scheme](QWidget& self) {
            self.setObjectName("song_row");
            self.setStyleSheet(sty::song_row_style(scheme));
        } },
        widget::pro::Layout<Row> {
            lnpro::ContentsMargin { { 14, 12, 14, 12 } },
            lnpro::Spacing { 12 },
            lnpro::Item<Text> {
                text::pro::Text { QStringLiteral("%1").arg(song.track_number, 2, 10, QChar('0')) },
                sty::meta_text_color(scheme),
            },
            lnpro::Item { sty::make_cover_label(scheme, song.cover_badge, QSize { 44, 44 },
                scheme.tertiary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 2 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { song.title },
                },
                lnpro::Item<Text> {
                    text::pro::Text { song.artist },
                    sty::meta_text_color(scheme),
                },
            },
            lnpro::Item<Text> {
                text::pro::Text { song.album },
                sty::meta_text_color(scheme),
            },
            lnpro::Item<Text> {
                text::pro::Text { song.duration_text },
                sty::meta_text_color(scheme),
            },
        },
    };
}

auto AllMusicPage::build_artist_card(const model::ArtistInfo& artist) -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelLow,
        fc::Radius { 20 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 16, 16, 16, 16 } },
            lnpro::Spacing { 14 },
            lnpro::Item { sty::make_cover_label(scheme, artist.cover_badge, QSize { 64, 64 },
                scheme.secondary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 4 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { artist.name },
                },
                lnpro::Item<Text> {
                    text::pro::Text { artist.subtitle },
                    sty::meta_text_color(scheme),
                },
            },
            lnpro::Item<Text> {
                text::pro::Text {
                    QStringLiteral("%1 首歌 · %2 张专辑").arg(artist.song_count).arg(artist.album_count) },
                sty::meta_text_color(scheme),
            },
        },
    };
}

auto AllMusicPage::build_album_card(const model::AlbumInfo& album) -> QWidget* {
    const auto scheme = m_theme_manager.color_scheme();

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelLow,
        fc::Radius { 20 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 16, 16, 16, 16 } },
            lnpro::Spacing { 14 },
            lnpro::Item { sty::make_cover_label(scheme, album.cover_badge, QSize { 64, 64 },
                scheme.primary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 4 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { album.title },
                },
                lnpro::Item<Text> {
                    text::pro::Text { album.artist },
                    sty::meta_text_color(scheme),
                },
            },
            lnpro::Item<Text> {
                text::pro::Text {
                    QStringLiteral("%1 首歌 · %2").arg(album.track_count).arg(album.subtitle) },
                sty::meta_text_color(scheme),
            },
        },
    };
}

void AllMusicPage::switch_view(int index) {
    if (m_views == nullptr) {
        return;
    }
    m_views->setCurrentIndex(index);
}
