#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int marble[9][9] = {
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,1,1,1,0,0,0},
    {1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,1,1,1,1}, // center empty
    {1,1,1,1,1,1,1,1,1},
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,1,1,1,0,0,0}
};

int winW = 600, winH = 600;
int clickStage = 0; // 0 = waiting for first click, 1 = waiting for second click
int firstX, firstY;

void drawFilledCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * (float)i / (float)segments;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

void drawBoard2D() {
    // board background
    glColor3f(0.55f, 0.35f, 0.18f);
    glBegin(GL_QUADS);
      glVertex2f(0.0f, 0.0f);
      glVertex2f(18.0f, 0.0f);
      glVertex2f(18.0f, 18.0f);
      glVertex2f(0.0f, 18.0f);
    glEnd();

    // draw holes + marbles
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (i==3 || i==4 || i==5 || j==3 || j==4 || j==5) {
                float cx = i*2.0f + 1.0f;
                float cy = j*2.0f + 1.0f;

                // hole
                glColor3f(0.9f, 0.9f, 0.9f);
                drawFilledCircle(cx, cy, 0.5f, 32);

                // marble
                if (marble[i][j] == 1) {
                    if (clickStage == 1 && i == firstX && j == firstY) {
                        glColor3f(1.0f, 0.0f, 0.0f); // selected = red
                    } else {
                        glColor3f(0.0f, 0.2f, 0.8f); // normal = blue
                    }
                    drawFilledCircle(cx, cy, 0.7f, 40);
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawFilledCircle(cx - 0.25f, cy + 0.25f, 0.18f, 20);
                }
            }
        }
    }
}

void display(void) {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 18.0, 0.0, 18.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawBoard2D();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float bx = (float)x / (float)winW * 18.0f;
        float by = (float)(winH - y) / (float)winH * 18.0f;
        int i = (int)(bx / 2.0f);
        int j = (int)(by / 2.0f);

        if (i < 0 || i > 8 || j < 0 || j > 8) return;

        if (clickStage == 0) {
            // pick first marble
            if (marble[i][j] == 1) {
                firstX = i;
                firstY = j;
                clickStage = 1;
            }
        } else {
            // try to move to empty hole
            if (marble[i][j] == 0) {
                if (firstX == i && abs(firstY - j) == 2) {
                    int mid = (firstY + j) / 2;
                    if (marble[i][mid] == 1) {
                        marble[firstX][firstY] = 0;
                        marble[i][mid] = 0;
                        marble[i][j] = 1;
                    }
                } else if (firstY == j && abs(firstX - i) == 2) {
                    int mid = (firstX + i) / 2;
                    if (marble[mid][j] == 1) {
                        marble[firstX][firstY] = 0;
                        marble[mid][j] = 0;
                        marble[i][j] = 1;
                    }
                }
            }
            clickStage = 0; // reset selection
        }
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Marble Solitaire (Playable, 2D)");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
