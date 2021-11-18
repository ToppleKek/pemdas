#ifndef PEMDAS_TOOLWIDGETLABEL_HPP
#define PEMDAS_TOOLWIDGETLABEL_HPP

#include <QLabel>
#include <QEvent>
#include <QHoverEvent>
#include <QHelpEvent>
#include <QCursor>
#include <QPropertyAnimation>
#include <memory>

class toolwidgetlabel : public QLabel {
    Q_OBJECT
public:
    explicit toolwidgetlabel(QWidget *parent = nullptr) : QLabel(parent) {}

    void set_tool_widget(std::unique_ptr<QWidget> tool_widget) {
        m_tool_widget = std::move(tool_widget);

        if (m_tool_widget)
            m_tool_widget->hide();
    }

    bool event(QEvent *event) override {
        if (event->type() == QEvent::Enter) {
            if (m_open)
                return true;

            m_open = true;

            QPoint point = mapTo(window(), pos());
            point.setY(point.y() - geometry().height() - m_tool_widget->height());
            point.setX((window()->width() / 2) - (m_tool_widget->width() / 2));

            m_tool_widget->move(point);
            m_tool_widget->show();

            return true;
        } else if (event->type() == QEvent::Leave) {
            m_open = false;
            m_tool_widget->hide();
            return true;
        } else {
            return QLabel::event(event);
        }
    }

private:
    std::unique_ptr<QWidget> m_tool_widget;
    bool m_open{false};
};


#endif//PEMDAS_TOOLWIDGETLABEL_HPP
