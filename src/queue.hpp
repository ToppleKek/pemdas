#ifndef PEMDAS_QUEUE_HPP
#define PEMDAS_QUEUE_HPP

#include <QObject>
#include <QMenu>

#include "mpd.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "ui_pemdas.h"

class queue : public QObject {
    Q_OBJECT

public:
    queue(Ui::pemdas &t_ui, mpd::player &t_player);

    void setup_ui();
    void update();

private slots:
    void player_update();
    void queue_update();
    void item_double_clicked();
    void context_menu(const QPoint &pos);

private:
    Ui::pemdas &m_ui;
    mpd::player &m_player;
    int m_current_index = 0;
};

#endif//PEMDAS_QUEUE_HPP
