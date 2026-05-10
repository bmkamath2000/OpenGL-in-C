#include <QApplication>
#include <QSurfaceFormat>
#include "PacmanWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Request a compatibility profile so legacy OpenGL (glBegin/glEnd,
    // display lists, fixed-function lighting) works on Qt 6
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setVersion(2, 1);   // OpenGL 2.1 compat — enough for fixed-function
    QSurfaceFormat::setDefaultFormat(fmt);

    PacmanWidget w;
    w.setWindowTitle("Pac GL 3D - Qt Port");
    w.resize(1200, 780);
    w.show();

    return app.exec();
}
