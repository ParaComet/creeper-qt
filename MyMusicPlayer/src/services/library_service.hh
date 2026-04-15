#pragma once

#include "models/library_models.hh"

#include <QSqlDatabase>
#include <QObject>
#include <QDateTime>

#include <vector>

class LibraryService : public QObject {
    Q_OBJECT

public:
    explicit LibraryService(QObject* parent = nullptr);

    const std::vector<mymusic::model::SongInfo>& songs() const noexcept;
    const std::vector<mymusic::model::ArtistInfo>& artists() const noexcept;
    const std::vector<mymusic::model::AlbumInfo>& albums() const noexcept;
    const std::vector<mymusic::model::PlaylistInfo>& playlists() const noexcept;
    const std::vector<mymusic::model::ScanRootInfo>& scan_roots() const noexcept;
    std::vector<mymusic::model::SongInfo> songs_for_source(const QString& source_id) const;
    const QString& last_scan_message() const noexcept;
    const QDateTime& last_scan_at() const noexcept;
    bool is_scanning() const noexcept;
    const mymusic::model::PlaylistInfo* find_playlist(const QString& playlist_id) const noexcept;
    bool source_supports_entity_favorite(const QString& source_id) const noexcept;
    bool source_entity_liked(const QString& source_id) const noexcept;
    bool set_source_entity_liked(const QString& source_id, bool liked);
    QString first_source_directory(const QString& source_id) const;
    bool set_song_liked(const QString& song_id, bool liked);

    void replace_library(std::vector<mymusic::model::SongInfo> songs,
        std::vector<mymusic::model::ArtistInfo> artists,
        std::vector<mymusic::model::AlbumInfo> albums,
        std::vector<mymusic::model::PlaylistInfo> playlists);

    void load_demo_library();
    bool add_scan_root(const QString& path);
    void scan_library();

signals:
    void library_changed();
    void scan_roots_changed();
    void scan_status_changed();
    void scan_started();
    void scan_finished();

private:
    bool open_database();
    void initialize_database();
    bool load_from_storage();
    void save_to_storage();

    std::vector<mymusic::model::SongInfo> m_songs;
    std::vector<mymusic::model::ArtistInfo> m_artists;
    std::vector<mymusic::model::AlbumInfo> m_albums;
    std::vector<mymusic::model::PlaylistInfo> m_playlists;
    std::vector<mymusic::model::ScanRootInfo> m_scan_roots;
    QString m_last_scan_message;
    QDateTime m_last_scan_at;
    bool m_is_scanning = false;
    QString m_database_path;
    QSqlDatabase m_database;
};
