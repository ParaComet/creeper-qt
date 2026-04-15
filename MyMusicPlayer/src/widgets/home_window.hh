#pragma once

#include "services/library_service.hh"
#include "models/library_models.hh"
#include "creeper-qt/layout/stacked.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/widget/cards/filled-card.hh"
#include <QMediaPlayer>
#include <QAudioOutput>

#include <creeper-qt/layout/mixer.hh>
#include <creeper-qt/widget/main-window.hh>
#include <creeper-qt/widget/widget.hh>
#include <creeper-qt/widget/sliders.hh>
#include <creeper-qt/widget/text.hh>
#include <creeper-qt/widget/buttons/filled-button.hh>
#include <creeper-qt/widget/buttons/icon-button.hh>
#include <creeper-qt/utility/material-icon.hh>
#include <creeper-qt/layout/linear.hh>
#include <QMouseEvent>
#include <QEvent>
#include <qmediaplayer.h>
#include <QString>
#include <unordered_map>
#include <vector>

using namespace creeper;
class PlayerBar;

struct DisplayWidget {
    QWidget* widget;
    std::string widget_name;
    std::string description;
    int index;
};

struct TopWindow : public creeper::MainWindow {

public:
    TopWindow();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
  
private:
    enum class PlaybackOrder {
        Sequential,
        RepeatAll,
        RepeatOne,
        Shuffle,
    };

    ThemeManager m_theme_manager;
    LibraryService m_library;

    QMediaPlayer *m_player;
    QAudioOutput *m_audio_output;

    NavHost *m_pages;
    std::unordered_map<const QWidget*, DisplayWidget> m_widget_map; 
    PlayerBar* m_player_bar = nullptr;
    std::vector<IconButton*> m_nav_buttons;
    std::vector<mymusic::model::SongInfo> m_playback_queue;
    int m_current_queue_index = -1;
    PlaybackOrder m_playback_order = PlaybackOrder::Sequential;

    auto build_siderbar() -> QWidget*;
    auto build_content_pages() -> creeper::Widget*;
    auto build_player_bar() -> creeper::Widget*;
    auto build_central_widget() -> creeper::Widget*;

    void switch_page(int index);
    void setup_player();
    auto playback_queue_for_source(const QString& source_id) const
        -> std::vector<mymusic::model::SongInfo>;
    void enqueue_source(const QString& source_id);
    void play_song_from_source(const QString& source_id, const QString& song_id);
    void play_source_from_beginning(const QString& source_id);
    void play_song_at_index(int index);
    void play_next_track();
    void play_previous_track();
    void load_default_track();
    void cycle_playback_order();
    void update_player_bar_order();
    auto resize_edges_for_pos(const QPoint& pos) const -> Qt::Edges;
    void update_window_cursor(const QPoint& pos);
};

struct NavComponetState {
    ThemeManager& manager;

    
};
