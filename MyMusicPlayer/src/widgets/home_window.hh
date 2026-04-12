#pragma once

#include "services/library_service.hh"
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
#include <qmediaplayer.h>
#include <unordered_map>

using namespace creeper;

struct DisplayWidget {
    QWidget* widget;
    std::string widget_name;
    std::string description;
    int index;
};

struct TopWindow : public creeper::MainWindow {

public:
    TopWindow();
  
private:
    ThemeManager m_theme_manager;
    LibraryService m_library;

    QMediaPlayer *m_player;
    QAudioOutput *m_audio_output;

    NavHost *m_pages;
    std::unordered_map<const QWidget*, DisplayWidget> m_widget_map; 

    auto build_siderbar() -> QWidget*;
    auto build_content_pages() -> creeper::Widget*;
    auto build_player_bar() -> creeper::Widget*;
    auto build_central_widget() -> creeper::Widget*;

    void switch_page(int index);
    void setup_player();
};

struct NavComponetState {
    ThemeManager& manager;

    
};
