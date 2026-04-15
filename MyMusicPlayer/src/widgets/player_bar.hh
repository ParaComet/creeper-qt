#pragma once

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/widget/buttons/filled-button.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "creeper-qt/widget/sliders.hh"
#include "creeper-qt/widget/text.hh"
#include "creeper-qt/widget/widget.hh"

#include <QPixmap>
#include <QWidget>

class PlayerBar : public creeper::Widget {
    Q_OBJECT

public:
    explicit PlayerBar(creeper::ThemeManager& manager, QWidget* parent = nullptr);

    void set_track_info(const QString& title, const QString& artist, const QString& album);
    void set_cover(const QString& label = QStringLiteral("封面"), const QString& cover_path = {});
    void set_progress(double progress);
    void set_volume(double volume);
    void set_playing(bool playing);
    void set_favorite(bool favorite);
    void set_order_icon(const QString& icon_name);

signals:
    void control_signal(const QString& action);
    void play_pause_clicked();
    void next_clicked();
    void previous_clicked();
    void order_clicked();
    void seek_requested(double progress);
    void volume_changed(double volume);
    void favorite_toggled(bool checked);
    void add_to_playlist_clicked();
    void cover_clicked();

private slots:
    void on_play_pause_clicked();

private:
    creeper::ThemeManager& m_theme_manager;

    creeper::FilledButton* m_cover_button;
    creeper::Text* m_title_text;
    creeper::Text* m_artist_text;
    creeper::Text* m_album_text;
    creeper::IconButton* m_play_pause_button;
    creeper::IconButton* m_previous_button;
    creeper::IconButton* m_next_button;
    creeper::IconButton* m_order_button;
    creeper::IconButton* m_favorite_button;
    creeper::IconButton* m_add_button;
    creeper::Slider* m_progress_slider;
    creeper::Slider* m_volume_slider;
    bool m_playing;

    auto build_content() -> creeper::Widget*;
    void apply_theme();
    void update_play_pause_button();
};
