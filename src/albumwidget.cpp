#include "albumwidget.hpp"

#include "utils.hpp"

albumwidget::albumwidget(const mpd::song &song, const QString &song_path, QWidget *parent) : QWidget(parent) {
    m_ui.setupUi(this);
    m_ui.title_label->setText(QString::fromStdString(song.title()));
    m_ui.artist_label->setText(QString::fromStdString("By: " + song.artist()));
    m_ui.album_label->setText(QString::fromStdString("On: " + song.album()));
    m_ui.cover_label->setText("<img style='float: left' width=128 height=128 src='data:image/png;base64,\"" + utils::get_cover(song_path) + "\"'>");
}
