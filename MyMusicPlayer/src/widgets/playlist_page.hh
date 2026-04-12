#pragma once

#include "services/library_service.hh"
#include "models/library_models.hh"
#include "mycomponet/PlaylistItem.hh"
#include "creeper-qt/layout/stacked.hh"
#include "creeper-qt/widget/widget.hh"
#include <creeper-qt/utility/wrapper/property.hh>
#include <creeper-qt/utility/wrapper/widget.hh>
#include <creeper-qt/utility/theme/theme.hh>
#include <vector>

class QVBoxLayout;

using namespace creeper;

struct PlaylistPage : public creeper::Widget {
    explicit PlaylistPage(ThemeManager& manager, LibraryService& library);
private:
    ThemeManager& m_theme_manager;
    LibraryService& m_library;
    NavHost* m_songlist_pages = nullptr;
    NavHost* m_header_pages = nullptr;
    QVBoxLayout* m_root_layout = nullptr;
    QWidget* m_content_widget = nullptr;
    std::vector<mycomponent::PlaylistItem*> m_playlist_items;
    auto build_content() -> creeper::Widget*;
    auto build_sidebar() -> QWidget*;
    auto build_header(const QString& title, const QString& description, const QString& meta,
        const QString& badge, const QColor& badge_color) -> QWidget*;
    auto build_song_list(const mymusic::model::PlaylistInfo& playlist) -> QWidget*;
    auto build_sidebar_playlist_item(int index, const QString& title, const QString& meta,
        const QString& badge, bool selected)
        -> mycomponent::PlaylistItem*;
    auto build_song_row(const QString& index, const QString& title, const QString& artist,
        const QString& album, const QString& duration, const QString& badge)
        -> creeper::Widget*;
    void reload_content();
    void switch_playlist(int index);
    void setup_connections();
};
