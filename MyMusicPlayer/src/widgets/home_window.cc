#include "home_window.hh"
#include "creeper-qt/layout/mixer.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include "creeper-qt/widget/widget.hh"
#include "all_music_page.hh"
#include "playlist_page.hh"
#include "scan_page.hh"
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


TopWindow::TopWindow()
    :  m_theme_manager(myThemePack)
    , m_library(this)
    , m_player(new QMediaPlayer(this))
    , m_audio_output(new QAudioOutput(this))
    , m_pages(new NavHost {
          nav_host::pro::CurrentIndex { 0 },
      }) {
    setup_player();
    const auto scheme = m_theme_manager.color_scheme();

    apply(widget::pro::Apply { [scheme](QWidget& self) {
        self.setWindowTitle("MyMusicPlayer");
        self.resize(1280, 800);
        self.setObjectName("top_window");
        self.setStyleSheet(
            QString("#top_window { background: %1; color: %2; }")
                .arg(sty::rgba_css(scheme.background))
                .arg(sty::rgba_css(scheme.on_background)));
    } });

    apply(mwpro::Central<creeper::Widget> { build_central_widget() });
    auto mask_window = ( MixerMask* ) {};
    //apply( mixer::pro::SetMixerMask { mask_window } );
    m_theme_manager.set_color_mode(theme::ColorMode::LIGHT);
    
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
    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 22 },
        fc::FixedWidth { 80 },

        fc::Layout<Col> {
            lnpro::ContentsMargin { {12, 20, 12, 12} },
            lnpro::Item<SelectGroup<Col, IconButton>> {
                {0, Qt::AlignHCenter},
                lnpro::Margin { 0 },
                lnpro::Spacing { 12 },
                sg::Compose {
                    std::array {
                        std::pair{0, material::icon::kFavorite},
                        std::pair{1, material::icon::kMenu},
                        std::pair{2, material::icon::kFolder},
                    },
                    [this, button_features](int index, const auto& icon) {
                        return new IconButton {
                            button_features,
                            index == 0 ? ibpro::TypesToggleSelected : ibpro::TypesToggleUnselected,
                            ib::ColorFilled,
                            ibpro::FontIcon { icon },
                            ib::Clickable { [this, index]() {
                                switch_page(index);
                            } },
                        };
                    },
                    Qt::AlignHCenter,
                },
                sg::SignalInjection { &IconButton::clicked }, // 将子按钮的 clicked 信号注入到 SelectGroup 中，以便在回调中正确识别当前选中项
            },
            lnpro::Stretch { 1 },
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
    const auto scheme = m_theme_manager.color_scheme();
    return new PlayerBar { m_theme_manager, this };
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
}

void TopWindow::setup_player() {
    m_player->setAudioOutput(m_audio_output);
    m_audio_output->setVolume(0.6);
}
