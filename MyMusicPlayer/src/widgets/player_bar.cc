#include "player_bar.hh"

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/material-icon.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "style.hh"
#include "creeper-qt/utility/wrapper/common.hh"

#include <QFileInfo>
#include <QIcon>
#include <QVBoxLayout>
#include <qwidget.h>

using namespace creeper;
namespace lnpro = linear::pro;
namespace ibpro = icon_button::pro;
namespace sty = mymusic::style;

PlayerBar::PlayerBar(ThemeManager& manager, QWidget* parent)
    : m_theme_manager(manager)
    , m_cover_button(nullptr)
    , m_title_text(nullptr)
    , m_artist_text(nullptr)
    , m_album_text(nullptr)
    , m_play_pause_button(nullptr)
    , m_previous_button(nullptr)
    , m_next_button(nullptr)
    , m_order_button(nullptr)
    , m_favorite_button(nullptr)
    , m_add_button(nullptr)
    , m_progress_slider(nullptr)
    , m_volume_slider(nullptr)
    , m_playing(false) {
    if (parent != nullptr) {
        setParent(parent);
    }

    auto* root_layout = new QVBoxLayout {};
    root_layout->setContentsMargins(0, 0, 0, 0);
    root_layout->addWidget(build_content());
    setLayout(root_layout);

    apply_theme();
    set_track_info("当前未播放", "", "");
    set_progress(0.0);
    set_volume(0.6);
    set_cover();
    update_play_pause_button();
}

auto PlayerBar::build_content() -> creeper::Widget* {
    const auto scheme = m_theme_manager.color_scheme();
    const auto button_features = std::tuple {
        ibpro::ThemeManager { m_theme_manager },
        ibpro::Font { material::kRoundSmallFont },
        ibpro::FixedSize { 40, 40 },
        ibpro::ShapeRound,
        ibpro::ColorStandard,
    };

    auto* state_panel = new creeper::Widget {         
        widget::pro::Layout<Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 6 },
            lnpro::Item<Text> {
                widget::pro::Bind { m_title_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "当前未播放" },
                widget::pro::Apply { [](Text& self) {
                    auto font = self.font();
                    font.setBold(true);
                    font.setPointSize(font.pointSize() + 1);
                    self.setFont(font);
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
            lnpro::Item<Text> {
                widget::pro::Bind { m_artist_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { },
                text::pro::Color { m_theme_manager.color_scheme().on_surface_variant },
                widget::pro::Apply { [](Text& self) {
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
            lnpro::Item<Text> {
                widget::pro::Bind { m_album_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { },
                text::pro::Color { m_theme_manager.color_scheme().primary },
                widget::pro::Apply { [](Text& self) {
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
        },
        
    };

    auto* left_panel = new creeper::Widget {
        widget::pro::Layout<Row> {
        
            lnpro::Margin { 0 },
            lnpro::Spacing { 12 },
            lnpro::Item<FilledButton> {
                widget::pro::Bind { m_cover_button },
                filled_button::pro::ThemeManager { m_theme_manager },
                filled_button::pro::Radius { 16 },
                button::pro::Text { "封面" },
                filled_button::pro::FixedSize { 72, 72 },
                button::pro::Clickable { [this] {
                    emit cover_clicked();
                    emit control_signal("cover");
                } },
            },
            lnpro::Item<Widget> { lnpro::Item<Widget>::LayoutMethod { 1 }, state_panel },
        },
        widget::pro::FixedWidth { 180 },
        widget::pro::SizePolicy { QSizePolicy::Preferred, QSizePolicy::Preferred },
    };

    auto* center_panel = new creeper::Widget {
        widget::pro::Layout<Row> {
            lnpro::Margin { 0 },
            lnpro::SpacingItem { 150 },
            lnpro::Spacing { 10 },
            lnpro::Stretch { 1 },
          
            lnpro::Item<IconButton> {
                button_features,
                ibpro::Font { material::kRoundLargeFont },
                ibpro::FontIcon { material::icon::kSkipPrev},
                widget::pro::Bind { m_previous_button },
                ibpro::Clickable { [this]() {
                    emit previous_clicked();
                    emit control_signal("previous");
                } },
            },
            lnpro::Item<IconButton> {
                button_features,
                ibpro::Font { material::kRoundExtraLargeFont },
                ibpro::FontIcon { material::icon::kPlayArrow },
                widget::pro::Bind { m_play_pause_button },
                ibpro::Clickable { [this]() {
                    on_play_pause_clicked();
                } },
            },
            lnpro::Item<IconButton> {
                button_features,
                ibpro::Font { material::kRoundLargeFont },
                ibpro::FontIcon { material::icon::kSkipNext },
                widget::pro::Bind { m_next_button },
                ibpro::Clickable { [this]() {
                    emit next_clicked();
                    emit control_signal("next");
                } },
            },
            // lnpro::Item<FilledButton> {
            //     widget::pro::Bind { m_next_button },
            //     filled_button::pro::ThemeManager { m_theme_manager },
            //     filled_button::pro::Text { "下一首" },
            //     filled_button::pro::MinimumWidth { 84 },
            //     filled_button::pro::FixedHeight { 36 },
            //     filled_button::pro::Clickable { [this] {
            //         emit next_clicked();
            //         emit control_signal("next");
            //     } },
            // },
            lnpro::Stretch { 1 },
        },
        widget::pro::MinimumWidth { 340 },
        widget::pro::FixedHeight { 44 },
        widget::pro::SizePolicy { QSizePolicy::Fixed, QSizePolicy::Fixed },
    };


    auto* volume_panel = new creeper::Widget {
        widget::pro::Layout<Row> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 8 },
            lnpro::Item<IconButton> {
                button_features,
                ibpro::FontIcon { material::icon::kVolumeUp },
                widget::pro::Apply { [](Text& self) {
                    self.setStyleSheet("QLabel { background: transparent; border: none; }");
                } },
            },
            lnpro::Item<Slider> {
                widget::pro::Bind { m_volume_slider },
                slider::pro::ThemeManager { m_theme_manager },
                slider::pro::Measurements { Slider::Measurements::Xs() },
                widget::pro::FixedWidth { 120 },
                widget::pro::FixedHeight { 44 },
                slider::pro::Progress { 0.6 },
                slider::pro::OnValueChange { [this](double volume) {
                    emit volume_changed(volume);
                    emit control_signal("volume");
                } },
            },
        },
    };

    auto* right_panel = new creeper::Widget {
        widget::pro::Layout<Row> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 8 },
            lnpro::Item { volume_panel },
            lnpro::Item<IconButton> {
                widget::pro::Bind { m_order_button },
                button_features,
                ibpro::FontIcon { QStringLiteral("reorder") },
                ibpro::Clickable { [this] {
                    emit order_clicked();
                    emit control_signal("order");
                } },
            },
            lnpro::Item<IconButton> {
                widget::pro::Bind { m_favorite_button },
                button_features,
                ibpro::ColorFilled,
                ibpro::TypesToggleUnselected,
                ibpro::FontIcon { material::icon::kFavorite },
                ibpro::Clickable { [this](IconButton& self) {
                    emit favorite_toggled(self.selected());
                    emit control_signal(self.selected() ? "favorite_on" : "favorite_off");
                } },
            },
            lnpro::Item<IconButton> {
                widget::pro::Bind { m_add_button },
                button_features,
                ibpro::FontIcon { material::icon::kAdd },
                ibpro::Clickable { [this] {
                    emit add_to_playlist_clicked();
                    emit control_signal("add_to_playlist");
                } },
            },
        },
        widget::pro::SizePolicy { QSizePolicy::Preferred, QSizePolicy::Preferred },
    };

    auto* lower_bar = new creeper::Widget {
        widget::pro::Layout<Row> {
            lnpro::Margin { 8 },
            lnpro::Spacing { 16 },
            lnpro::Stretch { 1 },
            lnpro::Item { center_panel },
            lnpro::Stretch { 1 },
            lnpro::Item { right_panel },
        },
        widget::pro::SizePolicy { QSizePolicy::Preferred, QSizePolicy::Preferred },
    };

    auto* ProgressBar = new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::ContentsMargin { {7, 0, 7, 0} },
            lnpro::Spacing { 0 },
            lnpro::Item<Slider> {
                widget::pro::Bind { m_progress_slider },
                slider::pro::ThemeManager { m_theme_manager },
                slider::pro::Measurements { Slider::Measurements::Xs() },
                widget::pro::FixedHeight { 36 },
                slider::pro::Progress { 0.0 },
                slider::pro::OnValueChangeFinished { [this](double progress) {
                    emit seek_requested(progress);
                    emit control_signal("seek");
                } },
            },
            lnpro::Item { lower_bar },
        },
        widget::pro::SizePolicy { QSizePolicy::Preferred, QSizePolicy::Preferred },
    };
    return new creeper::Widget {
        widget::pro::FixedHeight { 122 },
        widget::pro::Apply { [this](QWidget& self) {
            self.setObjectName("elevated_panel");
            self.setStyleSheet(sty::elevated_panel_style(m_theme_manager.color_scheme()));
        } },
        widget::pro::Layout<Row> {
            lnpro::ContentsMargin { {16, 10, 16, 12} },
            lnpro::Spacing { 10 },
            lnpro::Item {  left_panel },
            lnpro::Item<Widget> { { 1, Qt::AlignVCenter }, ProgressBar },
        },
        widget::pro::SizePolicy { QSizePolicy::Preferred, QSizePolicy::Expanding },

    };
}

void PlayerBar::set_track_info(const QString& title, const QString& artist, const QString& album) {
    if (m_title_text != nullptr) {
        m_title_text->setText(title);
    }
    if (m_artist_text != nullptr) {
        m_artist_text->setText(artist);
    }
    if (m_album_text != nullptr) {
        m_album_text->setText(album);
    }
}

void PlayerBar::set_cover(const QString& label, const QString& cover_path) {
    if (m_cover_button != nullptr) {
        if (!cover_path.isEmpty() && QFileInfo::exists(cover_path)) {
            m_cover_button->setText(QString {});
            m_cover_button->setIcon(QIcon(cover_path));
            m_cover_button->setIconSize(QSize { 72, 72 });
        } else {
            m_cover_button->setIcon(QIcon {});
            m_cover_button->setText(label);
        }
        m_cover_button->update();
    }
}

void PlayerBar::set_progress(double progress) {
    if (m_progress_slider != nullptr) {
        m_progress_slider->set_progress(progress);
    }
}

void PlayerBar::set_volume(double volume) {
    if (m_volume_slider != nullptr) {
        m_volume_slider->set_progress(volume);
    }
}

void PlayerBar::set_playing(bool playing) {
    m_playing = playing;
    update_play_pause_button();
}

void PlayerBar::set_favorite(bool favorite) {
    if (m_favorite_button != nullptr) {
        m_favorite_button->set_selected(favorite);
    }
}

void PlayerBar::set_order_icon(const QString& icon_name) {
    if (m_order_button != nullptr) {
        m_order_button->set_icon(icon_name);
    }
}

void PlayerBar::on_play_pause_clicked() {
    m_playing = !m_playing;
    update_play_pause_button();
    emit play_pause_clicked();
    emit control_signal(m_playing ? "play" : "pause");
}

void PlayerBar::apply_theme() {
    if (auto* content = findChild<QWidget*>("elevated_panel"); content != nullptr) {
        content->setStyleSheet(sty::elevated_panel_style(m_theme_manager.color_scheme()));
    }
}

void PlayerBar::update_play_pause_button() {
    if (m_play_pause_button != nullptr) {
        m_play_pause_button->set_icon(m_playing ? material::icon::kPause : material::icon::kPlayArrow);
    }
}
