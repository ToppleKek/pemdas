#include <iostream>
#include <QApplication>
#include <mpd/client.h>
#include "pemdas.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    pemdas window;
    window.show();

    return app.exec();
}
