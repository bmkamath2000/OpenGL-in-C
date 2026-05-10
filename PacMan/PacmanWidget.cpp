#include "PacmanWidget.h"
#include <math.h>
#include <QPainter>
#include <string.h>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

// ─── Board data ──────────────────────────────────────────────────────────────

static const int BOARD_INIT[BOARD_X][BOARD_Y] = {
    {8,5,5,5,5,5,5,5,5,5,5,5,5,1,1,5,5,5,5,5,5,5,5,5,5,5,5,7},
    {6,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {6,0,8,1,1,7,0,8,1,1,1,7,0,2,4,0,8,1,1,1,7,0,8,1,1,7,0,6},
    {6,0,2,11,11,4,0,2,11,11,11,4,0,2,4,0,2,11,11,11,4,0,2,11,11,4,0,6},
    {6,0,9,3,3,10,0,9,3,3,3,10,0,9,10,0,9,3,3,3,10,0,9,3,3,10,0,6},
    {6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {6,0,8,1,1,7,0,8,7,0,8,1,1,1,1,1,1,7,0,8,7,0,8,1,1,7,0,6},
    {6,0,9,3,3,10,0,2,4,0,9,3,3,11,11,3,3,10,0,2,4,0,9,3,3,10,0,6},
    {6,0,0,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,0,0,6},
    {9,5,5,5,5,7,0,2,11,1,1,7,0,2,4,0,8,1,1,11,4,0,8,5,5,5,5,10},
    {0,0,0,0,0,6,0,2,11,3,3,10,0,9,10,0,9,3,3,11,4,0,6,0,0,0,0,0},
    {0,0,0,0,0,6,0,2,4,0,0,0,0,0,0,0,0,0,0,2,4,0,6,0,0,0,0,0},
    {0,0,0,0,0,6,0,2,4,0,8,5,5,1,1,5,5,7,0,2,4,0,6,0,0,0,0,0},
    {5,5,5,5,5,10,0,9,10,0,6,0,0,0,0,0,0,6,0,9,10,0,9,5,5,5,5,5},
    {0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0},
    {5,5,5,5,5,7,0,8,7,0,6,0,0,0,0,0,0,6,0,8,7,0,8,5,5,5,5,5},
    {0,0,0,0,0,6,0,2,4,0,9,5,5,5,5,5,5,10,0,2,4,0,6,0,0,0,0,0},
    {0,0,0,0,0,6,0,2,4,0,0,0,0,0,0,0,0,0,0,2,4,0,6,0,0,0,0,0},
    {0,0,0,0,0,6,0,2,4,0,8,1,1,1,1,1,1,7,0,2,4,0,6,0,0,0,0,0},
    {8,5,5,5,5,10,0,9,10,0,9,3,3,11,11,3,3,10,0,9,10,0,9,5,5,5,5,7},
    {6,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {6,0,8,1,1,7,0,8,1,1,1,7,0,2,4,0,8,1,1,1,7,0,8,1,1,7,0,6},
    {6,0,9,3,11,4,0,9,3,3,3,10,0,9,10,0,9,3,3,3,10,0,2,11,3,10,0,6},
    {6,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,6},
    {2,1,7,0,2,4,0,8,7,0,8,1,1,1,1,1,1,7,0,8,7,0,2,4,0,8,1,4},
    {2,3,10,0,9,10,0,2,4,0,9,3,3,11,11,3,3,10,0,2,4,0,9,10,0,9,3,4},
    {6,0,0,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,0,0,6},
    {6,0,8,1,1,1,1,11,11,1,1,7,0,2,4,0,8,1,1,11,11,1,1,1,1,7,0,6},
    {6,0,9,3,3,3,3,3,3,3,3,10,0,9,10,0,9,3,3,3,3,3,3,3,3,10,0,6},
    {6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {9,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,10}
};

static const int PEBBLE_INIT[BOARD_X][BOARD_Y] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,3,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,3,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,3,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,3,0},
    {0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0},
    {0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0},
    {0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// ─── Constructor / Destructor ─────────────────────────────────────────────────

PacmanWidget::PacmanWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    memcpy(board_array,  BOARD_INIT,  sizeof(board_array));
    memcpy(pebble_array, PEBBLE_INIT, sizeof(pebble_array));

    speed1      = 0.1;
    angle1      = 90;
    pac_x       = 13.5;
    pac_y       = 23;
    animate     = false;
    lives       = 3;
    points      = 0;
    num_ghosts  = 4;
    start_timer = 3;
    gameover    = false;
    keyHeld     = false;
    currentKey  = Qt::Key_W;

    memset(list, 0, sizeof(list));

    // Ghost starting positions
    static const int start_x[4] = {11, 12, 15, 16};
    static const float ghost_colors[4][3] = {
        {255,0,0},{120,240,120},{255,200,200},{255,125,0}
    };

    for (int i = 0; i < num_ghosts; i++) {
        ghost[i] = new Ghost(start_x[i], 14);
        ghost[i]->x         = start_x[i];
        ghost[i]->y         = 14;
        ghost[i]->eaten     = false;
        ghost[i]->max_speed = 0.1 - 0.01f * (float)i;
        ghost[i]->speed     = ghost[i]->max_speed;
        for (int j = 0; j < 3; j++)
            ghost[i]->color[j] = ghost_colors[i][j] / 255.0f;
    }

    tp_restore();

    // ~16ms timer ≈ 60 fps game loop
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PacmanWidget::gameLoop);
    timer->start(16);

    setFocusPolicy(Qt::StrongFocus);
}

PacmanWidget::~PacmanWidget()
{
    for (int i = 0; i < num_ghosts; i++)
        delete ghost[i];
}

// ─── OpenGL init ──────────────────────────────────────────────────────────────

void PacmanWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // Lighting
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    float diff[4] = {1.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    float amb[4] = {1.0f, 0.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    create_list_lib();
}

void PacmanWidget::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double aspect = (double)w / (double)h;
    // original used gluPerspective(60, 1.33, 0.005, 100)
    GLUquadric *q = gluNewQuadric(); gluDeleteQuadric(q); // force glu link
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // replicate gluPerspective manually (or use GLU)
    gluPerspective(60.0, aspect, 0.005, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-1.5, 0, 40, -1.5, 0, 0,  0.0, 1.0, 0.0);
}

// ─── Game loop slot ───────────────────────────────────────────────────────────

void PacmanWidget::gameLoop()
{
    update();  // triggers paintGL
}

// ─── Key handling ─────────────────────────────────────────────────────────────

void PacmanWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;

    if (start_timer > 0)
        start_timer--;

    currentKey = (Qt::Key)event->key();
    keyHeld = true;

    switch (event->key()) {
    case Qt::Key_Up:
        if ((int)pac_x - pac_x > -0.1 && (int)angle1 != 270) {
            if (Open((int)pac_x, (int)pac_y - 1)) {
                animate = true;
                angle1 = 270;
            }
        }
        break;
    case Qt::Key_Down:
        if ((int)pac_x - pac_x > -0.1 && (int)angle1 != 90) {
            if (Open((int)pac_x, (int)pac_y + 1)) {
                animate = true;
                angle1 = 90;
            }
        }
        break;
    case Qt::Key_Left:
        if ((int)pac_y - pac_y > -0.1 && (int)angle1 != 180) {
            if (Open((int)pac_x - 1, (int)pac_y)) {
                animate = true;
                angle1 = 180;
            }
        }
        break;
    case Qt::Key_Right:
        if ((int)pac_y - pac_y > -0.1 && (int)angle1 != 0) {
            if (Open((int)pac_x + 1, (int)pac_y)) {
                animate = true;
                angle1 = 0;
            }
        }
        break;
    default:
        break;
    }
}

void PacmanWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    if ((Qt::Key)event->key() == currentKey)
        keyHeld = false;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

bool PacmanWidget::Open(int a, int b)
{
    // bounds check
    if (a < 0 || a >= BOARD_Y || b < 0 || b >= BOARD_X)
        return false;
    return board_array[b][a] == 0;
}

void PacmanWidget::tp_restore()
{
    for (int i = 0; i < BOARD_X; i++)
        for (int j = 0; j < BOARD_Y; j++)
            tp_array[i][j] = pebble_array[i][j];
    pebbles_left = 244;
}

void PacmanWidget::P_Reinit()
{
    pac_x  = 13.5;
    pac_y  = 23;
    angle1 = 90;
    animate = false;
}

void PacmanWidget::G_Reinit()
{
    start_timer = 3;
    static const int start_x[4] = {11, 12, 15, 16};
    static const float ghost_colors[4][3] = {
        {255,0,0},{120,240,120},{255,200,200},{255,125,0}
    };
    for (int i = 0; i < num_ghosts; i++) {
        ghost[i]->Reinit();
        ghost[i]->x         = start_x[i];
        ghost[i]->y         = 14;
        ghost[i]->eaten     = false;
        ghost[i]->jail_timer = i * 33 + 66;
        ghost[i]->max_speed = 0.1 - 0.01f * (float)i;
        ghost[i]->speed     = ghost[i]->max_speed;
        for (int j = 0; j < 3; j++)
            ghost[i]->color[j] = ghost_colors[i][j] / 255.0f;
    }
}

void PacmanWidget::drawSphere(double radius, int slices, int stacks)
{
    GLUquadric *q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}

// Render a string in the 3D scene at a given position
// Uses legacy glRasterPos + QPainter-style bitmap characters via Qt text rendering
void PacmanWidget::renderText(float x, float y, const char *str, float r, float g, float b)
{
    // We use Qt's renderText via QPainter overlay trick:
    // Save/restore OpenGL state and draw text using QPainter
    // (QOpenGLWidget supports painter after makeCurrent)
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    // Since we can't use glutBitmapCharacter in Qt, we'll use QPainter below.
    // This function is intentionally left as a marker; actual text is drawn
    // in paintGL via QPainter after the OpenGL scene.
    (void)str;
}

// ─── Pacman movement ──────────────────────────────────────────────────────────

void PacmanWidget::MovePac()
{
    pac_x += speed1 * cos(M_PI / 180.0 * angle1);
    pac_y += speed1 * sin(M_PI / 180.0 * angle1);

    // continuous movement: if key still held, try direction changes
    if (keyHeld) {
        switch (currentKey) {
        case Qt::Key_Up:
            if ((int)pac_x - pac_x > -0.1 && (int)angle1 != 270)
                if (Open((int)pac_x, (int)pac_y - 1)) { animate = true; angle1 = 270; }
            break;
        case Qt::Key_Down:
            if ((int)pac_x - pac_x > -0.1 && (int)angle1 != 90)
                if (Open((int)pac_x, (int)pac_y + 1)) { animate = true; angle1 = 90; }
            break;
        case Qt::Key_Left:
            if ((int)pac_y - pac_y > -0.1 && (int)angle1 != 180)
                if (Open((int)pac_x - 1, (int)pac_y)) { animate = true; angle1 = 180; }
            break;
        case Qt::Key_Right:
            if ((int)pac_y - pac_y > -0.1 && (int)angle1 != 0)
                if (Open((int)pac_x + 1, (int)pac_y)) { animate = true; angle1 = 0; }
            break;
        default: break;
        }
    }
}

// ─── Draw Pacman ─────────────────────────────────────────────────────────────

void PacmanWidget::DrawPac()
{
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef((float)pac_x, -(float)pac_y, 0);
    glTranslatef(0.5f, 0.6f, 0);
    glTranslatef((float)BOARD_X / -2.0f, (float)BOARD_Y / 2.0f, 0.5f);
    drawSphere(0.5, 15, 10);
    glPopMatrix();
}

// ─── Draw board + pebbles ────────────────────────────────────────────────────

void PacmanWidget::DrawScene()
{
    // Draw board in two halves (original logic preserved)
    for (int pass = 0; pass < 2; pass++) {
        int j_start = (pass == 0) ? 0        : BOARD_Y / 2;
        int j_end   = (pass == 0) ? BOARD_Y / 2 : BOARD_Y;
        int j_step  = (pass == 0) ? 1        : -1;
        int j_first = (pass == 0) ? j_start  : j_end - 1;
        int j_last  = (pass == 0) ? j_end    : j_start - 1;

        for (int i = 0; i < BOARD_X; i++) {
            for (int j = j_first; pass == 0 ? j < j_last : j >= j_last; j += j_step) {
                glColor3f(0, 0, 1);
                int call_this = 0;

                glPushMatrix();
                glTranslatef(-(float)BOARD_X / 2.0f, -(float)BOARD_Y / 2.0f, 0);
                glTranslatef((float)j, (float)(BOARD_Y - i), 0);

                glPushMatrix();
                glTranslatef(0.5f, 0.5f, 0);

                switch (board_array[i][j]) {
                case 4: glRotatef(90.0f, 0, 0, 1); // fall through
                case 3: glRotatef(90.0f, 0, 0, 1); // fall through
                case 2: glRotatef(90.0f, 0, 0, 1); // fall through
                case 1: call_this = 1; break;
                case 6: glRotatef(90.0f, 0, 0, 1); // fall through
                case 5: call_this = 2; break;
                case 10: glRotatef(90.0f, 0, 0, 1); // fall through
                case 9:  glRotatef(90.0f, 0, 0, 1); // fall through
                case 8:  glRotatef(90.0f, 0, 0, 1); // fall through
                case 7:  call_this = 3; break;
                }

                glScalef(1, 1, 0.5f);
                glTranslatef(-0.5f, -0.5f, 0);
                glCallList(list[call_this]);
                glPopMatrix();

                if (call_this != 0 || board_array[i][j] == 11) {
                    glTranslatef(0, 0, -0.5f);
                    glCallList(list[4]);
                }
                glPopMatrix();

                // Pebbles
                if (tp_array[i][j] > 0) {
                    glColor3f(0, 1.0f, 1.0f / (float)tp_array[i][j]);
                    glPushMatrix();
                    glTranslatef(-(float)BOARD_X / 2.0f, -(float)BOARD_Y / 2.0f, 0);
                    glTranslatef((float)j, (float)(BOARD_Y - i), 0);
                    glTranslatef(0.5f, 0.5f, 0.5f);
                    drawSphere(0.1f * (float)tp_array[i][j], 6, 6);
                    glPopMatrix();
                }
            }
        }
    }

    DrawPac();
}

// ─── Display lists (maze geometry) ───────────────────────────────────────────

void PacmanWidget::create_list_lib()
{
    list[1] = glGenLists(1);
    glNewList(list[1], GL_COMPILE);
    glBegin(GL_QUADS);
    glColor3f(0, 0, 1);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glEnd();
    glEndList();

    list[2] = glGenLists(1);
    glNewList(list[2], GL_COMPILE);
    glBegin(GL_QUADS);
    glColor3f(0, 0, 1);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glColor3f(0, 0, 1);
    glNormal3f(0.0f,-1.0f, 0.0f);
    glVertex3f(1,0,0); glVertex3f(1,0,1); glVertex3f(0,0,1); glVertex3f(0,0,0);
    glEnd();
    glEndList();

    list[3] = glGenLists(1);
    glNewList(list[3], GL_COMPILE);
    glBegin(GL_QUADS);
    glColor3f(0, 0, 1);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glColor3f(0, 0, 1);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1,1,0); glVertex3f(1,1,1); glVertex3f(1,0,1); glVertex3f(1,0,0);
    glEnd();
    glEndList();

    list[4] = glGenLists(1);
    glNewList(list[4], GL_COMPILE);
    glBegin(GL_QUADS);
    glColor3f(0, 0.3f, 0);
    glNormal3f(1.0f, 0.0f, 1.0f);
    glVertex3f(1,1,1); glVertex3f(0,1,1); glVertex3f(0,0,1); glVertex3f(1,0,1);
    glEnd();
    glEndList();
}

// ─── paintGL: main render + game logic ───────────────────────────────────────

void PacmanWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Tunnel warp for Pacman
    if ((int)pac_x == 27 && (int)pac_y == 14 && (int)angle1 == 0) {
        pac_x = 0; animate = true;
    } else if ((int)(pac_x + 0.9) == 0 && (int)pac_y == 14 && (int)angle1 == 180) {
        pac_x = 27; animate = true;
    }

    if (animate)
        MovePac();

    // Stop if hitting a wall
    if (!Open((int)(pac_x + cos(M_PI / 180.0 * angle1)),
              (int)(pac_y + sin(M_PI / 180.0 * angle1))) &&
        pac_x - (int)pac_x < 0.1 && pac_y - (int)pac_y < 0.1)
        animate = false;

    // Eat normal pebble
    if (tp_array[(int)(pac_y + 0.5)][(int)(pac_x + 0.5)] == 1) {
        tp_array[(int)(pac_y + 0.5)][(int)(pac_x + 0.5)] = 0;
        pebbles_left--;
        points += 1;
    }
    // Eat super pebble
    else if (tp_array[(int)(pac_y + 0.5)][(int)(pac_x + 0.5)] == 3) {
        tp_array[(int)(pac_y + 0.5)][(int)(pac_x + 0.5)] = 0;
        pebbles_left--;
        points += 5;
        for (int i = 0; i < 4; i++)
            if (!ghost[i]->eaten)
                ghost[i]->Vulnerable();
    }

    // Level complete
    if (pebbles_left == 0) {
        G_Reinit();
        P_Reinit();
        tp_restore();
        points = 0;
        lives  = 3;
    }

    if (!gameover)
        DrawScene();

    // Ghost logic
    for (int d = 0; d < num_ghosts; d++) {
        if (!gameover && start_timer == 0)
            ghost[d]->Update();

        if (!ghost[d]->in_jail &&
            ghost[d]->x - (int)ghost[d]->x < 0.1 &&
            ghost[d]->y - (int)ghost[d]->y < 0.1)
        {
            bool om[4];
            for (int ang = 0; ang < 4; ang++)
                om[ang] = Open((int)(ghost[d]->x + cos(M_PI / 180.0 * ang * 90)),
                               (int)(ghost[d]->y + sin(M_PI / 180.0 * ang * 90)));

            if (!ghost[d]->eaten)
                ghost[d]->Chase(pac_x, pac_y, om);
            else
                ghost[d]->Chase(13, 11, om);
        }

        if (ghost[d]->in_jail &&
            !Open((int)(ghost[d]->x + cos(M_PI / 180.0 * ghost[d]->angle)),
                  (int)(ghost[d]->y + sin(M_PI / 180.0 * ghost[d]->angle))) &&
            ghost[d]->jail_timer > 0 &&
            ghost[d]->x - (int)ghost[d]->x < 0.1 &&
            ghost[d]->y - (int)ghost[d]->y < 0.1)
        {
            ghost[d]->angle = (double)(((int)ghost[d]->angle + 180) % 360);
        }

        if (!gameover && start_timer == 0)
            ghost[d]->Move();
        ghost[d]->Draw();

        if (!ghost[d]->eaten) {
            bool collide = ghost[d]->Catch(pac_x, pac_y);
            if (lives > 0 && collide && !ghost[d]->edible) {
                lives--;
                if (lives == 0) { gameover = true; ghost[d]->game_over(); }
                P_Reinit();
                d = 4;
            } else if (collide && ghost[d]->edible) {
                ghost[d]->edible = false;
                ghost[d]->eaten  = true;
                ghost[d]->speed  = 1;
            }
        }
    }

    // ── HUD text via QPainter overlay ────────────────────────────────────────
    // Qt 6: use QPainter on top of the OpenGL scene
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font("Arial", 14, QFont::Bold);
    painter.setFont(font);

    int ww = width(), wh = height();

    if (gameover) {
        painter.setPen(QColor(255, 0, 0));
        QFont bigFont("Arial", 28, QFont::Bold);
        painter.setFont(bigFont);
        painter.drawText(QRect(0, wh / 2 - 20, ww, 40), Qt::AlignCenter, "GAME OVER");
        painter.setFont(font);
    }

    painter.setPen(QColor(255, 0, 0));
    painter.drawText(10, 24, "PAC MAN");

    painter.setPen(QColor(255, 255, 0));
    painter.drawText(ww / 2 - 50, 24, QString("Points: %1").arg(points));

    painter.setPen(QColor(255, 255, 0));
    painter.drawText(ww - 120, 24, QString("Lives: %1").arg(lives));

    if (start_timer > 0) {
        painter.setPen(QColor(255, 255, 255));
        QFont bigFont("Arial", 22, QFont::Bold);
        painter.setFont(bigFont);
        painter.drawText(QRect(0, wh / 2 - 30, ww, 50), Qt::AlignCenter,
                         "Press arrow key to start!");
    }

    painter.end();
}
