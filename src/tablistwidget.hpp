#ifndef PEMDAS_TABLISTWIDGET_HPP
#define PEMDAS_TABLISTWIDGET_HPP

#include <QListWidget>
#include <QDebug>

class tablistwidget : public QListWidget {
    Q_OBJECT
public:
    explicit tablistwidget(QWidget *parent = nullptr) : QListWidget(parent) {}

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override {
        if (selected.isEmpty())
            return;

        QListWidget::selectionChanged(selected, deselected);
    }
};

#endif//PEMDAS_TABLISTWIDGET_HPP
