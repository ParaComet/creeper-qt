#pragma once

#include <QAudioOutput>
#include <QListWidget>
#include <QMediaPlayer>

#include <creeper-qt/widget/buttons/filled-button.hh>
#include <creeper-qt/widget/main-window.hh>
#include <creeper-qt/widget/sliders.hh>
#include <creeper-qt/widget/text.hh>

class QMediaDevices;

class PlayerWindow final : public creeper::MainWindow {
public:
    PlayerWindow();

private:
    void setup_connections();

    void open_files();
    void play_track(int index);
    void toggle_play_pause();
    void play_next();
    void play_previous();

    void refresh_track_list();
    void refresh_time_label();
    void refresh_play_button();
    void refresh_audio_output_state();
    void bind_preferred_audio_device();

    static QString format_time(qint64 ms);

private:
    creeper::ThemeManager m_theme_manager;

    QMediaPlayer* m_player;
    QAudioOutput* m_audio_output;
    QMediaDevices* m_media_devices;

    QListWidget* m_track_list;
    creeper::Text* m_title_text;
    creeper::Text* m_subtitle_text;
    creeper::Text* m_time_text;
    creeper::Slider* m_progress_slider;
    creeper::Slider* m_volume_slider;
    creeper::FilledButton* m_play_pause_button;

    QStringList m_playlist;
    int m_current_index;
    bool m_has_audio_output_device;

    bool m_syncing_progress;
};
