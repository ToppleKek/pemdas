#ifndef PEMDAS_STATUSBAR_HPP
#define PEMDAS_STATUSBAR_HPP

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QByteArray>
#include <QBuffer>
#include "player.hpp"
#include "utils.hpp"

#include "ui_pemdas.h"

class statusbar : public QObject {
    Q_OBJECT

public:
    statusbar(Ui::pemdas &t_ui, mpd::player &t_player);

    void setup_ui();
    void update();
    void update_elapsed();
    void update_elapsed(unsigned int elapsed);
    void update_duration(unsigned int duration);
    void update_tooltip(const mpd::song &s, const QString &path);

private slots:
    void player_update();
    void mixer_update();
    void play_pause_button_clicked();
    void next_button_clicked();
    void prev_button_clicked();
    void volume_slider_value_changed(int value);
    void seek_slider_value_changed(int value);

private:
    Ui::pemdas &m_ui;
    mpd::player &m_player;
    QTimer m_timer;
    unsigned int m_current_elapsed = 0;
    unsigned int m_current_song_id = 0;
};

#endif //PEMDAS_STATUSBAR_HPP