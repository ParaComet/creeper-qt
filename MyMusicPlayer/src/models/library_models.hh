#pragma once

#include <QDateTime>
#include <QColor>
#include <QString>

#include <vector>

namespace mymusic::model {

// 歌曲信息
struct SongInfo {
    QString id;
    QString title;
    QString artist;
    QString album;
    QString file_path;
    QString duration_text;
    QString cover_badge;
    QString cover_path;
    int track_number = 0;
    bool liked       = false;
};

// 歌单信息
struct PlaylistInfo {
    QString id;
    QString title;
    QString description;
    QString meta;
    QString badge;
    QString sidebar_meta;
    QString sidebar_badge;
    QColor accent_color;
    bool system_generated = false;
    std::vector<SongInfo> songs;
};

struct ArtistInfo {
    QString id;
    QString name;
    QString subtitle;
    QString cover_badge;
    QString cover_path;
    int song_count  = 0;
    int album_count = 0;
    bool liked      = false;
};

struct AlbumInfo {
    QString id;
    QString title;
    QString artist;
    QString subtitle;
    QString cover_badge;
    QString cover_path;
    int track_count = 0;
    bool liked      = false;
};

struct ScanRootInfo {
    QString path;
    QDateTime last_scan_at;
    int discovered_count = 0;
};

}
