// robowidget.cpp
#include "robowidget.h"
#include <GL/glu.h> // gluPerspective etc.
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
// Pull in ALL your existing globals and functions from robo.c
// (rename to robo.cpp, keep everything except main() and GLUT callbacks)
extern void myinit();
extern void
display_scene(); // your display() logic, minus glFlush/glutSwapBuffers
extern void myReshape(int w, int h);
extern void animation();

RoboWidget::RoboWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RoboWidget::animate);
}

void RoboWidget::initializeGL() {
    myinit(); // your existing init — sets up display lists, materials
}

void RoboWidget::resizeGL(int w, int h) {
    myReshape(w, h); // your existing reshape — sets projection matrix
}

void RoboWidget::paintGL() {
    display_scene(); // your existing display() body
    // QOpenGLWidget handles buffer swap automatically — remove glutSwapBuffers
}

void RoboWidget::animate() {
    animation(); // your existing walk animation stepper
    update();    // triggers paintGL — replaces glutPostRedisplay
}

void RoboWidget::keyPressEvent(QKeyEvent *e) {
    // Map Qt keys → your existing functions
    bool isShift = (e->modifiers() & Qt::ShiftModifier);

    switch (e->key()) {
    // Shoulder 1 & 2 (q, a, w, s)
    case Qt::Key_Q:
        isShift ? elbow2Subtract() : shoulder2Subtract();
        break;
    case Qt::Key_A:
        isShift ? elbow2Add() : shoulder2Add();
        break;
    case Qt::Key_W:
        isShift ? elbow1Subtract() : shoulder1Subtract();
        break;
    case Qt::Key_S:
        isShift ? elbow1Add() : shoulder1Add();
        break;

    // Shoulder 3 & 4 (Numbers 1-4)
    case Qt::Key_1:
        shoulder4Add();
        break;
    case Qt::Key_2:
        shoulder3Add();
        break;
    case Qt::Key_3:
        shoulder4Subtract();
        break;
    case Qt::Key_4:
        shoulder3Subtract();
        break;

    // Lateral Control (z, x)
    case Qt::Key_Z:
        isShift ? lat2Lower() : lat2Raise();
        break;
    case Qt::Key_X:
        isShift ? lat1Lower() : lat1Raise();
        break;

    // Torso Control
    case Qt::Key_D:
        RotateAdd();
        break;

    // System Control
    case Qt::Key_Escape:
        qApp->quit();
        break;

    default:
        QWidget::keyPressEvent(e);
        return;
    }

    update(); // Triggers a repaint of the widget
}
void RoboWidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Start Walk", [this] { m_timer->start(16); });
        menu.addAction("Stop Walk", [this] { m_timer->stop(); });
        menu.addAction("Toggle Wire", [] { Toggle(); });
        menu.addAction("Quit", qApp, &QApplication::quit);
        menu.exec(QCursor::pos());
    }
}
