#include "library_service.hh"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>

#include <algorithm>

using namespace mymusic::model;

namespace {

constexpr auto kConnectionName = "mymusic_library_service";

const QColor kGeneratedPlaylistColors[] {
    QColor(181, 233, 229),
    QColor(189, 226, 239),
    QColor(214, 234, 226),
    QColor(220, 226, 229),
    QColor(205, 233, 214),
    QColor(202, 226, 244),
};

auto make_cover_badge(const QString& text) -> QString {
    QString badge;
    for (const auto ch : text) {
        if (ch.isLetterOrNumber()) {
            badge.append(ch.toUpper());
        }
        if (badge.size() >= 2) {
            break;
        }
    }
    return badge.isEmpty() ? QStringLiteral("NA") : badge;
}

auto parse_song_from_file(const QString& path, int track_number) -> SongInfo {
    const QFileInfo info { path };
    const auto parent_dir = info.dir().dirName();
    const auto stem       = info.completeBaseName();

    QString artist = QStringLiteral("Unknown Artist");
    QString title  = stem;

    if (const auto separator = stem.indexOf(QStringLiteral(" - ")); separator > 0) {
        artist = stem.first(separator).trimmed();
        title  = stem.mid(separator + 3).trimmed();
    }

    return SongInfo {
        .id            = info.absoluteFilePath(),
        .title         = title,
        .artist        = artist,
        .album         = parent_dir.isEmpty() ? QStringLiteral("Unknown Album") : parent_dir,
        .file_path     = info.absoluteFilePath(),
        .duration_text = QStringLiteral("--:--"),
        .cover_badge   = make_cover_badge(title),
        .track_number  = track_number,
    };
}

auto make_slug(const QString& text) -> QString {
    QString slug;
    slug.reserve(text.size());

    auto append_dash = [&slug] {
        if (!slug.isEmpty() && !slug.endsWith('-')) {
            slug.append('-');
        }
    };

    for (const auto ch : text) {
        if (ch.isLetterOrNumber()) {
            slug.append(ch.toLower());
        } else {
            append_dash();
        }
    }

    while (slug.endsWith('-')) {
        slug.chop(1);
    }
    return slug.isEmpty() ? QStringLiteral("playlist") : slug;
}

auto generated_playlist_color(int index) -> QColor {
    return kGeneratedPlaylistColors[index
        % static_cast<int>(std::size(kGeneratedPlaylistColors))];
}

auto collect_manual_playlists(const std::vector<PlaylistInfo>& playlists) -> std::vector<PlaylistInfo> {
    std::vector<PlaylistInfo> manual_playlists;
    for (const auto& playlist : playlists) {
        if (!playlist.system_generated) {
            manual_playlists.push_back(playlist);
        }
    }
    return manual_playlists;
}

auto build_generated_playlists(
    const std::vector<SongInfo>& songs, const std::vector<ArtistInfo>& artists,
    const std::vector<AlbumInfo>& albums, int scan_root_count)
    -> std::vector<PlaylistInfo> {
    std::vector<PlaylistInfo> playlists;
    if (songs.empty()) {
        return playlists;
    }

    playlists.push_back(PlaylistInfo {
        .id            = QStringLiteral("playlist-library-all"),
        .title         = QStringLiteral("本地全部音乐"),
        .description   = QStringLiteral("由扫描目录自动生成，包含当前本地资料库中的全部歌曲。"),
        .meta          = QStringLiteral("%1 songs · %2 个扫描目录").arg(songs.size()).arg(scan_root_count),
        .badge         = QStringLiteral("LOCAL"),
        .sidebar_meta  = QStringLiteral("%1 songs · 资料库").arg(songs.size()),
        .sidebar_badge = QStringLiteral("LO"),
        .accent_color  = generated_playlist_color(0),
        .system_generated = true,
        .songs         = songs,
    });

    std::vector<SongInfo> recent_songs;
    recent_songs.reserve(std::min<std::size_t>(songs.size(), 20));
    for (auto it = songs.rbegin(); it != songs.rend() && recent_songs.size() < 20; ++it) {
        recent_songs.push_back(*it);
    }
    std::reverse(recent_songs.begin(), recent_songs.end());

    playlists.push_back(PlaylistInfo {
        .id            = QStringLiteral("playlist-library-recent"),
        .title         = QStringLiteral("最近导入"),
        .description   = QStringLiteral("最近一次扫描中整理出来的曲目，适合先快速试听一轮。"),
        .meta          = QStringLiteral("%1 songs · 最近扫描").arg(recent_songs.size()),
        .badge         = QStringLiteral("RECENT"),
        .sidebar_meta  = QStringLiteral("%1 songs · 新导入").arg(recent_songs.size()),
        .sidebar_badge = QStringLiteral("RC"),
        .accent_color  = generated_playlist_color(1),
        .system_generated = true,
        .songs         = std::move(recent_songs),
    });

    QHash<QString, std::vector<SongInfo>> songs_by_album;
    QHash<QString, std::vector<SongInfo>> songs_by_artist;
    for (const auto& song : songs) {
        songs_by_album[song.album].push_back(song);
        songs_by_artist[song.artist].push_back(song);
    }

    std::vector<AlbumInfo> sorted_albums = albums;
    std::sort(sorted_albums.begin(), sorted_albums.end(),
        [](const auto& lhs, const auto& rhs) {
            if (lhs.track_count != rhs.track_count) {
                return lhs.track_count > rhs.track_count;
            }
            return lhs.title.localeAwareCompare(rhs.title) < 0;
        });

    int accent_index = 2;
    int added_albums = 0;
    for (const auto& album : sorted_albums) {
        const auto album_songs = songs_by_album.value(album.title);
        if (album_songs.size() < 2) {
            continue;
        }
        if (album.title == QStringLiteral("Unknown Album")) {
            continue;
        }

        playlists.push_back(PlaylistInfo {
            .id            = QStringLiteral("playlist-album-%1").arg(make_slug(album.title)),
            .title         = album.title,
            .description   = QStringLiteral("由专辑自动生成，便于从完整专辑视角继续整理本地音乐。"),
            .meta          = QStringLiteral("%1 songs · %2").arg(album_songs.size()).arg(album.artist),
            .badge         = album.cover_badge.isEmpty() ? make_cover_badge(album.title) : album.cover_badge,
            .sidebar_meta  = QStringLiteral("%1 songs · 专辑").arg(album_songs.size()),
            .sidebar_badge = QStringLiteral("AL"),
            .accent_color  = generated_playlist_color(accent_index++),
            .system_generated = true,
            .songs         = album_songs,
        });

        ++added_albums;
        if (added_albums >= 4) {
            break;
        }
    }

    std::vector<ArtistInfo> sorted_artists = artists;
    std::sort(sorted_artists.begin(), sorted_artists.end(),
        [](const auto& lhs, const auto& rhs) {
            if (lhs.song_count != rhs.song_count) {
                return lhs.song_count > rhs.song_count;
            }
            return lhs.name.localeAwareCompare(rhs.name) < 0;
        });

    int added_artists = 0;
    for (const auto& artist : sorted_artists) {
        const auto artist_songs = songs_by_artist.value(artist.name);
        if (artist_songs.size() < 2) {
            continue;
        }
        if (artist.name == QStringLiteral("Unknown Artist")) {
            continue;
        }

        playlists.push_back(PlaylistInfo {
            .id            = QStringLiteral("playlist-artist-%1").arg(make_slug(artist.name)),
            .title         = artist.name,
            .description   = QStringLiteral("按歌手自动聚合，适合连续听同一位创作者的作品。"),
            .meta          = QStringLiteral("%1 songs · %2").arg(artist.song_count).arg(artist.subtitle),
            .badge         = artist.cover_badge.isEmpty() ? make_cover_badge(artist.name) : artist.cover_badge,
            .sidebar_meta  = QStringLiteral("%1 songs · 歌手").arg(artist.song_count),
            .sidebar_badge = QStringLiteral("AR"),
            .accent_color  = generated_playlist_color(accent_index++),
            .system_generated = true,
            .songs         = artist_songs,
        });

        ++added_artists;
        if (added_artists >= 4) {
            break;
        }
    }

    return playlists;
}

}

LibraryService::LibraryService(QObject* parent)
    : QObject(parent) {
    if (open_database()) {
        initialize_database();
        if (load_from_storage()) {
            return;
        }
    }

    load_demo_library();
}

const std::vector<SongInfo>& LibraryService::songs() const noexcept { return m_songs; }

const std::vector<ArtistInfo>& LibraryService::artists() const noexcept { return m_artists; }

const std::vector<AlbumInfo>& LibraryService::albums() const noexcept { return m_albums; }

const std::vector<PlaylistInfo>& LibraryService::playlists() const noexcept { return m_playlists; }

const std::vector<ScanRootInfo>& LibraryService::scan_roots() const noexcept { return m_scan_roots; }

const QString& LibraryService::last_scan_message() const noexcept { return m_last_scan_message; }

const QDateTime& LibraryService::last_scan_at() const noexcept { return m_last_scan_at; }

bool LibraryService::is_scanning() const noexcept { return m_is_scanning; }

bool LibraryService::open_database() {
    auto app_data_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (app_data_dir.isEmpty()) {
        app_data_dir = QDir::homePath() + QStringLiteral("/.local/share/MyMusicPlayer");
    }
    QDir {}.mkpath(app_data_dir);
    m_database_path = QDir(app_data_dir).filePath(QStringLiteral("library.db"));

    if (QSqlDatabase::contains(kConnectionName)) {
        m_database = QSqlDatabase::database(kConnectionName);
    } else {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    }
    m_database.setDatabaseName(m_database_path);

    if (!m_database.open()) {
        m_last_scan_message = QStringLiteral("SQLite 打开失败：%1").arg(m_database.lastError().text());
        return false;
    }
    return true;
}

void LibraryService::initialize_database() {
    QSqlQuery query { m_database };
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS tracks ("
        "id TEXT PRIMARY KEY,"
        "title TEXT,"
        "artist TEXT,"
        "album TEXT,"
        "file_path TEXT,"
        "duration_text TEXT,"
        "cover_badge TEXT,"
        "cover_path TEXT,"
        "track_number INTEGER,"
        "liked INTEGER DEFAULT 0)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS artists ("
        "id TEXT PRIMARY KEY,"
        "name TEXT,"
        "subtitle TEXT,"
        "cover_badge TEXT,"
        "cover_path TEXT,"
        "song_count INTEGER,"
        "album_count INTEGER)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS albums ("
        "id TEXT PRIMARY KEY,"
        "title TEXT,"
        "artist TEXT,"
        "subtitle TEXT,"
        "cover_badge TEXT,"
        "cover_path TEXT,"
        "track_count INTEGER)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS playlists ("
        "id TEXT PRIMARY KEY,"
        "title TEXT,"
        "description TEXT,"
        "meta TEXT,"
        "badge TEXT,"
        "sidebar_meta TEXT,"
        "sidebar_badge TEXT,"
        "accent_color TEXT,"
        "is_system_generated INTEGER DEFAULT 0)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS playlist_tracks ("
        "playlist_id TEXT,"
        "track_id TEXT,"
        "sort_order INTEGER,"
        "PRIMARY KEY(playlist_id, sort_order))"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS scan_roots ("
        "path TEXT PRIMARY KEY,"
        "last_scan_at TEXT,"
        "discovered_count INTEGER)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS scan_state ("
        "state_key TEXT PRIMARY KEY,"
        "state_value TEXT)"));
    query.exec(QStringLiteral(
        "ALTER TABLE playlists ADD COLUMN is_system_generated INTEGER DEFAULT 0"));
}

bool LibraryService::load_from_storage() {
    if (!m_database.isOpen()) {
        return false;
    }

    std::vector<SongInfo> songs;
    std::vector<ArtistInfo> artists;
    std::vector<AlbumInfo> albums;
    std::vector<PlaylistInfo> playlists;
    std::vector<ScanRootInfo> scan_roots;

    QHash<QString, SongInfo> song_map;

    QSqlQuery tracks_query { m_database };
    tracks_query.exec(QStringLiteral(
        "SELECT id, title, artist, album, file_path, duration_text, cover_badge, cover_path, "
        "track_number, liked FROM tracks ORDER BY track_number ASC, title ASC"));
    while (tracks_query.next()) {
        auto song = SongInfo {
            .id            = tracks_query.value(0).toString(),
            .title         = tracks_query.value(1).toString(),
            .artist        = tracks_query.value(2).toString(),
            .album         = tracks_query.value(3).toString(),
            .file_path     = tracks_query.value(4).toString(),
            .duration_text = tracks_query.value(5).toString(),
            .cover_badge   = tracks_query.value(6).toString(),
            .cover_path    = tracks_query.value(7).toString(),
            .track_number  = tracks_query.value(8).toInt(),
            .liked         = tracks_query.value(9).toInt() != 0,
        };
        song_map.insert(song.id, song);
        songs.push_back(song);
    }

    QSqlQuery artists_query { m_database };
    artists_query.exec(QStringLiteral(
        "SELECT id, name, subtitle, cover_badge, cover_path, song_count, album_count FROM artists"));
    while (artists_query.next()) {
        artists.push_back(ArtistInfo {
            .id = artists_query.value(0).toString(),
            .name = artists_query.value(1).toString(),
            .subtitle = artists_query.value(2).toString(),
            .cover_badge = artists_query.value(3).toString(),
            .cover_path = artists_query.value(4).toString(),
            .song_count = artists_query.value(5).toInt(),
            .album_count = artists_query.value(6).toInt(),
        });
    }

    QSqlQuery albums_query { m_database };
    albums_query.exec(QStringLiteral(
        "SELECT id, title, artist, subtitle, cover_badge, cover_path, track_count FROM albums"));
    while (albums_query.next()) {
        albums.push_back(AlbumInfo {
            .id = albums_query.value(0).toString(),
            .title = albums_query.value(1).toString(),
            .artist = albums_query.value(2).toString(),
            .subtitle = albums_query.value(3).toString(),
            .cover_badge = albums_query.value(4).toString(),
            .cover_path = albums_query.value(5).toString(),
            .track_count = albums_query.value(6).toInt(),
        });
    }

    QHash<QString, PlaylistInfo> playlist_map;
    QSqlQuery playlists_query { m_database };
    playlists_query.exec(QStringLiteral(
        "SELECT id, title, description, meta, badge, sidebar_meta, sidebar_badge, accent_color, "
        "is_system_generated "
        "FROM playlists"));
    while (playlists_query.next()) {
        auto playlist = PlaylistInfo {
            .id = playlists_query.value(0).toString(),
            .title = playlists_query.value(1).toString(),
            .description = playlists_query.value(2).toString(),
            .meta = playlists_query.value(3).toString(),
            .badge = playlists_query.value(4).toString(),
            .sidebar_meta = playlists_query.value(5).toString(),
            .sidebar_badge = playlists_query.value(6).toString(),
            .accent_color = QColor(playlists_query.value(7).toString()),
            .system_generated = playlists_query.value(8).toInt() != 0,
        };
        playlist_map.insert(playlist.id, playlist);
    }

    QSqlQuery playlist_tracks_query { m_database };
    playlist_tracks_query.exec(QStringLiteral(
        "SELECT playlist_id, track_id FROM playlist_tracks ORDER BY playlist_id, sort_order ASC"));
    while (playlist_tracks_query.next()) {
        const auto playlist_id = playlist_tracks_query.value(0).toString();
        const auto track_id    = playlist_tracks_query.value(1).toString();
        if (playlist_map.contains(playlist_id) && song_map.contains(track_id)) {
            playlist_map[playlist_id].songs.push_back(song_map.value(track_id));
        }
    }
    const auto playlist_values = playlist_map.values();
    playlists.reserve(playlist_values.size());
    for (const auto& playlist : playlist_values) {
        playlists.push_back(playlist);
    }

    QSqlQuery roots_query { m_database };
    roots_query.exec(QStringLiteral(
        "SELECT path, last_scan_at, discovered_count FROM scan_roots ORDER BY path ASC"));
    while (roots_query.next()) {
        scan_roots.push_back(ScanRootInfo {
            .path = roots_query.value(0).toString(),
            .last_scan_at = QDateTime::fromString(roots_query.value(1).toString(), Qt::ISODate),
            .discovered_count = roots_query.value(2).toInt(),
        });
    }

    QSqlQuery state_query { m_database };
    state_query.exec(QStringLiteral("SELECT state_key, state_value FROM scan_state"));
    while (state_query.next()) {
        const auto key = state_query.value(0).toString();
        const auto value = state_query.value(1).toString();
        if (key == QStringLiteral("last_scan_message")) {
            m_last_scan_message = value;
        } else if (key == QStringLiteral("last_scan_at")) {
            m_last_scan_at = QDateTime::fromString(value, Qt::ISODate);
        }
    }

    if (songs.empty() && playlists.empty() && scan_roots.empty()) {
        return false;
    }

    m_songs      = std::move(songs);
    m_artists    = std::move(artists);
    m_albums     = std::move(albums);
    m_playlists  = std::move(playlists);
    m_scan_roots = std::move(scan_roots);
    return true;
}

void LibraryService::save_to_storage() {
    if (!m_database.isOpen()) {
        return;
    }

    QSqlQuery query { m_database };
    m_database.transaction();
    query.exec(QStringLiteral("DELETE FROM tracks"));
    query.exec(QStringLiteral("DELETE FROM artists"));
    query.exec(QStringLiteral("DELETE FROM albums"));
    query.exec(QStringLiteral("DELETE FROM playlists"));
    query.exec(QStringLiteral("DELETE FROM playlist_tracks"));
    query.exec(QStringLiteral("DELETE FROM scan_roots"));
    query.exec(QStringLiteral("DELETE FROM scan_state"));

    query.prepare(QStringLiteral(
        "INSERT INTO tracks (id, title, artist, album, file_path, duration_text, cover_badge, "
        "cover_path, track_number, liked) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& song : m_songs) {
        query.addBindValue(song.id);
        query.addBindValue(song.title);
        query.addBindValue(song.artist);
        query.addBindValue(song.album);
        query.addBindValue(song.file_path);
        query.addBindValue(song.duration_text);
        query.addBindValue(song.cover_badge);
        query.addBindValue(song.cover_path);
        query.addBindValue(song.track_number);
        query.addBindValue(song.liked ? 1 : 0);
        query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO artists (id, name, subtitle, cover_badge, cover_path, song_count, album_count) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& artist : m_artists) {
        query.addBindValue(artist.id);
        query.addBindValue(artist.name);
        query.addBindValue(artist.subtitle);
        query.addBindValue(artist.cover_badge);
        query.addBindValue(artist.cover_path);
        query.addBindValue(artist.song_count);
        query.addBindValue(artist.album_count);
        query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO albums (id, title, artist, subtitle, cover_badge, cover_path, track_count) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& album : m_albums) {
        query.addBindValue(album.id);
        query.addBindValue(album.title);
        query.addBindValue(album.artist);
        query.addBindValue(album.subtitle);
        query.addBindValue(album.cover_badge);
        query.addBindValue(album.cover_path);
        query.addBindValue(album.track_count);
        query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO playlists (id, title, description, meta, badge, sidebar_meta, sidebar_badge, accent_color, is_system_generated) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& playlist : m_playlists) {
        query.addBindValue(playlist.id);
        query.addBindValue(playlist.title);
        query.addBindValue(playlist.description);
        query.addBindValue(playlist.meta);
        query.addBindValue(playlist.badge);
        query.addBindValue(playlist.sidebar_meta);
        query.addBindValue(playlist.sidebar_badge);
        query.addBindValue(playlist.accent_color.name(QColor::HexArgb));
        query.addBindValue(playlist.system_generated ? 1 : 0);
        query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO playlist_tracks (playlist_id, track_id, sort_order) VALUES (?, ?, ?)"));
    for (const auto& playlist : m_playlists) {
        for (int index = 0; index < static_cast<int>(playlist.songs.size()); ++index) {
            query.addBindValue(playlist.id);
            query.addBindValue(playlist.songs[index].id);
            query.addBindValue(index);
            query.exec();
        }
    }

    query.prepare(QStringLiteral(
        "INSERT INTO scan_roots (path, last_scan_at, discovered_count) VALUES (?, ?, ?)"));
    for (const auto& root : m_scan_roots) {
        query.addBindValue(root.path);
        query.addBindValue(root.last_scan_at.toString(Qt::ISODate));
        query.addBindValue(root.discovered_count);
        query.exec();
    }

    query.prepare(QStringLiteral("INSERT INTO scan_state (state_key, state_value) VALUES (?, ?)"));
    query.addBindValue(QStringLiteral("last_scan_message"));
    query.addBindValue(m_last_scan_message);
    query.exec();
    query.addBindValue(QStringLiteral("last_scan_at"));
    query.addBindValue(m_last_scan_at.toString(Qt::ISODate));
    query.exec();

    m_database.commit();
}

void LibraryService::replace_library(std::vector<SongInfo> songs, std::vector<ArtistInfo> artists,
    std::vector<AlbumInfo> albums, std::vector<PlaylistInfo> playlists) {
    m_songs     = std::move(songs);
    m_artists   = std::move(artists);
    m_albums    = std::move(albums);
    m_playlists = std::move(playlists);
    save_to_storage();
    emit library_changed();
}

void LibraryService::load_demo_library() {
    replace_library(
        {
            { .id = "tell-your-world", .title = "Tell Your World", .artist = "livetune feat. Hatsune Miku", .album = "Re:Dial", .file_path = "/demo/tell-your-world.mp3", .duration_text = "4:18", .cover_badge = "TW", .track_number = 1 },
            { .id = "melt", .title = "Melt", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/melt.mp3", .duration_text = "4:21", .cover_badge = "ME", .track_number = 2 },
            { .id = "world-is-mine", .title = "World is Mine", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/world-is-mine.mp3", .duration_text = "4:15", .cover_badge = "WM", .track_number = 3 },
            { .id = "39", .title = "39", .artist = "sasakure.UK x DECO*27", .album = "Magical Mirai", .file_path = "/demo/39.mp3", .duration_text = "3:54", .cover_badge = "39", .track_number = 4 },
            { .id = "highlight", .title = "Highlight", .artist = "KIRA feat. Miku", .album = "Single", .file_path = "/demo/highlight.mp3", .duration_text = "3:41", .cover_badge = "HL", .track_number = 5 },
            { .id = "blue-star", .title = "Blue Star", .artist = "Hachioji-P", .album = "Single", .file_path = "/demo/blue-star.mp3", .duration_text = "4:01", .cover_badge = "BS", .track_number = 6 },
        },
        {
            { .id = "artist-livetune", .name = "livetune", .subtitle = "电子流行 / Vocaloid", .cover_badge = "LV", .song_count = 3, .album_count = 2 },
            { .id = "artist-ryo", .name = "ryo", .subtitle = "supercell / 经典早期曲目", .cover_badge = "RY", .song_count = 2, .album_count = 1 },
            { .id = "artist-kira", .name = "KIRA", .subtitle = "偏国际化的电子流行线条", .cover_badge = "KR", .song_count = 1, .album_count = 1 },
        },
        {
            { .id = "album-redial", .title = "Re:Dial", .artist = "livetune", .subtitle = "EP", .cover_badge = "RD", .track_count = 4 },
            { .id = "album-supercell", .title = "supercell", .artist = "ryo", .subtitle = "Studio Album", .cover_badge = "SC", .track_count = 15 },
            { .id = "album-magical-mirai", .title = "Magical Mirai", .artist = "Various Artists", .subtitle = "Compilation", .cover_badge = "MM", .track_count = 12 },
        },
        {
            {
                .id            = "playlist-miku-daily",
                .title         = "Miku Daily Mix",
                .description   = "轻快的电子流行和偏透明的 Vocal 质感，适合播放器主页默认展示。",
                .meta          = "12 songs · 42 min · 今天想听一点发光的青绿色",
                .badge         = "MIKU",
                .sidebar_meta  = "12 songs · 常听",
                .sidebar_badge = "MI",
                .accent_color  = QColor(181, 233, 229),
                .system_generated = true,
                .songs         = {
                    { .id = "tell-your-world", .title = "Tell Your World", .artist = "livetune feat. Hatsune Miku", .album = "Re:Dial", .file_path = "/demo/tell-your-world.mp3", .duration_text = "4:18", .cover_badge = "TW", .track_number = 1 },
                    { .id = "melt", .title = "Melt", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/melt.mp3", .duration_text = "4:21", .cover_badge = "ME", .track_number = 2 },
                    { .id = "world-is-mine", .title = "World is Mine", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/world-is-mine.mp3", .duration_text = "4:15", .cover_badge = "WM", .track_number = 3 },
                    { .id = "39", .title = "39", .artist = "sasakure.UK x DECO*27", .album = "Magical Mirai", .file_path = "/demo/39.mp3", .duration_text = "3:54", .cover_badge = "39", .track_number = 4 },
                },
            },
            {
                .id            = "playlist-night-drive",
                .title         = "Night Drive",
                .description   = "更冷一点、更直一点的合成器线条，适合夜晚或者长路。",
                .meta          = "28 songs · 96 min · 低饱和蓝绿和都市感",
                .badge         = "NDRV",
                .sidebar_meta  = "28 songs · 合成器",
                .sidebar_badge = "ND",
                .accent_color  = QColor(189, 226, 239),
                .system_generated = true,
                .songs         = {
                    { .id = "highlight", .title = "Highlight", .artist = "KIRA feat. Miku", .album = "Single", .file_path = "/demo/highlight.mp3", .duration_text = "3:41", .cover_badge = "HL", .track_number = 1 },
                    { .id = "digital-girl", .title = "Digital Girl", .artist = "KIRA", .album = "Single", .file_path = "/demo/digital-girl.mp3", .duration_text = "3:23", .cover_badge = "DG", .track_number = 2 },
                    { .id = "blue-star", .title = "Blue Star", .artist = "Hachioji-P", .album = "Single", .file_path = "/demo/blue-star.mp3", .duration_text = "4:01", .cover_badge = "BS", .track_number = 3 },
                    { .id = "last-night-good-night", .title = "Last Night, Good Night", .artist = "livetune", .album = "Re:Package", .file_path = "/demo/last-night-good-night.mp3", .duration_text = "5:42", .cover_badge = "LG", .track_number = 4 },
                },
            },
            {
                .id            = "playlist-soft-vocal",
                .title         = "Soft Vocal",
                .description   = "偏温柔的人声和轻拍节奏，适合写字、发呆或者整理播放器。",
                .meta          = "16 songs · 58 min · 声音更靠前，封面更柔和",
                .badge         = "SOFT",
                .sidebar_meta  = "16 songs · 人声",
                .sidebar_badge = "SV",
                .accent_color  = QColor(214, 234, 226),
                .system_generated = true,
                .songs         = {
                    { .id = "from-y-to-y", .title = "From Y to Y", .artist = "JimmyThumb-P", .album = "Single", .file_path = "/demo/from-y-to-y.mp3", .duration_text = "5:36", .cover_badge = "YY", .track_number = 1 },
                    { .id = "glow", .title = "Glow", .artist = "keeno", .album = "Single", .file_path = "/demo/glow.mp3", .duration_text = "4:48", .cover_badge = "GL", .track_number = 2 },
                    { .id = "starduster", .title = "Starduster", .artist = "JimmyThumb-P", .album = "Single", .file_path = "/demo/starduster.mp3", .duration_text = "5:01", .cover_badge = "SD", .track_number = 3 },
                    { .id = "deep-sea-girl", .title = "Deep Sea Girl", .artist = "Yuuyu", .album = "Single", .file_path = "/demo/deep-sea-girl.mp3", .duration_text = "3:37", .cover_badge = "DS", .track_number = 4 },
                },
            },
            {
                .id            = "playlist-study-loop",
                .title         = "Study Loop",
                .description   = "重复性更强、存在感更低的循环歌单，适合长期后台播放。",
                .meta          = "33 songs · 121 min · 节奏平稳，注意力友好",
                .badge         = "LOOP",
                .sidebar_meta  = "33 songs · 轻节奏",
                .sidebar_badge = "SL",
                .accent_color  = QColor(220, 226, 229),
                .system_generated = true,
                .songs         = {
                    { .id = "weekender-girl", .title = "Weekender Girl", .artist = "kz(livetune) x Hachioji-P", .album = "Single", .file_path = "/demo/weekender-girl.mp3", .duration_text = "3:33", .cover_badge = "WG", .track_number = 1 },
                    { .id = "decorator", .title = "DECORATOR", .artist = "livetune", .album = "Single", .file_path = "/demo/decorator.mp3", .duration_text = "3:30", .cover_badge = "DE", .track_number = 2 },
                    { .id = "hand-in-hand", .title = "Hand in Hand", .artist = "livetune", .album = "Magical Mirai", .file_path = "/demo/hand-in-hand.mp3", .duration_text = "5:12", .cover_badge = "HH", .track_number = 3 },
                    { .id = "love-trial", .title = "Love Trial", .artist = "40mP", .album = "Single", .file_path = "/demo/love-trial.mp3", .duration_text = "3:44", .cover_badge = "LT", .track_number = 4 },
                },
            },
        });

    m_last_scan_message = QStringLiteral("当前使用 demo 资料库");
    m_last_scan_at      = QDateTime::currentDateTime();
    save_to_storage();
    emit scan_status_changed();
}

bool LibraryService::add_scan_root(const QString& path) {
    const QFileInfo info { path };
    if (!info.exists() || !info.isDir()) {
        return false;
    }

    const auto normalized = info.absoluteFilePath();
    for (const auto& root : m_scan_roots) {
        if (root.path == normalized) {
            m_last_scan_message = QStringLiteral("目录已存在：%1").arg(normalized);
            emit scan_status_changed();
            return false;
        }
    }

    m_scan_roots.push_back(ScanRootInfo { .path = normalized });
    save_to_storage();
    emit scan_roots_changed();
    m_last_scan_message = QStringLiteral("已添加扫描目录：%1").arg(normalized);
    emit scan_status_changed();
    return true;
}

void LibraryService::scan_library() {
    if (m_is_scanning) {
        return;
    }

    m_is_scanning = true;
    m_last_scan_message = QStringLiteral("正在扫描 %1 个目录...").arg(m_scan_roots.size());
    emit scan_started();
    emit scan_status_changed();

    if (m_scan_roots.empty()) {
        m_is_scanning = false;
        m_last_scan_message = QStringLiteral("还没有添加扫描目录");
        m_last_scan_at = QDateTime::currentDateTime();
        save_to_storage();
        emit scan_finished();
        emit scan_status_changed();
        return;
    }

    const QStringList patterns {
        QStringLiteral("*.mp3"),
        QStringLiteral("*.flac"),
        QStringLiteral("*.wav"),
        QStringLiteral("*.ogg"),
        QStringLiteral("*.m4a"),
        QStringLiteral("*.aac"),
    };

    std::vector<SongInfo> songs;
    QHash<QString, int> artist_song_count;
    QHash<QString, QSet<QString>> artist_album_set;
    QHash<QString, QPair<QString, int>> album_info;

    const auto now = QDateTime::currentDateTime();
    int track_number = 1;

    for (auto& root : m_scan_roots) {
        int discovered = 0;
        QDirIterator iterator(root.path, patterns, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const auto path = iterator.next();
            auto song = parse_song_from_file(path, track_number++);
            ++discovered;

            artist_song_count[song.artist] += 1;
            artist_album_set[song.artist].insert(song.album);
            album_info[song.album].first = song.artist;
            album_info[song.album].second += 1;

            songs.push_back(std::move(song));
        }

        root.discovered_count = discovered;
        root.last_scan_at     = now;
    }

    std::vector<ArtistInfo> artists;
    for (auto it = artist_song_count.cbegin(); it != artist_song_count.cend(); ++it) {
        artists.push_back(ArtistInfo {
            .id = QStringLiteral("artist-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)),
            .name = it.key(),
            .subtitle = QStringLiteral("来自本地扫描资料库"),
            .cover_badge = make_cover_badge(it.key()),
            .song_count = it.value(),
            .album_count = static_cast<int>(artist_album_set.value(it.key()).size()),
        });
    }

    std::vector<AlbumInfo> albums;
    for (auto it = album_info.cbegin(); it != album_info.cend(); ++it) {
        albums.push_back(AlbumInfo {
            .id = QStringLiteral("album-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)),
            .title = it.key(),
            .artist = it.value().first,
            .subtitle = QStringLiteral("来自本地扫描资料库"),
            .cover_badge = make_cover_badge(it.key()),
            .track_count = it.value().second,
        });
    }

    auto playlists = build_generated_playlists(songs, artists, albums, m_scan_roots.size());
    auto manual_playlists = collect_manual_playlists(m_playlists);
    playlists.insert(playlists.end(),
        std::make_move_iterator(manual_playlists.begin()),
        std::make_move_iterator(manual_playlists.end()));
    replace_library(std::move(songs), std::move(artists), std::move(albums), std::move(playlists));
    m_last_scan_at = now;
    m_last_scan_message = QStringLiteral("扫描完成，共发现 %1 首音频文件").arg(m_songs.size());
    m_is_scanning = false;
    save_to_storage();
    emit scan_roots_changed();
    emit scan_finished();
    emit scan_status_changed();
}
