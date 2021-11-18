#ifndef PEMDAS_MUSICPAGE_HPP
#define PEMDAS_MUSICPAGE_HPP

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <vector>

#include "utils.hpp"
#include "player.hpp"
#include "ui_pemdas.h"

class musicpage : public QObject {
    Q_OBJECT

public:
    musicpage(Ui::pemdas &t_ui, mpd::player &t_player);

    void setup_ui();
    void update();

private slots:
    void tree_item_double_clicked(QTreeWidgetItem *item, int column);
    void context_menu(const QPoint &pos);

private:
    Ui::pemdas &m_ui;
    mpd::player &m_player;
};

#endif//PEMDAS_MUSICPAGE_HPP
