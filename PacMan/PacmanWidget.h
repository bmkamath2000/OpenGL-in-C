#ifndef PACMANWIDGET_H
#define PACMANWIDGET_H

#include <QOpenGLWidget>
#include <QTimer>
#include <QKeyEvent>
#include "Ghost.h"
#include "Constants.h"

class PacmanWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit PacmanWidget(QWidget *parent = nullptr);
    ~PacmanWidget();

protected:
    // QOpenGLWidget interface
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    // ---- game state ----
    int  board_array[BOARD_X][BOARD_Y];
    int  pebble_array[BOARD_X][BOARD_Y];
    int  tp_array[BOARD_X][BOARD_Y];

    int  pebbles_left;
    double speed1;
    double angle1;
    double pac_x, pac_y;   // pacman position (was 'a','b')
    bool animate;
    int  lives;
    int  points;
    int  num_ghosts;
    int  start_timer;
    bool gameover;
    Qt::Key currentKey;
    bool keyHeld;

    Ghost *ghost[4];

    // display lists
    GLuint list[5];

    // ---- helpers ----
    void tp_restore();
    void P_Reinit();
    void G_Reinit();
    bool Open(int a, int b);
    void MovePac();
    void DrawScene();
    void DrawPac();
    void create_list_lib();
    void drawSphere(double radius, int slices, int stacks);
    void renderText(float x, float y, const char *str, float r, float g, float b);

    QTimer *timer;
};

#endif // PACMANWIDGET_H
