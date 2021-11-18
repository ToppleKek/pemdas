#ifndef PEMDAS_SETTINGSPAGE_HPP
#define PEMDAS_SETTINGSPAGE_HPP

#include <QSettings>

namespace Ui {
    class pemdas;
}
class settingspage : public QObject {
public:
    settingspage(Ui::pemdas &t_ui, QSettings &t_settings);

signals:
    void settings_updated();
    
private:
    Ui::pemdas &m_ui;
    QSettings &m_settings;
};

#endif//PEMDAS_SETTINGSPAGE_HPP
