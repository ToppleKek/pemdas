#include "pemdas.hpp"

#include "albumwidget.hpp"

pemdas::pemdas(QMainWindow *parent) :
    QMainWindow(parent),
    m_player(),
    m_statusbar(m_ui, m_player),
    m_musicpage(m_ui, m_player),
    m_queue(m_ui, m_player),
    m_sidebar(m_ui, m_player),
    m_playlistpage(m_ui, m_player, m_sidebar),
    m_settings(QSettings::UserScope, "ToppleKek", "pemdas"),
    m_discordrpc(m_player, discordapp{m_settings.value("discord/appid").toString(), m_settings.value("discord/token").toString(), true}) {
    m_ui.setupUi(this);

    m_settings.setValue("discord/appid", "767837079803134002");
    m_settings.setValue("discord/token", "");
	
	qDebug() << "pemdas: settings location:" << m_settings.fileName();

    m_statusbar.setup_ui();
    m_musicpage.setup_ui();
    m_queue.setup_ui();
    m_sidebar.setup_ui();
    m_playlistpage.setup_ui();

    m_ui.song_info_label->set_tool_widget(std::make_unique<QLabel>("Not Playing", m_ui.centralwidget));
    m_ui.tab_list_widget->setCurrentRow(1);
    m_ui.tab_queue_library_splitter->setSizes(QList<int>() << 130 << 980 << 150);
    m_player.connect();
    m_player.update();
    m_statusbar.update();
    m_musicpage.update();
    m_queue.update();
    m_sidebar.update();
    connect(&m_poll_timer, &QTimer::timeout, this, &pemdas::poll_mpd_fd, Qt::QueuedConnection);
    connect(&m_player, &mpd::player::player_update, this, &pemdas::player_update, Qt::QueuedConnection);
    connect(&m_player, &mpd::player::mixer_update, this, &pemdas::mixer_update, Qt::QueuedConnection);

    m_poll_timer.start();
}

void pemdas::player_update() {
    qDebug() << "pemdas: player_upate:" << m_player.status().current_song_id();
}

void pemdas::mixer_update() {
    qDebug() << "pemdas: mixer_update:" << m_player.status().volume();
}

void pemdas::poll_mpd_fd() {
    fd_set set;
    struct timeval tv = {0, 1000};
    FD_ZERO(&set);
    FD_SET(m_player.fd(), &set);

    int result = select(m_player.fd() + 1, &set, nullptr, nullptr, &tv);

    if (result > 0 && FD_ISSET(m_player.fd(), &set))
        m_player.handle_events();
}