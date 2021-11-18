#ifndef PEMDAS_SIDEBAR_HPP
#define PEMDAS_SIDEBAR_HPP

#include <QObject>
#include <QListWidget>
#include <vector>

#include "ui_pemdas.h"
#include "player.hpp"

#define SELECTED_STYLE "QListWidget::item:selected {background-color: rgba(0,0,0,0.3);} QListWidget::item {background-color: rgba(0,0,0,0.3);}"
class sidebar : public QObject {
    Q_OBJECT

public:
    sidebar(Ui::pemdas &t_ui, mpd::player &t_player);

    void setup_ui();
    void update();

private slots:
    void playlist_update();
    void current_item_changed(QListWidgetItem *current, QListWidgetItem *previous);
    void selection_changed();

signals:
    void playlist_selected(QString playlist);

private:
    Ui::pemdas &m_ui;
    mpd::player &m_player;
    int m_num_playlists = 0;
};

#endif//PEMDAS_SIDEBAR_HPP
