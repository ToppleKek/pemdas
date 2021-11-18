#include "statusbar.hpp"

#include <QFontDatabase>
#include "albumwidget.hpp"

//#define PLAY_ICON "\u25B6"
#define PLAY_ICON ""
//#define PAUSE_ICON "\u23F8"
#define PAUSE_ICON ""

statusbar::statusbar(Ui::pemdas &t_ui, mpd::player &t_player) : m_ui(t_ui), m_player(t_player) {
    connect(&m_player, &mpd::player::player_update, this, &statusbar::player_update, Qt::QueuedConnection);
    connect(&m_player, &mpd::player::mixer_update, this, &statusbar::mixer_update, Qt::QueuedConnection);
    connect(&m_timer, &QTimer::timeout, this, QOverload<>::of(&statusbar::update_elapsed), Qt::QueuedConnection);
}

void statusbar::setup_ui() {
    int id = QFontDatabase::addApplicationFont(":/fonts/icons.ttf");
    QFont f(QFontDatabase::applicationFontFamilies(id).at(0));
    f.setPointSize(16);

    m_ui.playPauseButton->setFont(f);
    m_ui.nextButton->setFont(f);
    m_ui.prevButton->setFont(f);

    connect(m_ui.playPauseButton, &QPushButton::clicked, this, &statusbar::play_pause_button_clicked, Qt::QueuedConnection);
    connect(m_ui.nextButton, &QPushButton::clicked, this, &statusbar::next_button_clicked, Qt::QueuedConnection);
    connect(m_ui.prevButton, &QPushButton::clicked, this, &statusbar::prev_button_clicked, Qt::QueuedConnection);
    connect(m_ui.volume_slider, &QSlider::valueChanged, this, &statusbar::volume_slider_value_changed, Qt::QueuedConnection);
    connect(m_ui.seek_slider, &QSlider::valueChanged, this, &statusbar::seek_slider_value_changed, Qt::QueuedConnection);
}

void statusbar::update() {
    player_update();
    mixer_update();
}

void statusbar::update_elapsed() {
    update_elapsed(m_current_elapsed++);
}

void statusbar::update_elapsed(unsigned int elapsed) {
    m_ui.elapsed_label->setText(utils::s_to_mmss(elapsed));
    m_ui.seek_slider->blockSignals(true);
    m_ui.seek_slider->setValue(elapsed);
    m_ui.seek_slider->blockSignals(false);
}

void statusbar::update_duration(unsigned int duration) {
    m_ui.duration_label->setText(utils::s_to_mmss(duration));
    m_ui.seek_slider->blockSignals(true);
    m_ui.seek_slider->setMaximum(duration);
    m_ui.seek_slider->blockSignals(false);
}

void statusbar::update_tooltip(const mpd::song &s, const QString &path) {
    //<h3 style='text-align: center'>Now Playing</h3> align='left'style='vertical-align:middle'style='display: flex; align-items: center;'
//    m_ui.song_info_label->setToolTip(QString("<div><img style='float: left' width=128 height=128 src='data:image/png;base64," +
//                                     utils::get_cover(path) + "'><span><p>") +
//                                     QString::fromStdString(s.title()) +
//                                     "</p><p><i>By</i> " + QString::fromStdString(s.artist()) +
//                                     "</p><p><i>On</i> " + QString::fromStdString(s.album()) +
//                                     "</p></span></div>");
    m_ui.song_info_label->set_tool_widget(std::make_unique<albumwidget>(s, path, m_ui.centralwidget));
}

void statusbar::player_update() {
    mpd::status status = m_player.status();
    mpd_state state = status.state();

    update_elapsed(status.elapsed_time());
    update_duration(status.total_time());

    switch (state) {
        case MPD_STATE_PLAY:
            m_current_elapsed = status.elapsed_time();
            update_elapsed();
            m_timer.start(1000);
            m_ui.playPauseButton->setText(PAUSE_ICON);
            break;
        case MPD_STATE_PAUSE:
            m_timer.stop();
            m_ui.playPauseButton->setText(PLAY_ICON);
            break;
        case MPD_STATE_STOP:
            m_timer.stop();
            m_ui.song_info_label->setText("Not Playing");
            m_ui.song_info_label->set_tool_widget(std::make_unique<QLabel>("Not Playing", m_ui.centralwidget));
            m_ui.playPauseButton->setText(PLAY_ICON);
            return;
        case MPD_STATE_UNKNOWN:
            m_timer.stop();
            m_ui.playPauseButton->setText(PLAY_ICON);
            m_ui.playPauseButton->setEnabled(false);
    }

    mpd::song s = m_player.current_song();

    if (s.id() != m_current_song_id) {
        m_current_song_id = s.id();
        update_tooltip(s, QString::fromStdString(m_player.music_dir() + "/" + s.path()));
    }

    QString title = QString::fromStdString([&]() -> std::string {
        if (s.title().empty())
            return "Unknown";

        return s.title();
    }());

    QString artist = QString::fromStdString([&]() -> std::string {
        if (s.artist().empty())
            return "Unknown";

        return s.artist();
    }());

    m_ui.song_info_label->setText(title + " - " + artist);
}

void statusbar::mixer_update() {
    int vol = m_player.status().volume();

    if (vol == -1) {
        m_ui.volume_slider->setValue(0);
        m_ui.volume_label->setText("Vol. N/A");
        return;
    }

    m_ui.volume_slider->blockSignals(true);
    m_ui.volume_slider->setValue(vol);
    m_ui.volume_slider->blockSignals(false);
    m_ui.volume_label->setText("Vol. " + QString::number(vol) + "%");
}

void statusbar::play_pause_button_clicked() {
    m_player.play_pause();
}

void statusbar::next_button_clicked() {
    m_player.next();
}

void statusbar::prev_button_clicked() {
    m_player.prev();
}

void statusbar::volume_slider_value_changed(int value) {
    m_player.set_volume(value);
}

void statusbar::seek_slider_value_changed(int value) {
    m_player.seek(value);
}
