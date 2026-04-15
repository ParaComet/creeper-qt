#include "home_window.hh"
#include "creeper-qt/layout/mixer.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/widget.hh"
#include "all_music_page.hh"
#include "playlist_page.hh"
#include "scan_page.hh"
#include "services/library_service.hh"
#include "style.hh"

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/material-icon.hh"
#include "creeper-qt/utility/theme/color-scheme.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "creeper-qt/layout/group.hh"
#include "creeper-qt/layout/mutual-exclusion-group.hh"
#include "mytheme.hh"
#include "widgets/player_bar.hh"
#include <QFileInfo>
#include <QAbstractButton>
#include <QDesktopServices>
#include <QRandomGenerator>
#include <QUrl>
#include <QWindow>
#include <qnamespace.h>
#include <qpalette.h>
#include <QString>
#include <qwidget.h>

using namespace creeper;

namespace lnpro = linear::pro;
namespace mwpro = main_window::pro;
namespace sty   = mymusic::style;
namespace fc    = filled_card::pro;
namespace sg    = select_group::pro;
namespace ib    = icon_button::pro;

namespace {

auto is_interactive_widget(QWidget* widget) -> bool {
    for (auto* current = widget; current != nullptr; current = current->parentWidget()) {
        if (qobject_cast<QAbstractButton*>(current) != nullptr) {
            return true;
        }
        if (qobject_cast<slider::internal::Slider*>(current) != nullptr) {
            return true;
        }
    }
    return false;
}

}


TopWindow::TopWindow()
    :  m_theme_manager(mymusic::theme_preset::kWarmPaperThemePack)
    // 三套主题预设 mymusic::theme_preset::kWarmPaperThemePack
    // mymusic::theme_preset::kWhiteMistThemePack
    // mymusic::theme_preset::kWhiteMintThemePack
    , m_library(this)
    , m_player(new QMediaPlayer(this))
    , m_audio_output(new QAudioOutput(this))
    , m_pages(new NavHost {
          nav_host::pro::CurrentIndex { 0 },
      }) {
    setup_player();
    if (!m_library.scan_roots().empty()) m_library.scan_library();
    const auto scheme = m_theme_manager.color_scheme();

    apply(widget::pro::Apply { [scheme](QWidget& self) {
        self.setWindowFlag(Qt::FramelessWindowHint, true);
        self.setWindowTitle("MyMusicPlayer");
        self.resize(1280, 800);
        self.setObjectName("top_window");
        self.setStyleSheet(
            QString("#top_window { background: %1; color: %2; }")
                .arg(sty::rgba_css(scheme.background))
                .arg(sty::rgba_css(scheme.on_background)));
    } });

    apply(mwpro::Central<creeper::Widget> { build_central_widget() });
    load_default_track();
    auto mask_window = ( MixerMask* ) {};
    //apply( mixer::pro::SetMixerMask { mask_window } );
    m_theme_manager.set_color_mode(theme::ColorMode::LIGHT);
    setMouseTracking(true);
    
    // 应用主题以更新样式 每次增加widget或控件后都需要调用一次apply_theme来遍历所有widget并应用样式
    m_theme_manager.apply_theme(); 
}

namespace ibpro = icon_button::pro;

auto TopWindow::build_siderbar() -> QWidget* {
    const auto button_features = std::tuple {        
        ibpro::ThemeManager { m_theme_manager },
        ibpro::Font { material::kRoundSmallFont },
        ibpro::ShapeSquare,
        ibpro::FixedSize{ 44, 44 },
        ibpro::TypesToggleUnselected,
    };

    const auto window_button_features = std::tuple {
        ibpro::ThemeManager { m_theme_manager },
        ibpro::Font { material::kRoundSmallFont },
        ibpro::ShapeSquare,
        ibpro::FixedSize { 40, 40 },
        ibpro::ColorStandard,
    };

    m_nav_buttons.clear();
    auto* playlist_button = new IconButton {
        button_features,
        ibpro::TypesToggleSelected,
        ib::ColorFilled,
        ibpro::FontIcon { material::icon::kFavorite },
        ib::Clickable { [this] { switch_page(0); } },
    };
    auto* library_button = new IconButton {
        button_features,
        ibpro::TypesToggleUnselected,
        ib::ColorFilled,
        ibpro::FontIcon { material::icon::kMenu },
        ib::Clickable { [this] { switch_page(1); } },
    };
    auto* scan_button = new IconButton {
        button_features,
        ibpro::TypesToggleUnselected,
        ib::ColorFilled,
        ibpro::FontIcon { material::icon::kFolder },
        ib::Clickable { [this] { switch_page(2); } },
    };
    m_nav_buttons = { playlist_button, library_button, scan_button };

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 22 },
        fc::FixedWidth { 80 },

        fc::Layout<Col> {
            lnpro::ContentsMargin { {12, 20, 12, 12} },
            lnpro::Spacing { 12 },
            lnpro::Item<IconButton> {
                { 0, Qt::AlignHCenter },
                playlist_button,
            },
            lnpro::Item<IconButton> {
                { 0, Qt::AlignHCenter },
                library_button,
            },
            lnpro::Item<IconButton> {
                { 0, Qt::AlignHCenter },
                scan_button,
            },
            lnpro::Stretch { 1 },
            lnpro::SpacingItem { 16 },
            lnpro::Item<Col> {
                { 0, Qt::AlignHCenter },
                lnpro::Margin { 0 },
                lnpro::Spacing { 10 },
                lnpro::Item<IconButton> {
                    { 0, Qt::AlignHCenter },
                    window_button_features,
                    ibpro::FontIcon { QStringLiteral("remove") },
                    ibpro::Clickable { [this] { showMinimized(); } },
                },
                lnpro::Item<IconButton> {
                    { 0, Qt::AlignHCenter },
                    window_button_features,
                    ibpro::FontIcon { QStringLiteral("crop_square") },
                    ibpro::Clickable { [this] {
                        isMaximized() ? showNormal() : showMaximized();
                    } },
                },
                lnpro::Item<IconButton> {
                    { 0, Qt::AlignHCenter },
                    window_button_features,
                    //ibpro::ColorOutlined,
                    ibpro::FontIcon { material::icon::kLogout },
                    ibpro::Clickable { [this] { close(); } },
                },
            },
            //lnpro::Stretch { 1 },
        },
        // fc::Layout<Col> {
        //     lnpro::ContentsMargin { {12, 20, 12, 12} },
        //     lnpro::Spacing { 12 },
        //     lnpro::Item<IconButton> {
        //         { 0, Qt::AlignCenter },
        //         button_features,
        //         ibpro::FontIcon { material::icon::kFavorite },
        //         ibpro::Clickable { [this] { switch_page(0); } },
        //     },
        //     lnpro::Item<IconButton> {
        //         { 0, Qt::AlignCenter },
        //         button_features,
        //         ibpro::FontIcon { material::icon::kMenu },
        //         ibpro::Clickable { [this] { switch_page(1); } },
        //     },
        //     lnpro::Item<IconButton> {
        //         { 0, Qt::AlignCenter },
        //         button_features,
        //         ibpro::FontIcon { material::icon::kFolder },
        //         ibpro::Clickable { [this] { switch_page(2); } },
        //     },
        //     lnpro::Stretch { 1 },
        // },
    };
}

auto TopWindow::build_content_pages() -> creeper::Widget* {
    auto* playlist_page = new PlaylistPage { m_theme_manager, m_library };
    // auto* playlist_page = new creeper::Widget {
    //     widget::pro::Layout<Col> {
    //         lnpro::ContentsMargin { {24, 24, 24, 24} },
    //         lnpro::Item<Text> {
    //             text::pro::ThemeManager { m_theme_manager },
    //             text::pro::Text { "歌单页面" },
    //         },
    //     },
    //     widget::pro::Apply { [scheme](QWidget& self) {
    //         self.setStyleSheet(panel_style(scheme));
    //     } },
    // };

    auto* all_music_page = new AllMusicPage { m_theme_manager, m_library };

    auto* scan_page = new ScanPage { m_theme_manager, m_library };

    QObject::connect(playlist_page, &PlaylistPage::song_activated, this,
        [this](const QString& source_id, const QString& song_id) {
            play_song_from_source(source_id, song_id);
        });
    QObject::connect(all_music_page, &AllMusicPage::song_activated, this,
        [this](const QString& source_id, const QString& song_id) {
            play_song_from_source(source_id, song_id);
        });
    QObject::connect(all_music_page, &AllMusicPage::play_all_requested, this,
        [this](const QString& source_id) { play_source_from_beginning(source_id); });
    QObject::connect(all_music_page, &AllMusicPage::enqueue_requested, this,
        [this](const QString& source_id) { enqueue_source(source_id); });
    QObject::connect(all_music_page, &AllMusicPage::entity_favorite_requested, this,
        [this](const QString& source_id, bool liked) {
            m_library.set_source_entity_liked(source_id, liked);
        });
    QObject::connect(all_music_page, &AllMusicPage::reveal_requested, this,
        [this](const QString& source_id) {
            const auto directory = m_library.first_source_directory(source_id);
            if (!directory.isEmpty()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
            }
        });

    m_widget_map.emplace(playlist_page, DisplayWidget { playlist_page, "playlist", "歌单" });
    m_widget_map.emplace(all_music_page, DisplayWidget { all_music_page, "library", "全部音乐" });
    m_widget_map.emplace(scan_page, DisplayWidget { scan_page, "scan", "扫描文件夹" });

    m_pages->addWidget(playlist_page);
    m_pages->addWidget(all_music_page);
    m_pages->addWidget(scan_page);

    auto* content = new creeper::Widget {};
    content->setLayout(m_pages);
    return content;
}

auto TopWindow::build_player_bar() -> creeper::Widget* {
    m_player_bar = new PlayerBar { m_theme_manager, this };
    update_player_bar_order();

    QObject::connect(m_player_bar, &PlayerBar::play_pause_clicked, this, [this] {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->pause();
        } else if (m_player->source().isEmpty()) {
            play_song_at_index(m_current_queue_index >= 0 ? m_current_queue_index : 0);
        } else {
            m_player->play();
        }
    });
    QObject::connect(m_player_bar, &PlayerBar::previous_clicked, this,
        [this] { play_previous_track(); });
    QObject::connect(m_player_bar, &PlayerBar::next_clicked, this,
        [this] { play_next_track(); });
    QObject::connect(m_player_bar, &PlayerBar::seek_requested, this,
        [this](double progress) {
            if (m_player->duration() > 0) {
                m_player->setPosition(static_cast<qint64>(progress * m_player->duration()));
            }
        });
    QObject::connect(m_player_bar, &PlayerBar::volume_changed, this,
        [this](double volume) { m_audio_output->setVolume(volume); });
    QObject::connect(m_player_bar, &PlayerBar::order_clicked, this,
        [this] { cycle_playback_order(); });
    QObject::connect(m_player_bar, &PlayerBar::favorite_toggled, this,
        [this](bool checked) {
            if (m_current_queue_index < 0 || m_current_queue_index >= static_cast<int>(m_playback_queue.size())) {
                return;
            }
            const auto& song = m_playback_queue[m_current_queue_index];
            if (m_library.set_song_liked(song.id, checked)) {
                m_playback_queue[m_current_queue_index].liked = checked;
            }
        });

    return m_player_bar;
    // return new creeper::Widget {
    //     widget::pro::FixedHeight { 100 },
    //     widget::pro::Apply { [scheme](QWidget& self) {
    //         self.setObjectName("elevated_panel");
    //         self.setStyleSheet(sty::elevated_panel_style(scheme));
    //     } },
    //     widget::pro::Layout<Row> {
    //         lnpro::ContentsMargin { {16, 12, 16, 12} },
    //         lnpro::Spacing { 12 },

    //         lnpro::Item<IconButton> {
    //             icon_button::pro::ThemeManager { m_theme_manager },
    //         },
    //         lnpro::Item<Text> {
    //             text::pro::SizePolicy { QSizePolicy::Expanding, QSizePolicy::Preferred },
    //             text::pro::ThemeManager { m_theme_manager },
    //             text::pro::Text { "当前未播放" },
    //             widget::pro::Apply { [](Text& self) {
    //                 self.setStyleSheet("QLabel { background: transparent; border: none;}");
    //             } },
    //         },
    //         //lnpro::Stretch { 1 },
    //         lnpro::Item<Slider> {
                
    //             lnpro::Item<Slider>::LayoutMethod { 1 },
    //             slider::pro::ThemeManager { m_theme_manager },
    //             slider::pro::Progress { 0.0 },
    //         },
    //     },
    // };
}

auto TopWindow::build_central_widget() -> creeper::Widget* {
    return new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 0 },

            lnpro::Item<Row> {
                lnpro::Item<Row>::LayoutMethod { 1 },
                lnpro::Margin { 0 }, 
                lnpro::ContentsMargin { {10, 10, 10, 10} },
                lnpro::Spacing { 12 },
                lnpro::Item { build_siderbar() },
                lnpro::Item<creeper::Widget> {
                    lnpro::Item<creeper::Widget>::LayoutMethod { 1 },
                    build_content_pages(),
                },
            },
            lnpro::Item { build_player_bar() },
        },
    };
}

void TopWindow::switch_page(int index) {
    if (m_pages == nullptr) {
        return;
    }
    m_pages->setCurrentIndex(index);
    for (int button_index = 0; button_index < static_cast<int>(m_nav_buttons.size()); ++button_index) {
        m_nav_buttons[button_index]->set_selected(button_index == index);
    }
}

void TopWindow::setup_player() {
    m_player->setAudioOutput(m_audio_output);
    m_audio_output->setVolume(0.6);

    QObject::connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (m_player_bar == nullptr) {
            return;
        }
        const auto duration = m_player->duration();
        m_player_bar->set_progress(duration > 0 ? static_cast<double>(position) / duration : 0.0);
    });
    QObject::connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        if (m_player_bar == nullptr || duration <= 0) {
            return;
        }
        m_player_bar->set_progress(duration > 0
                ? static_cast<double>(m_player->position()) / duration
                : 0.0);
    });
    QObject::connect(m_player, &QMediaPlayer::playbackStateChanged, this,
        [this](QMediaPlayer::PlaybackState state) {
            if (m_player_bar != nullptr) {
                m_player_bar->set_playing(state == QMediaPlayer::PlayingState);
            }
        });
    QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, this,
        [this](QMediaPlayer::MediaStatus status) {
            if (status != QMediaPlayer::EndOfMedia) {
                return;
            }
            if (m_playback_order == PlaybackOrder::RepeatOne) {
                play_song_at_index(m_current_queue_index);
                return;
            }
            play_next_track();
        });
}

/**
 * @brief 从数据库中获取指定来源的播放列表 用于播放和添加到播放队列
 * 
 * @param source_id 播放来源ID 可以是歌单ID、专辑ID、艺术家ID等
 * @return std::vector<mymusic::model::SongInfo> 来源的歌曲列表 如果来源无效
 *         或没有歌曲则返回全部音乐列表或空列表
 */
auto TopWindow::playback_queue_for_source(const QString& source_id) const
    -> std::vector<mymusic::model::SongInfo> {
    const auto songs = m_library.songs_for_source(source_id);
    return songs.empty() ? m_library.songs() : songs;
}

void TopWindow::enqueue_source(const QString& source_id) {
    const auto source_queue = playback_queue_for_source(source_id);
    if (source_queue.empty()) {
        return;
    }

    if (m_playback_queue.empty()) {
        m_playback_queue = source_queue;
        m_current_queue_index = 0;
        const auto& song = m_playback_queue.front();
        if (m_player_bar != nullptr) {
            m_player_bar->set_track_info(song.title, song.artist, song.album);
            m_player_bar->set_cover(
                song.cover_badge.isEmpty() ? QStringLiteral("封面") : song.cover_badge,
                song.cover_path);
            m_player_bar->set_favorite(song.liked);
            m_player_bar->set_progress(0.0);
            m_player_bar->set_playing(false);
        }
        return;
    }

    m_playback_queue.insert(m_playback_queue.end(), source_queue.begin(), source_queue.end());
}
/**
 * @brief 从指定来源播放歌曲
 * 
 * @param source_id 歌曲所在的来源ID 可以是歌单ID、专辑ID、艺术家ID等
 * @param song_id 歌曲ID 如果在来源中找不到该歌曲则不执行任何操作
 */
void TopWindow::play_song_from_source(const QString& source_id, const QString& song_id) {
    m_playback_queue = playback_queue_for_source(source_id);
    m_current_queue_index = -1;

    for (int index = 0; index < static_cast<int>(m_playback_queue.size()); ++index) {
        if (m_playback_queue[index].id == song_id) {
            m_current_queue_index = index;
            break;
        }
    }

    if (m_current_queue_index < 0) {
        return;
    }

    play_song_at_index(m_current_queue_index);
}

void TopWindow::play_source_from_beginning(const QString& source_id) {
    m_playback_queue = playback_queue_for_source(source_id);
    if (m_playback_queue.empty()) {
        return;
    }

    play_song_at_index(0);
}

void TopWindow::play_song_at_index(int index) {
    if (index < 0 || index >= static_cast<int>(m_playback_queue.size())) {
        return;
    }

    m_current_queue_index = index;
    const auto& song = m_playback_queue[index];

    if (m_player_bar != nullptr) {
        m_player_bar->set_track_info(song.title, song.artist, song.album);
        m_player_bar->set_cover(
            song.cover_badge.isEmpty() ? QStringLiteral("封面") : song.cover_badge,
            song.cover_path);
        m_player_bar->set_favorite(song.liked);
        m_player_bar->set_progress(0.0);
    }

    if (song.file_path.isEmpty() || !QFileInfo::exists(song.file_path)) {
        m_player->stop();
        if (m_player_bar != nullptr) {
            m_player_bar->set_playing(false);
        }
        return;
    }

    m_player->setSource(QUrl::fromLocalFile(song.file_path));
    m_player->play();
    if (m_player_bar != nullptr) {
        m_player_bar->set_playing(true);
    }
}

void TopWindow::play_next_track() {
    if (m_playback_queue.empty()) {
        return;
    }

    if (m_playback_order == PlaybackOrder::Shuffle) {
        if (m_playback_queue.size() == 1) {
            play_song_at_index(0);
            return;
        }

        auto next_index = m_current_queue_index;
        while (next_index == m_current_queue_index) {
            next_index = QRandomGenerator::global()->bounded(static_cast<int>(m_playback_queue.size()));
        }
        play_song_at_index(next_index);
        return;
    }

    auto next_index = m_current_queue_index + 1;
    if (next_index >= static_cast<int>(m_playback_queue.size())) {
        if (m_playback_order == PlaybackOrder::RepeatAll) {
            next_index = 0;
        } else {
            m_player->stop();
            if (m_player_bar != nullptr) {
                m_player_bar->set_playing(false);
            }
            return;
        }
    }

    play_song_at_index(next_index);
}

void TopWindow::play_previous_track() {
    if (m_playback_queue.empty()) {
        return;
    }

    auto previous_index = m_current_queue_index - 1;
    if (previous_index < 0) {
        previous_index = (m_playback_order == PlaybackOrder::RepeatAll)
            ? static_cast<int>(m_playback_queue.size()) - 1
            : 0;
    }

    play_song_at_index(previous_index);
}

void TopWindow::load_default_track() {
    const auto liked_queue = playback_queue_for_source(QStringLiteral("playlist-library-liked"));
    if (!liked_queue.empty()) {
        m_playback_queue = liked_queue;
        m_current_queue_index = 0;
    } else {
        m_playback_queue = m_library.songs();
        m_current_queue_index = m_playback_queue.empty() ? -1 : 0;
    }

    if (m_current_queue_index < 0 || m_current_queue_index >= static_cast<int>(m_playback_queue.size())) {
        return;
    }

    const auto& song = m_playback_queue[m_current_queue_index];
    if (m_player_bar != nullptr) {
        m_player_bar->set_track_info(song.title, song.artist, song.album);
        m_player_bar->set_cover(
            song.cover_badge.isEmpty() ? QStringLiteral("封面") : song.cover_badge,
            song.cover_path);
        m_player_bar->set_favorite(song.liked);
        m_player_bar->set_progress(0.0);
        m_player_bar->set_playing(false);
    }
}

void TopWindow::cycle_playback_order() {
    switch (m_playback_order) {
    case PlaybackOrder::Sequential:
        m_playback_order = PlaybackOrder::RepeatAll;
        break;
    case PlaybackOrder::RepeatAll:
        m_playback_order = PlaybackOrder::RepeatOne;
        break;
    case PlaybackOrder::RepeatOne:
        m_playback_order = PlaybackOrder::Shuffle;
        break;
    case PlaybackOrder::Shuffle:
        m_playback_order = PlaybackOrder::Sequential;
        break;
    }

    update_player_bar_order();
}

void TopWindow::update_player_bar_order() {
    if (m_player_bar == nullptr) {
        return;
    }

    switch (m_playback_order) {
    case PlaybackOrder::Sequential:
        m_player_bar->set_order_icon(QStringLiteral("reorder"));
        break;
    case PlaybackOrder::RepeatAll:
        m_player_bar->set_order_icon(QStringLiteral("repeat"));
        break;
    case PlaybackOrder::RepeatOne:
        m_player_bar->set_order_icon(QStringLiteral("repeat_one"));
        break;
    case PlaybackOrder::Shuffle:
        m_player_bar->set_order_icon(QStringLiteral("shuffle"));
        break;
    }
}

auto TopWindow::resize_edges_for_pos(const QPoint& pos) const -> Qt::Edges {
    constexpr auto kResizeMargin = 8;

    Qt::Edges edges {};
    if (pos.x() <= kResizeMargin) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= width() - kResizeMargin) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() <= kResizeMargin) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= height() - kResizeMargin) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

void TopWindow::update_window_cursor(const QPoint& pos) {
    const auto edges = resize_edges_for_pos(pos);

    Qt::CursorShape shape = Qt::ArrowCursor;
    if (edges == (Qt::TopEdge | Qt::LeftEdge) || edges == (Qt::BottomEdge | Qt::RightEdge)) {
        shape = Qt::SizeFDiagCursor;
    } else if (edges == (Qt::TopEdge | Qt::RightEdge) || edges == (Qt::BottomEdge | Qt::LeftEdge)) {
        shape = Qt::SizeBDiagCursor;
    } else if ((edges & Qt::LeftEdge) || (edges & Qt::RightEdge)) {
        shape = Qt::SizeHorCursor;
    } else if ((edges & Qt::TopEdge) || (edges & Qt::BottomEdge)) {
        shape = Qt::SizeVerCursor;
    }

    setCursor(shape);
}

void TopWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (auto* handle = windowHandle(); handle != nullptr) {
            const auto edges = resize_edges_for_pos(event->pos());
            if (edges != Qt::Edges {}) {
                handle->startSystemResize(edges);
                event->accept();
                return;
            }

            if (!is_interactive_widget(childAt(event->pos()))) {
                handle->startSystemMove();
                event->accept();
                return;
            }
        }
    }

    QMainWindow::mousePressEvent(event);
}

void TopWindow::mouseMoveEvent(QMouseEvent* event) {
    update_window_cursor(event->pos());
    QMainWindow::mouseMoveEvent(event);
}

void TopWindow::leaveEvent(QEvent* event) {
    unsetCursor();
    QMainWindow::leaveEvent(event);
}
