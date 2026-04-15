#pragma once

#include "models/library_models.hh"
#include "services/library_service.hh"
#include "mycomponet/PlaylistItem.hh"
#include "mycomponet/SongItem.hh"

#include "creeper-qt/layout/stacked.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/widget/widget.hh"

class QVBoxLayout;

struct AllMusicPage : public creeper::Widget {
    Q_OBJECT
public:
    explicit AllMusicPage(creeper::ThemeManager& manager, LibraryService& library);

signals:
    void song_activated(const QString& source_id, const QString& song_id);
    void play_all_requested(const QString& source_id);
    void enqueue_requested(const QString& source_id);
    void entity_favorite_requested(const QString& source_id, bool liked);
    void reveal_requested(const QString& source_id);

private:
    enum class DetailMode {
        None,
        Artist,
        Album,
    };

    creeper::ThemeManager& m_theme_manager;
    LibraryService& m_library;
    creeper::NavHost* m_views = nullptr;
    QVBoxLayout* m_root_layout = nullptr;
    QWidget* m_content_widget = nullptr;
    int m_current_view_index = 0;
    int m_detail_origin_view_index = 0;
    DetailMode m_detail_mode = DetailMode::None;
    QString m_detail_source_id;
    mymusic::model::ArtistInfo m_detail_artist;
    mymusic::model::AlbumInfo m_detail_album;

    auto build_content() -> creeper::Widget*;
    auto build_header_card() -> QWidget*;
    auto build_view_switch() -> QWidget*;
    auto build_detail_header() -> QWidget*; // 二级界面头部
    auto build_detail_song_list() -> QWidget*; // 二级界面歌曲列表
    auto build_songs_view() -> QWidget*;
    auto build_artists_view() -> QWidget*;
    auto build_albums_view() -> QWidget*;
    auto build_song_row(const QString& source_id, const mymusic::model::SongInfo& song)
        -> mycomponent::SongItem*;
    auto build_artist_card(const mymusic::model::ArtistInfo& artist) -> QWidget*;
    auto build_album_card(const mymusic::model::AlbumInfo& album) -> QWidget*;

    void reload_content();
    void switch_view(int index);
    void open_artist_detail(const mymusic::model::ArtistInfo& artist);
    void open_album_detail(const mymusic::model::AlbumInfo& album);
    void leave_detail();
};
