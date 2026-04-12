#pragma once

#include "models/library_models.hh"
#include "services/library_service.hh"

#include "creeper-qt/layout/stacked.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/widget/widget.hh"

class QVBoxLayout;

struct AllMusicPage : public creeper::Widget {
    explicit AllMusicPage(creeper::ThemeManager& manager, LibraryService& library);

private:
    creeper::ThemeManager& m_theme_manager;
    LibraryService& m_library;
    creeper::NavHost* m_views = nullptr;
    QVBoxLayout* m_root_layout = nullptr;
    QWidget* m_content_widget = nullptr;

    auto build_content() -> creeper::Widget*;
    auto build_header_card() -> QWidget*;
    auto build_view_switch() -> QWidget*;
    auto build_songs_view() -> QWidget*;
    auto build_artists_view() -> QWidget*;
    auto build_albums_view() -> QWidget*;
    auto build_song_row(const mymusic::model::SongInfo& song) -> QWidget*;
    auto build_artist_card(const mymusic::model::ArtistInfo& artist) -> QWidget*;
    auto build_album_card(const mymusic::model::AlbumInfo& album) -> QWidget*;

    void reload_content();
    void switch_view(int index);
};
