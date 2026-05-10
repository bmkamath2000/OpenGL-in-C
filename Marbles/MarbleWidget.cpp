#include "MarbleWidget.h"
#include <QPainter>
#include <QFont>
#include <math.h>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initial board layout (1 = marble, 0 = empty hole, -1 = not a valid cell)
static const int BOARD_INIT[9][9] = {
    {-1,-1,-1, 1, 1, 1,-1,-1,-1},
    {-1,-1,-1, 1, 1, 1,-1,-1,-1},
    {-1,-1,-1, 1, 1, 1,-1,-1,-1},
    { 1, 1, 1, 1, 1, 1, 1, 1, 1},
    { 1, 1, 1, 1, 0, 1, 1, 1, 1},  // centre empty
    { 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {-1,-1,-1, 1, 1, 1,-1,-1,-1},
    {-1,-1,-1, 1, 1, 1,-1,-1,-1},
    {-1,-1,-1, 1, 1, 1,-1,-1,-1}
};

MarbleWidget::MarbleWidget(QWidget *parent)
    : QOpenGLWidget(parent), clickStage(0), firstX(-1), firstY(-1)
{
    resetBoard();
    setMinimumSize(600, 650);  // extra height for HUD text
}

void MarbleWidget::resetBoard()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            marble[i][j] = BOARD_INIT[i][j];
    clickStage = 0;
    firstX = firstY = -1;
    marblesLeft = countMarbles();
}

int MarbleWidget::countMarbles()
{
    int c = 0;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (marble[i][j] == 1) c++;
    return c;
}

bool MarbleWidget::hasValidMoves()
{
    int dx[] = {0, 0, 2, -2};
    int dy[] = {2, -2, 0,  0};
    int mx[] = {0, 0,  1,  -1};
    int my[] = {1, -1, 0,  0};

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (marble[i][j] != 1) continue;
            for (int d = 0; d < 4; d++) {
                int ni = i + dx[d], nj = j + dy[d];
                int mi = i + mx[d], mj = j + my[d];
                if (ni < 0 || ni > 8 || nj < 0 || nj > 8) continue;
                if (marble[ni][nj] == 0 && marble[mi][mj] == 1)
                    return true;
            }
        }
    }
    return false;
}

// ── OpenGL ────────────────────────────────────────────────────────────────────

void MarbleWidget::initializeGL()
{
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
}

void MarbleWidget::resizeGL(int w, int h)
{
    // Reserve bottom 50px for HUD; board fills the rest square
    int boardPx = qMin(w, h - 50);
    if (boardPx < 1) boardPx = 1;
    glViewport(0, 0, boardPx, boardPx);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 18.0, 0.0, 18.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void MarbleWidget::drawFilledCircle(float cx, float cy, float r, int segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * (float)M_PI * (float)i / (float)segments;
        glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
    }
    glEnd();
}

void MarbleWidget::drawBoard()
{
    // Board background
    glColor3f(0.55f, 0.35f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(18, 0);
    glVertex2f(18, 18); glVertex2f(0, 18);
    glEnd();

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (marble[i][j] == -1) continue;  // not part of cross

            float cx = i * 2.0f + 1.0f;
            float cy = j * 2.0f + 1.0f;

            // Hole
            glColor3f(0.2f, 0.1f, 0.05f);
            drawFilledCircle(cx, cy, 0.52f, 32);
            glColor3f(0.85f, 0.85f, 0.85f);
            drawFilledCircle(cx, cy, 0.45f, 32);

            if (marble[i][j] == 1) {
                // Marble body
                bool selected = (clickStage == 1 && i == firstX && j == firstY);
                if (selected) {
                    glColor3f(1.0f, 0.85f, 0.0f);   // gold when selected
                } else {
                    glColor3f(0.1f, 0.25f, 0.85f);  // blue marble
                }
                drawFilledCircle(cx, cy, 0.68f, 40);

                // Shading ring
                if (selected)
                    glColor3f(0.9f, 0.6f, 0.0f);
                else
                    glColor3f(0.05f, 0.1f, 0.55f);
                drawFilledCircle(cx + 0.12f, cy - 0.12f, 0.55f, 40);

                // Highlight
                if (selected)
                    glColor3f(1.0f, 1.0f, 0.7f);
                else
                    glColor3f(0.6f, 0.75f, 1.0f);
                drawFilledCircle(cx - 0.22f, cy + 0.22f, 0.18f, 20);
            }
        }
    }
}

void MarbleWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Recalculate viewport each frame so resize is always correct
    int w = width(), h = height();
    int boardPx = qMin(w, h - 50);
    if (boardPx < 1) boardPx = 1;
    glViewport(0, 0, boardPx, boardPx);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 18.0, 0.0, 18.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawBoard();

    // ── HUD overlay via QPainter ──────────────────────────────────────────────
    marblesLeft = countMarbles();
    bool gameWon  = (marblesLeft == 1);
    bool gameLost = (!gameWon && !hasValidMoves());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // HUD bar background
    painter.fillRect(0, 0, w, 50, QColor(40, 25, 10));

    QFont font("Arial", 13, QFont::Bold);
    painter.setFont(font);
    painter.setPen(QColor(255, 220, 100));
    painter.drawText(15, 22, QString("Marbles left: %1").arg(marblesLeft));

    if (clickStage == 1)
        painter.drawText(15, 42, "Click destination (2 steps away, jump over a marble)");
    else if (gameWon) {
        painter.setPen(QColor(50, 255, 80));
        QFont bigFont("Arial", 18, QFont::Bold);
        painter.setFont(bigFont);
        painter.drawText(QRect(0, 5, w, 40), Qt::AlignCenter, "🎉  You Win!  Click to restart");
    } else if (gameLost) {
        painter.setPen(QColor(255, 80, 60));
        QFont bigFont("Arial", 18, QFont::Bold);
        painter.setFont(bigFont);
        painter.drawText(QRect(0, 5, w, 40), Qt::AlignCenter, "No moves left!  Click to restart");
    } else {
        painter.drawText(15, 42, "Click a marble to select it");
    }

    painter.end();
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

void MarbleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    marblesLeft = countMarbles();
    bool gameWon  = (marblesLeft == 1);
    bool gameLost = (!gameWon && !hasValidMoves());

    // Restart on click when game is over
    if (gameWon || gameLost) {
        resetBoard();
        update();
        return;
    }

    // Map pixel → board cell
    int w = width(), h = height();
    int boardPx = qMin(w, h - 50);
    if (boardPx < 1) return;

    // Board viewport starts at y=50 (bottom in Qt coords = h-50 from top)
    int px = event->pos().x() + 50;
    //int py = event->pos().y() - 50;        // subtract HUD height
    int py = event->pos().y() -  100;        // subtract HUD height
    int boardH = boardPx;

    if (px < 0 || px >= boardPx || py < 0 || py >= boardH) return;

    float bx = (float)px / (float)boardPx * 18.0f;
    float by = (float)(boardH - py) / (float)boardH * 18.0f;  // flip Y

    int i = (int)(bx / 2.0f);
    int j = (int)(by / 2.0f);

    if (i < 0 || i > 8 || j < 0 || j > 8) return;
    if (marble[i][j] == -1) return;  // outside cross

    if (clickStage == 0) {
        if (marble[i][j] == 1) {
            firstX = i; firstY = j;
            clickStage = 1;
        }
    } else {
        bool moved = false;

        if (marble[i][j] == 0) {
            // Vertical jump
            if (i == firstX && abs(firstY - j) == 2) {
                int mid = (firstY + j) / 2;
                if (marble[i][mid] == 1) {
                    marble[firstX][firstY] = 0;
                    marble[i][mid]         = 0;
                    marble[i][j]           = 1;
                    moved = true;
                }
            }
            // Horizontal jump
            else if (j == firstY && abs(firstX - i) == 2) {
                int mid = (firstX + i) / 2;
                if (marble[mid][j] == 1) {
                    marble[firstX][firstY] = 0;
                    marble[mid][j]         = 0;
                    marble[i][j]           = 1;
                    moved = true;
                }
            }
        }

        // If clicked another marble instead, re-select it
        if (!moved && marble[i][j] == 1) {
            firstX = i; firstY = j;
            // keep clickStage = 1
        } else {
            clickStage = 0;
        }
    }

    update();
}
