// robowidget.h
#pragma once
#include <QOpenGLWidget>
#include <QTimer>
#include <QKeyEvent>
#include "robo.h"
class RoboWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit RoboWidget(QWidget *parent = nullptr);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
private slots:
    void animate();

private:
    QTimer *m_timer;
};
