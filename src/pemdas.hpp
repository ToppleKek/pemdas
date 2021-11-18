#ifndef PEMDAS_PEMDAS_H
#define PEMDAS_PEMDAS_H

#include "musicpage.hpp"
#include "player.hpp"
#include "statusbar.hpp"
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QtGlobal>
#include <QSettings>
#include "discord/discordrpc.hpp"
#include <memory>
#include <unistd.h>

#include "playlistpage.hpp"
#include "queue.hpp"
#include "sidebar.hpp"
#include "ui_pemdas.h"

class pemdas : public QMainWindow {
    Q_OBJECT

public:
    explicit pemdas(QMainWindow *parent = nullptr);

private slots:
    void player_update();
    void mixer_update();
    void poll_mpd_fd();

private:
    Ui::pemdas m_ui{};
    mpd::player m_player;
    QSettings m_settings;
    QTimer m_poll_timer;
    statusbar m_statusbar;
    musicpage m_musicpage;
    queue m_queue;
    sidebar m_sidebar;
    playlistpage m_playlistpage;
    discordrpc m_discordrpc;
};

#endif //PEMDAS_PEMDAS_H
