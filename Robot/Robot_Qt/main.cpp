#include <QApplication>
#include <QMainWindow>
#include "robowidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow win;
    auto *robo = new RoboWidget(&win);
    win.setCentralWidget(robo);
    win.resize(1000, 1000);
    win.setWindowTitle("Vulcan Gunner — Qt");
    win.show();

    return app.exec();
}
