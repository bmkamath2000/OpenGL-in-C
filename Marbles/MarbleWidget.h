#ifndef MARBLEWIDGET_H
#define MARBLEWIDGET_H

#include <QOpenGLWidget>
#include <QMouseEvent>

class MarbleWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit MarbleWidget(QWidget *parent = nullptr);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int  marble[9][9];
    int  clickStage;   // 0 = awaiting first click, 1 = awaiting destination
    int  firstX, firstY;
    int  marblesLeft;

    void resetBoard();
    int  countMarbles();
    bool hasValidMoves();

    void drawFilledCircle(float cx, float cy, float r, int segments);
    void drawBoard();
};

#endif // MARBLEWIDGET_H
