#include <QApplication>
#include <QSurfaceFormat>
#include "MarbleWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setVersion(2, 1);
    QSurfaceFormat::setDefaultFormat(fmt);

    MarbleWidget w;
    w.setWindowTitle("Marble Solitaire (Brainvita)");
    w.resize(600, 650);
    w.show();

    return app.exec();
}
