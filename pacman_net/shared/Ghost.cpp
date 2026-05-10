#include "Ghost.h"
#include "Constants.h"
#include <math.h>

#ifndef PACMAN_SERVER_BUILD
// We use Qt's OpenGL headers instead of freeglut
#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif
#endif

// Helper: draw a solid sphere using GLU (replaces glutSolidSphere)
static void drawSphere(double radius, int slices, int stacks)
{
    #ifndef PACMAN_SERVER_BUILD
    GLUquadric *q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
    #endif
}

Ghost::Ghost(double tx, double ty)
{
    x = tx;   // BUG FIX: original had tx=x; ty=y; (assignments backwards)
    y = ty;
    angle = 90;
    speed = max_speed = 1;
    color[0] = 1; color[1] = 0; color[2] = 0;
    eaten = false;
    edible_max_time = 300;
    edible = false;
    in_jail = true;
    jail_timer = 30;
    transporting = false;
    edible_timer = 0;
}

Ghost::~Ghost(void) {}

void Ghost::Reinit(void)
{
    edible = false;
    in_jail = true;
    angle = 90;
}

void Ghost::Move()
{
    x += speed * cos(M_PI / 180.0 * angle);
    y += speed * sin(M_PI / 180.0 * angle);
}

void Ghost::game_over() {}

void Ghost::Update(void)
{
    if ((int)x == 0 && (int)y == 14 && (!transporting))
        angle = 180;

    if (x < 0.1 && (int)y == 14) {
        x = 26.9;
        transporting = true;
    }

    if ((int)x == 27 && (int)y == 14 && (!transporting))
        angle = 0;

    if (x > 26.9 && (int)y == 14) {
        x = 0.1;
        transporting = true;
    }

    if ((int)x == 2 || (int)x == 25)
        transporting = false;

    // slow down near tunnel (original logic preserved, though second line always executes)
    if (((int)x < 5 || (int)x > 21) && (int)y == 14 && !edible && !eaten)
        speed = max_speed / 2;
    speed = max_speed;   // kept as-is from original

    // edibility timer
    if (edible_timer == 0 && edible && !eaten) {
        edible = false;
        speed = max_speed;
    }
    if (edible)
        edible_timer--;

    // JAIL exit logic
    if (in_jail && (int)(y + 0.9) == 11) {
        in_jail = false;
        angle = 180;
    }

    if (in_jail && ((int)x == 13 || (int)x == 14))
        angle = 270;

    if (jail_timer == 0 && in_jail) {
        if (x < 13) angle = 0;
        if (x > 14) angle = 180;
    }

    if (jail_timer > 0)
        jail_timer--;

    // eaten ghost return to jail
    if (eaten && ((int)x == 13 || (int)(x + 0.9) == 14) &&
        ((int)y > 10 && (int)y < 15))
    {
        in_jail = true;
        angle = 90;
        if ((int)y == 14) {
            eaten = false;
            speed = max_speed;
            jail_timer = 66;
            x = 11;
        }
    }
}

bool Ghost::Catch(double px, double py)
{
    if (px - x < 0.2 && px - x > -0.2 && py - y < 0.2 && py - y > -0.2)
        return true;
    return false;
}

void Ghost::Vulnerable(void)
{
    if (!edible)
        angle = ((int)angle + 180) % 360;
    edible = true;
    edible_timer = edible_max_time;
}

void Ghost::Chase(double px, double py, bool *open_move)
{
    int c = edible ? -1 : 1;

    if ((int)angle == 0 || (int)angle == 180) {
        if ((int)c * py > (int)c * y && open_move[1])
            angle = 90;
        else if ((int)c * py < (int)c * y && open_move[3])
            angle = 270;
    }
    else if ((int)angle == 90 || (int)angle == 270) {
        if ((int)c * px > (int)c * x && open_move[0])
            angle = 0;
        else if ((int)c * px < (int)c * x && open_move[2])
            angle = 180;
    }

    if ((int)angle == 0   && !open_move[0]) angle = 90;
    if ((int)angle == 90  && !open_move[1]) angle = 180;
    if ((int)angle == 180 && !open_move[2]) angle = 270;
    if ((int)angle == 270 && !open_move[3]) angle = 0;
    if ((int)angle == 0   && !open_move[0]) angle = 90;
}

void Ghost::Draw(void)
{
    #ifndef PACMAN_SERVER_BUILD
    if (!edible)
        glColor3f(color[0], color[1], color[2]);
    else {
        if (edible_timer < 150)
            glColor3f((edible_timer / 10) % 2, (edible_timer / 10) % 2, 1);
        else
            glColor3f(0, 0, 1);
    }

    if (eaten)
        glColor3f(1, 1, 0);

    glPushMatrix();
    glTranslatef((float)x, -(float)y, 0);
    glTranslatef(0.5f, 0.6f, 0);
    glTranslatef((float)BOARD_X / -2.0f, (float)BOARD_Y / 2.0f, 0.5f);
    drawSphere(0.5, 10, 10);
    glPopMatrix();
    #endif
}
