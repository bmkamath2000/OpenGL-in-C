#include <QApplication>
#include <QSurfaceFormat>
#include "ClientWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setVersion(2, 1);
    QSurfaceFormat::setDefaultFormat(fmt);

    std::string host     = (argc > 1) ? argv[1] : "localhost";
    int         port     = (argc > 2) ? std::stoi(argv[2]) : 5672;
    std::string user     = (argc > 3) ? argv[3] : "guest";
    std::string password = (argc > 4) ? argv[4] : "guest";

    ClientWidget w(host, port, user, password);
    w.setWindowTitle("Pacman Viewer");
    w.show();

    return app.exec();
}
