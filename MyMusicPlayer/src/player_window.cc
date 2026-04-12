#include "player_window.hh"

#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QShortcut>
#include <QTimer>
#include <QUrl>

#include <algorithm>

#include <creeper-qt/layout/linear.hh>
#include <creeper-qt/utility/theme/preset/gloden-harvest.hh>
#include <creeper-qt/utility/wrapper/widget.hh>
#include <creeper-qt/widget/widget.hh>

using namespace creeper;

namespace lnpro = linear::pro;
namespace mwpro = main_window::pro;

PlayerWindow::PlayerWindow()
    : m_theme_manager(kGoldenHarvestThemePack)
    , m_player(new QMediaPlayer(this))
    , m_audio_output(new QAudioOutput(this))
    , m_media_devices(new QMediaDevices(this))
    , m_track_list(nullptr)
    , m_title_text(nullptr)
    , m_subtitle_text(nullptr)
    , m_time_text(nullptr)
    , m_progress_slider(nullptr)
    , m_volume_slider(nullptr)
    , m_play_pause_button(nullptr)
    , m_current_index(-1)
    , m_has_audio_output_device(false)
    , m_syncing_progress(false) {
    m_player->setAudioOutput(m_audio_output);
    m_audio_output->setVolume(0.6);
    m_track_list = new QListWidget;
    m_track_list->setMinimumWidth(300);

    auto title_font = QFont {};
    title_font.setPointSize(16);
    title_font.setBold(true);

    apply(widget::pro::Apply { [this](QWidget& self) {
        self.setWindowTitle("MyMusicPlayer (creeper-qt demo)");
        self.resize(920, 620);
    } });

    auto* controls_panel = new creeper::Widget {
        widget::pro::Apply { [](QWidget& self) {
            self.setStyleSheet("QWidget { border: 1px solid #c7c7c7; border-radius: 10px; }");
        } },
        widget::pro::Layout<creeper::Col> {
            lnpro::ContentsMargin { {10, 10, 10, 10} },
            lnpro::Spacing { 8 },

            lnpro::Item<Text> {
                text::pro::Text { "播放控制" },
                widget::pro::Apply { [](Text& self) {
                    auto font = self.font();
                    font.setPointSize(14);
                    font.setBold(true);
                    self.setFont(font);
                } },
            },

            lnpro::Item<creeper::Row> {
                lnpro::Margin { 0 },
                lnpro::Spacing { 8 },

                lnpro::Item<FilledButton> {
                    filled_button::pro::ThemeManager { m_theme_manager },
                    button::pro::Text { "打开文件" },
                    filled_button::pro::MinimumWidth { 110 },
                    filled_button::pro::FixedHeight { 40 },
                    button::pro::Clickable { [this] { open_files(); } },
                },
                lnpro::Item<FilledButton> {
                    filled_button::pro::ThemeManager { m_theme_manager },
                    button::pro::Text { "上一首" },
                    button::pro::Background { QColor("#546e7a") },
                    button::pro::TextColor { QColor("#ffffff") },
                    filled_button::pro::MinimumWidth { 96 },
                    filled_button::pro::FixedHeight { 40 },
                    button::pro::Clickable { [this] { play_previous(); } },
                },
                lnpro::Item<FilledButton> {
                    widget::pro::Bind { m_play_pause_button },
                    filled_button::pro::ThemeManager { m_theme_manager },
                    button::pro::Text { "播放" },
                    button::pro::Background { QColor("#2e7d32") },
                    button::pro::TextColor { QColor("#ffffff") },
                    filled_button::pro::MinimumWidth { 156 },
                    filled_button::pro::FixedHeight { 48 },
                    button::pro::Clickable { [this] { toggle_play_pause(); } },
                },
                lnpro::Item<FilledButton> {
                    filled_button::pro::ThemeManager { m_theme_manager },
                    button::pro::Text { "下一首" },
                    button::pro::Background { QColor("#546e7a") },
                    button::pro::TextColor { QColor("#ffffff") },
                    filled_button::pro::MinimumWidth { 96 },
                    filled_button::pro::FixedHeight { 40 },
                    button::pro::Clickable { [this] { play_next(); } },
                },
                lnpro::Stretch { 1 },
            },
        },
    };

    auto* volume_row = new creeper::Row {
        lnpro::Margin { 0 },
        lnpro::Spacing { 8 },
        lnpro::Item<Text> {
            text::pro::ThemeManager { m_theme_manager },
            text::pro::Text { "音量" },
        },
        lnpro::Item<Slider> {
            lnpro::Item<Slider>::LayoutMethod { 1 },
            widget::pro::Bind { m_volume_slider },
            slider::pro::ThemeManager { m_theme_manager },
            slider::pro::Measurements { Slider::Measurements::Xs() },
            widget::pro::FixedHeight { 44 },
            slider::pro::Progress { m_audio_output->volume() },
            slider::pro::OnValueChange { [this](double progress) {
                m_audio_output->setVolume(std::clamp(progress, 0.0, 1.0));
            } },
        },
    };

    auto* right_panel = new creeper::Widget {
        widget::pro::Layout<creeper::Col> {
            lnpro::Margin { 0 },
            lnpro::Spacing { 12 },

            lnpro::Item<Text> {
                widget::pro::Bind { m_title_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "请选择音频文件" },
                widget::pro::Font { title_font },
            },
            lnpro::Item<Text> {
                widget::pro::Bind { m_subtitle_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "支持 mp3 / wav / flac / m4a / ogg" },
                text::pro::WordWrap { true },
            },
            lnpro::SpacingItem { 8 },
            lnpro::Item<Slider> {
                widget::pro::Bind { m_progress_slider },
                slider::pro::ThemeManager { m_theme_manager },
                slider::pro::Measurements { Slider::Measurements::S() },
                widget::pro::FixedHeight { 44 },
                slider::pro::Progress { 0.0 },
                slider::pro::OnValueChangeFinished { [this](double progress) {
                    if (m_syncing_progress || m_player->duration() <= 0) {
                        return;
                    }
                    const auto new_position =
                        static_cast<qint64>(progress * m_player->duration());
                    m_player->setPosition(new_position);
                    refresh_time_label();
                } },
            },
            lnpro::Item<Text> {
                widget::pro::Bind { m_time_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "00:00 / 00:00" },
            },
            lnpro::Item { controls_panel },
            lnpro::Item { volume_row },
            lnpro::Stretch { 1 },
        },
    };

    auto central = mwpro::Central<creeper::Widget> {
        widget::pro::Layout<creeper::Row> {
            lnpro::ContentsMargin { {16, 16, 16, 16} },
            lnpro::Spacing { 16 },
            lnpro::Item { m_track_list },
            lnpro::Item { right_panel },
        },
    };
    apply(central);

    setup_connections();

    m_theme_manager.apply_theme();
    refresh_time_label();
    refresh_play_button();
    refresh_audio_output_state();

    QTimer::singleShot(0, this, [this] { refresh_audio_output_state(); });
    QTimer::singleShot(0, this, &PlayerWindow::open_files);
}

void PlayerWindow::setup_connections() {
    auto* open_shortcut = new QShortcut(QKeySequence::Open, this);
    connect(open_shortcut, &QShortcut::activated, this, &PlayerWindow::open_files);

    connect(m_track_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr) {
            return;
        }
        play_track(m_track_list->row(item));
    });

    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64) {
        if (m_syncing_progress || m_player->duration() <= 0) {
            refresh_time_label();
            return;
        }

        m_syncing_progress = true;
        m_progress_slider->set_progress(
            static_cast<double>(m_player->position()) / static_cast<double>(m_player->duration()));
        m_syncing_progress = false;

        refresh_time_label();
    });

    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64) { refresh_time_label(); });

    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
        [this](QMediaPlayer::PlaybackState) { refresh_play_button(); });

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            play_next();
        }
    });

    connect(m_player, &QMediaPlayer::errorOccurred, this,
        [this](QMediaPlayer::Error, const QString& error_string) {
            if (!error_string.isEmpty()) {
                m_subtitle_text->setText(QString("播放错误: %1").arg(error_string));
            }
        });

    connect(m_media_devices, &QMediaDevices::audioOutputsChanged, this,
        [this] { refresh_audio_output_state(); });
}

void PlayerWindow::open_files() {
    const QStringList files = QFileDialog::getOpenFileNames(this,
        tr("选择音频文件"),
        QString(),
        tr("Audio Files (*.mp3 *.wav *.flac *.m4a *.ogg *.aac);;All Files (*)"));

    if (files.isEmpty()) {
        return;
    }

    for (const QString& file : files) {
        if (!m_playlist.contains(file)) {
            m_playlist.push_back(file);
        }
    }

    refresh_track_list();

    if (m_current_index < 0) {
        play_track(0);
    }
}

void PlayerWindow::play_track(int index) {
    if (index < 0 || index >= m_playlist.size()) {
        return;
    }

    m_current_index = index;

    const QString& path = m_playlist.at(index);
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();

    m_title_text->setText(QFileInfo(path).fileName());
    m_subtitle_text->setText(path);

    m_track_list->setCurrentRow(index);
    refresh_play_button();
}

void PlayerWindow::toggle_play_pause() {
    if (m_playlist.isEmpty()) {
        open_files();
        return;
    }

    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        if (m_current_index < 0) {
            play_track(0);
            return;
        }
        m_player->play();
    }
    refresh_play_button();
}

void PlayerWindow::play_next() {
    if (m_playlist.isEmpty()) {
        return;
    }

    const int next = (m_current_index + 1) % m_playlist.size();
    play_track(next);
}

void PlayerWindow::play_previous() {
    if (m_playlist.isEmpty()) {
        return;
    }

    int previous = m_current_index - 1;
    if (previous < 0) {
        previous = m_playlist.size() - 1;
    }
    play_track(previous);
}

void PlayerWindow::refresh_track_list() {
    m_track_list->clear();
    for (const QString& file : m_playlist) {
        m_track_list->addItem(QFileInfo(file).fileName());
    }

    if (m_current_index >= 0 && m_current_index < m_track_list->count()) {
        m_track_list->setCurrentRow(m_current_index);
    }
}

void PlayerWindow::refresh_time_label() {
    const QString current = format_time(m_player->position());
    const QString total   = format_time(m_player->duration());
    m_time_text->setText(QString("%1 / %2").arg(current, total));
}

void PlayerWindow::refresh_play_button() {
    const bool playing = m_player->playbackState() == QMediaPlayer::PlayingState;
    m_play_pause_button->setText(playing ? "暂停" : "播放");
}

void PlayerWindow::refresh_audio_output_state() {
    const auto outputs        = m_media_devices->audioOutputs();
    const auto default_output = m_media_devices->defaultAudioOutput();
    m_has_audio_output_device = !outputs.isEmpty() || !default_output.isNull();

    bind_preferred_audio_device();

    if (!m_has_audio_output_device) {
        m_subtitle_text->setText("未检测到音频输出设备（检测可能延迟），可先尝试直接播放");
        return;
    }

    if (m_current_index < 0) {
        m_subtitle_text->setText("支持 mp3 / wav / flac / m4a / ogg");
    }
}

void PlayerWindow::bind_preferred_audio_device() {
    const auto outputs        = m_media_devices->audioOutputs();
    const auto default_output = m_media_devices->defaultAudioOutput();

    if (!default_output.isNull()) {
        m_audio_output->setDevice(default_output);
        return;
    }

    if (!outputs.isEmpty()) {
        m_audio_output->setDevice(outputs.first());
    }
}

QString PlayerWindow::format_time(qint64 ms) {
    if (ms <= 0) {
        return QStringLiteral("00:00");
    }

    const qint64 total_seconds = ms / 1000;
    const qint64 minutes       = total_seconds / 60;
    const qint64 seconds       = total_seconds % 60;

    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}
