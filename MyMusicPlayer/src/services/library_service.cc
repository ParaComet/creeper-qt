#include "library_service.hh"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QPixmap>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>
#include <QVariant>

#include <algorithm>

using namespace mymusic::model;

namespace {

constexpr auto kConnectionName = "mymusic_library_service";
const auto kLikedPlaylistColor = QColor(245, 224, 230);

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

auto find_cover_image_for_directory(const QDir& dir) -> QString {
    static const QStringList names {
        QStringLiteral("cover.jpg"),
        QStringLiteral("cover.jpeg"),
        QStringLiteral("cover.png"),
        QStringLiteral("folder.jpg"),
        QStringLiteral("folder.jpeg"),
        QStringLiteral("folder.png"),
        QStringLiteral("front.jpg"),
        QStringLiteral("front.jpeg"),
        QStringLiteral("front.png"),
    };

    for (const auto& name : names) {
        const auto candidate = dir.filePath(name);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

auto format_duration(qint64 duration_ms) -> QString {
    if (duration_ms <= 0) {
        return QStringLiteral("--:--");
    }

    const auto total_seconds = duration_ms / 1000;
    const auto minutes = total_seconds / 60;
    const auto seconds = total_seconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'));
}

auto metadata_string(const QMediaMetaData& metadata, QMediaMetaData::Key key) -> QString {
    return metadata.stringValue(key).trimmed();
}

auto extract_cover_image(const QMediaMetaData& metadata) -> QImage {
    const auto extract = [](const QVariant& value) -> QImage {
        if (value.canConvert<QImage>()) {
            return qvariant_cast<QImage>(value);
        }
        if (value.canConvert<QPixmap>()) {
            return qvariant_cast<QPixmap>(value).toImage();
        }
        return {};
    };

    auto image = extract(metadata.value(QMediaMetaData::CoverArtImage));
    if (!image.isNull()) {
        return image;
    }
    return extract(metadata.value(QMediaMetaData::ThumbnailImage));
}

auto cache_cover_image(const QImage& image, const QString& audio_path, const QString& cache_dir)
    -> QString {
    if (image.isNull()) {
        return {};
    }

    QDir {}.mkpath(cache_dir);
    const auto hash = QString::fromLatin1(
        QCryptographicHash::hash(audio_path.toUtf8(), QCryptographicHash::Sha1).toHex());
    const auto target_path = QDir(cache_dir).filePath(QStringLiteral("%1.png").arg(hash));

    if (image.save(target_path, "PNG")) {
        return target_path;
    }
    return {};
}

auto parse_song_from_file(const QString& path, int track_number, const QString& cover_cache_dir)
    -> SongInfo {
    const QFileInfo info { path };
    const auto parent = info.dir();
    const auto parent_dir = parent.dirName();
    const auto stem       = info.completeBaseName();

    QString artist = QStringLiteral("Unknown Artist");
    QString title  = stem;

    if (const auto separator = stem.indexOf(QStringLiteral(" - ")); separator > 0) {
        artist = stem.first(separator).trimmed();
        title  = stem.mid(separator + 3).trimmed();
    }

    auto song = SongInfo {
        .id            = info.absoluteFilePath(),
        .title         = title,
        .artist        = artist,
        .album         = parent_dir.isEmpty() ? QStringLiteral("Unknown Album") : parent_dir,
        .file_path     = info.absoluteFilePath(),
        .duration_text = QStringLiteral("--:--"),
        .cover_badge   = make_cover_badge(title),
        .cover_path    = find_cover_image_for_directory(parent),
        .track_number  = track_number,
    };

    auto player = QMediaPlayer {};
    auto loop = QEventLoop {};
    auto timeout = QTimer {};
    timeout.setSingleShot(true);
    timeout.setInterval(1500);

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, &loop,
        [&](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::InvalidMedia
                || status == QMediaPlayer::NoMedia) {
                loop.quit();
            }
        });
    QObject::connect(&player, &QMediaPlayer::errorOccurred, &loop,
        [&](QMediaPlayer::Error, const QString&) { loop.quit(); });

    player.setSource(QUrl::fromLocalFile(path));
    timeout.start();
    loop.exec();

    const auto metadata = player.metaData();
    if (const auto meta_title = metadata_string(metadata, QMediaMetaData::Title); !meta_title.isEmpty()) {
        song.title = meta_title;
    }

    const auto album_title = metadata_string(metadata, QMediaMetaData::AlbumTitle);
    if (!album_title.isEmpty()) {
        song.album = album_title;
    }

    const auto artist_name = [&]() -> QString {
        const auto album_artist = metadata_string(metadata, QMediaMetaData::AlbumArtist);
        if (!album_artist.isEmpty()) {
            return album_artist;
        }
        const auto lead = metadata_string(metadata, QMediaMetaData::LeadPerformer);
        if (!lead.isEmpty()) {
            return lead;
        }
        const auto author = metadata_string(metadata, QMediaMetaData::Author);
        if (!author.isEmpty()) {
            return author;
        }
        return metadata_string(metadata, QMediaMetaData::ContributingArtist);
    }();
    if (!artist_name.isEmpty()) {
        song.artist = artist_name;
    }

    const auto metadata_track_number = metadata.value(QMediaMetaData::TrackNumber);
    if (metadata_track_number.isValid()) {
        song.track_number = metadata_track_number.toInt();
    }

    song.duration_text = format_duration(player.duration());
    song.cover_badge = make_cover_badge(song.title);

    if (const auto embedded_cover =
            cache_cover_image(extract_cover_image(metadata), path, cover_cache_dir);
        !embedded_cover.isEmpty()) {
        song.cover_path = embedded_cover;
    }

    return song;
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

auto make_artist_id(const QString& artist_name) -> QString {
    return QStringLiteral("artist-%1").arg(make_slug(artist_name));
}

auto make_album_id(const QString& album_title) -> QString {
    return QStringLiteral("album-%1").arg(make_slug(album_title));
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

auto make_playlist(const QString& id, const QString& title, const QString& description,
    const QString& badge, const QString& sidebar_badge, const QColor& accent_color,
    std::vector<SongInfo> songs, const QString& meta_suffix, const QString& sidebar_suffix)
    -> PlaylistInfo {
    const auto count = songs.size();

    return PlaylistInfo {
        .id               = id,
        .title            = title,
        .description      = description,
        .meta             = QStringLiteral("%1 songs · %2").arg(count).arg(meta_suffix),
        .badge            = badge,
        .sidebar_meta     = QStringLiteral("%1 songs · %2").arg(count).arg(sidebar_suffix),
        .sidebar_badge    = sidebar_badge,
        .accent_color     = accent_color,
        .system_generated = true,
        .songs            = std::move(songs),
    };
}

auto build_generated_playlists(
    const std::vector<SongInfo>& songs, const std::vector<ArtistInfo>& artists,
    const std::vector<AlbumInfo>& albums, int scan_root_count)
    -> std::vector<PlaylistInfo> {
    Q_UNUSED(artists);
    Q_UNUSED(albums);

    std::vector<PlaylistInfo> playlists;
    if (songs.empty()) {
        return playlists;
    }

    std::vector<SongInfo> liked_songs;
    liked_songs.reserve(songs.size());
    for (const auto& song : songs) {
        if (song.liked) {
            liked_songs.push_back(song);
        }
    }

    playlists.push_back(make_playlist(QStringLiteral("playlist-library-liked"),
        QStringLiteral("我喜欢的音乐"),
        QStringLiteral("收藏的歌曲会自动汇总到这里，方便快速回到最常听的内容～"),
        QStringLiteral("LIKED"), QStringLiteral("LK"), kLikedPlaylistColor,
        std::move(liked_songs), QStringLiteral("收藏歌曲"), QStringLiteral("收藏")));

    playlists.push_back(make_playlist(QStringLiteral("playlist-library-all"),
        QStringLiteral("本地全部音乐"),
        QStringLiteral("由扫描目录自动生成，包含当前本地资料库中的全部歌曲～"),
        QStringLiteral("LOCAL"), QStringLiteral("LO"), generated_playlist_color(1), songs,
        QStringLiteral("%1 个扫描目录").arg(scan_root_count), QStringLiteral("资料库")));

    std::vector<SongInfo> recent_songs;
    recent_songs.reserve(std::min<std::size_t>(songs.size(), 20));
    for (auto it = songs.rbegin(); it != songs.rend() && recent_songs.size() < 20; ++it) {
        recent_songs.push_back(*it);
    }
    std::reverse(recent_songs.begin(), recent_songs.end());

    playlists.push_back(make_playlist(QStringLiteral("playlist-library-recent"),
        QStringLiteral("最近导入"),
        QStringLiteral("最近一次扫描中整理出来的曲目，适合先快速试听一轮～"),
        QStringLiteral("RECENT"), QStringLiteral("RC"), generated_playlist_color(2),
        std::move(recent_songs), QStringLiteral("最近扫描"), QStringLiteral("新导入")));

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

std::vector<SongInfo> LibraryService::songs_for_source(const QString& source_id) const {
    if (source_id == QStringLiteral("all-music")) {
        return m_songs;
    }

    if (const auto* playlist = find_playlist(source_id); playlist != nullptr) {
        return playlist->songs;
    }

    std::vector<SongInfo> matched_songs;
    if (source_id.startsWith(QStringLiteral("artist-"))) {
        auto artist_name = QString {};
        for (const auto& artist : m_artists) {
            if (artist.id == source_id) {
                artist_name = artist.name;
                break;
            }
        }
        for (const auto& song : m_songs) {
            if (song.artist == artist_name) {
                matched_songs.push_back(song);
            }
        }
        return matched_songs;
    }

    if (source_id.startsWith(QStringLiteral("album-"))) {
        auto album_title = QString {};
        for (const auto& album : m_albums) {
            if (album.id == source_id) {
                album_title = album.title;
                break;
            }
        }
        for (const auto& song : m_songs) {
            if (song.album == album_title) {
                matched_songs.push_back(song);
            }
        }
        return matched_songs;
    }

    return {};
}

const QString& LibraryService::last_scan_message() const noexcept { return m_last_scan_message; }

const QDateTime& LibraryService::last_scan_at() const noexcept { return m_last_scan_at; }

bool LibraryService::is_scanning() const noexcept { return m_is_scanning; }

const PlaylistInfo* LibraryService::find_playlist(const QString& playlist_id) const noexcept {
    for (const auto& playlist : m_playlists) {
        if (playlist.id == playlist_id) {
            return &playlist;
        }
    }
    return nullptr;
}

bool LibraryService::source_supports_entity_favorite(const QString& source_id) const noexcept {
    return source_id.startsWith(QStringLiteral("artist-"))
        || source_id.startsWith(QStringLiteral("album-"));
}

bool LibraryService::source_entity_liked(const QString& source_id) const noexcept {
    if (source_id.startsWith(QStringLiteral("artist-"))) {
        for (const auto& artist : m_artists) {
            if (artist.id == source_id) {
                return artist.liked;
            }
        }
    }

    if (source_id.startsWith(QStringLiteral("album-"))) {
        for (const auto& album : m_albums) {
            if (album.id == source_id) {
                return album.liked;
            }
        }
    }

    return false;
}

bool LibraryService::set_source_entity_liked(const QString& source_id, bool liked) {
    auto changed = false;

    if (source_id.startsWith(QStringLiteral("artist-"))) {
        for (auto& artist : m_artists) {
            if (artist.id == source_id && artist.liked != liked) {
                artist.liked = liked;
                changed = true;
            }
        }
    } else if (source_id.startsWith(QStringLiteral("album-"))) {
        for (auto& album : m_albums) {
            if (album.id == source_id && album.liked != liked) {
                album.liked = liked;
                changed = true;
            }
        }
    }

    if (!changed) {
        return false;
    }

    save_to_storage();
    emit library_changed();
    return true;
}

QString LibraryService::first_source_directory(const QString& source_id) const {
    const auto songs = songs_for_source(source_id);
    if (songs.empty()) {
        return {};
    }

    const QFileInfo info { songs.front().file_path };
    return info.absolutePath();
}

bool LibraryService::set_song_liked(const QString& song_id, bool liked) {
    auto changed = false;

    for (auto& song : m_songs) {
        if (song.id == song_id && song.liked != liked) {
            song.liked = liked;
            changed = true;
        }
    }

    for (auto& playlist : m_playlists) {
        for (auto& song : playlist.songs) {
            if (song.id == song_id) {
                song.liked = liked;
            }
        }
    }

    if (!changed) {
        return false;
    }

    auto manual_playlists = collect_manual_playlists(m_playlists);
    auto regenerated_playlists = build_generated_playlists(
        m_songs, m_artists, m_albums, static_cast<int>(m_scan_roots.size()));
    regenerated_playlists.insert(regenerated_playlists.end(),
        std::make_move_iterator(manual_playlists.begin()),
        std::make_move_iterator(manual_playlists.end()));
    m_playlists = std::move(regenerated_playlists);

    save_to_storage();
    emit library_changed();
    return true;
}

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
        "album_count INTEGER,"
        "liked INTEGER DEFAULT 0)"));
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS albums ("
        "id TEXT PRIMARY KEY,"
        "title TEXT,"
        "artist TEXT,"
        "subtitle TEXT,"
        "cover_badge TEXT,"
        "cover_path TEXT,"
        "track_count INTEGER,"
        "liked INTEGER DEFAULT 0)"));
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
    query.exec(QStringLiteral(
        "ALTER TABLE artists ADD COLUMN liked INTEGER DEFAULT 0"));
    query.exec(QStringLiteral(
        "ALTER TABLE albums ADD COLUMN liked INTEGER DEFAULT 0"));
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
        "SELECT id, name, subtitle, cover_badge, cover_path, song_count, album_count, liked FROM artists"));
    while (artists_query.next()) {
        artists.push_back(ArtistInfo {
            .id = artists_query.value(0).toString(),
            .name = artists_query.value(1).toString(),
            .subtitle = artists_query.value(2).toString(),
            .cover_badge = artists_query.value(3).toString(),
            .cover_path = artists_query.value(4).toString(),
            .song_count = artists_query.value(5).toInt(),
            .album_count = artists_query.value(6).toInt(),
            .liked = artists_query.value(7).toInt() != 0,
        });
    }

    QSqlQuery albums_query { m_database };
    albums_query.exec(QStringLiteral(
        "SELECT id, title, artist, subtitle, cover_badge, cover_path, track_count, liked FROM albums"));
    while (albums_query.next()) {
        albums.push_back(AlbumInfo {
            .id = albums_query.value(0).toString(),
            .title = albums_query.value(1).toString(),
            .artist = albums_query.value(2).toString(),
            .subtitle = albums_query.value(3).toString(),
            .cover_badge = albums_query.value(4).toString(),
            .cover_path = albums_query.value(5).toString(),
            .track_count = albums_query.value(6).toInt(),
            .liked = albums_query.value(7).toInt() != 0,
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

    auto manual_playlists = collect_manual_playlists(playlists);
    auto regenerated_playlists =
        build_generated_playlists(songs, artists, albums, static_cast<int>(scan_roots.size()));
    regenerated_playlists.insert(regenerated_playlists.end(),
        std::make_move_iterator(manual_playlists.begin()),
        std::make_move_iterator(manual_playlists.end()));
    playlists = std::move(regenerated_playlists);

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
        "INSERT INTO artists (id, name, subtitle, cover_badge, cover_path, song_count, album_count, liked) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& artist : m_artists) {
        query.addBindValue(artist.id);
        query.addBindValue(artist.name);
        query.addBindValue(artist.subtitle);
        query.addBindValue(artist.cover_badge);
        query.addBindValue(artist.cover_path);
        query.addBindValue(artist.song_count);
        query.addBindValue(artist.album_count);
        query.addBindValue(artist.liked ? 1 : 0);
        query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO albums (id, title, artist, subtitle, cover_badge, cover_path, track_count, liked) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const auto& album : m_albums) {
        query.addBindValue(album.id);
        query.addBindValue(album.title);
        query.addBindValue(album.artist);
        query.addBindValue(album.subtitle);
        query.addBindValue(album.cover_badge);
        query.addBindValue(album.cover_path);
        query.addBindValue(album.track_count);
        query.addBindValue(album.liked ? 1 : 0);
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
    std::vector<SongInfo> songs {
        { .id = "tell-your-world", .title = "Tell Your World", .artist = "livetune feat. Hatsune Miku", .album = "Re:Dial", .file_path = "/demo/tell-your-world.mp3", .duration_text = "4:18", .cover_badge = "TW", .track_number = 1, .liked = true },
        { .id = "melt", .title = "Melt", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/melt.mp3", .duration_text = "4:21", .cover_badge = "ME", .track_number = 2, .liked = true },
        { .id = "world-is-mine", .title = "World is Mine", .artist = "ryo feat. Hatsune Miku", .album = "supercell", .file_path = "/demo/world-is-mine.mp3", .duration_text = "4:15", .cover_badge = "WM", .track_number = 3 },
        { .id = "39", .title = "39", .artist = "sasakure.UK x DECO*27", .album = "Magical Mirai", .file_path = "/demo/39.mp3", .duration_text = "3:54", .cover_badge = "39", .track_number = 4, .liked = true },
        { .id = "highlight", .title = "Highlight", .artist = "KIRA feat. Miku", .album = "Single", .file_path = "/demo/highlight.mp3", .duration_text = "3:41", .cover_badge = "HL", .track_number = 5 },
        { .id = "blue-star", .title = "Blue Star", .artist = "Hachioji-P", .album = "Single", .file_path = "/demo/blue-star.mp3", .duration_text = "4:01", .cover_badge = "BS", .track_number = 6, .liked = true },
    };

    std::vector<ArtistInfo> artists {
        { .id = "artist-livetune", .name = "livetune", .subtitle = "电子流行 / Vocaloid", .cover_badge = "LV", .song_count = 3, .album_count = 2 },
        { .id = "artist-ryo", .name = "ryo", .subtitle = "supercell / 经典早期曲目", .cover_badge = "RY", .song_count = 2, .album_count = 1 },
        { .id = "artist-kira", .name = "KIRA", .subtitle = "偏国际化的电子流行线条", .cover_badge = "KR", .song_count = 1, .album_count = 1 },
    };

    std::vector<AlbumInfo> albums {
        { .id = "album-redial", .title = "Re:Dial", .artist = "livetune", .subtitle = "EP", .cover_badge = "RD", .track_count = 4 },
        { .id = "album-supercell", .title = "supercell", .artist = "ryo", .subtitle = "Studio Album", .cover_badge = "SC", .track_count = 15 },
        { .id = "album-magical-mirai", .title = "Magical Mirai", .artist = "Various Artists", .subtitle = "Compilation", .cover_badge = "MM", .track_count = 12 },
    };

    auto playlists = build_generated_playlists(songs, artists, albums, 1);
    replace_library(std::move(songs), std::move(artists), std::move(albums), std::move(playlists));

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
    const auto cover_cache_dir =
        QFileInfo(m_database_path).absoluteDir().filePath(QStringLiteral("covers"));

    std::vector<SongInfo> songs;
    QHash<QString, int> artist_song_count;
    QHash<QString, QSet<QString>> artist_album_set;
    QHash<QString, QPair<QString, int>> album_info;
    QHash<QString, QString> artist_cover_path;
    QHash<QString, QString> album_cover_path;
    QHash<QString, bool> existing_artist_liked;
    QHash<QString, bool> existing_album_liked;

    for (const auto& artist : m_artists) {
        existing_artist_liked.insert(artist.name, artist.liked);
    }
    for (const auto& album : m_albums) {
        existing_album_liked.insert(album.title, album.liked);
    }

    const auto now = QDateTime::currentDateTime();
    int track_number = 1;

    for (auto& root : m_scan_roots) {
        int discovered = 0;
        QDirIterator iterator(root.path, patterns, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const auto path = iterator.next();
            auto song = parse_song_from_file(path, track_number++, cover_cache_dir);
            ++discovered;

            artist_song_count[song.artist] += 1;
            artist_album_set[song.artist].insert(song.album);
            album_info[song.album].first = song.artist;
            album_info[song.album].second += 1;
            if (!song.cover_path.isEmpty()) {
                if (!artist_cover_path.contains(song.artist) || artist_cover_path.value(song.artist).isEmpty()) {
                    artist_cover_path.insert(song.artist, song.cover_path);
                }
                if (!album_cover_path.contains(song.album) || album_cover_path.value(song.album).isEmpty()) {
                    album_cover_path.insert(song.album, song.cover_path);
                }
            }

            songs.push_back(std::move(song));
        }

        root.discovered_count = discovered;
        root.last_scan_at     = now;
    }

    std::vector<ArtistInfo> artists;
    for (auto it = artist_song_count.cbegin(); it != artist_song_count.cend(); ++it) {
        artists.push_back(ArtistInfo {
            .id = make_artist_id(it.key()),
            .name = it.key(),
            .subtitle = QStringLiteral("来自本地扫描资料库"),
            .cover_badge = make_cover_badge(it.key()),
            .cover_path = artist_cover_path.value(it.key()),
            .song_count = it.value(),
            .album_count = static_cast<int>(artist_album_set.value(it.key()).size()),
            .liked = existing_artist_liked.value(it.key(), false),
        });
    }

    std::vector<AlbumInfo> albums;
    for (auto it = album_info.cbegin(); it != album_info.cend(); ++it) {
        albums.push_back(AlbumInfo {
            .id = make_album_id(it.key()),
            .title = it.key(),
            .artist = it.value().first,
            .subtitle = QStringLiteral("来自本地扫描资料库"),
            .cover_badge = make_cover_badge(it.key()),
            .cover_path = album_cover_path.value(it.key()),
            .track_count = it.value().second,
            .liked = existing_album_liked.value(it.key(), false),
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
