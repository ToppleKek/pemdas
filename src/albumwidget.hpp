#ifndef PEMDAS_ALBUMWIDGET_HPP
#define PEMDAS_ALBUMWIDGET_HPP

#include <QWidget>
#include <ui_albumwidget.h>
#include "mpd.hpp"

class albumwidget : public QWidget {
    Q_OBJECT
public:
    albumwidget(const mpd::song &song, const QString &song_path, QWidget *parent = nullptr);

private:
    Ui::albumwidget m_ui{};
};

#endif//PEMDAS_ALBUMWIDGET_HPP
