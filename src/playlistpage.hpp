#ifndef PEMDAS_PLAYLISTPAGE_HPP
#define PEMDAS_PLAYLISTPAGE_HPP

#include <QObject>

#include "player.hpp"
#include "mpd.hpp"
#include "sidebar.hpp"
#include "utils.hpp"
#include "ui_pemdas.h"

class playlistpage : public QObject {
    Q_OBJECT

public:
    playlistpage(Ui::pemdas &t_ui, mpd::player &t_player, sidebar &t_sidebar);

    void setup_ui();

private slots:
    void tree_item_double_clicked(QTreeWidgetItem *item, int);
    void context_menu(const QPoint &pos);
    void load_playlist(const QString& playlist);

private:
    Ui::pemdas &m_ui;
    mpd::player &m_player;
    sidebar &m_sidebar;
    QString m_current_playlist = "";
};

#endif//PEMDAS_PLAYLISTPAGE_HPP
