// marble_2d.c
// Simple 2D marble solitaire viewer (no lighting, no materials)
// Draws an 18x18 board (9x9 cells each 2x2) and filled circles for marbles.

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

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

// Draw a filled circle (triangle fan) at (cx,cy) with radius r
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
    glColor3f(0.55f, 0.35f, 0.18f); // brown
    glBegin(GL_QUADS);
      glVertex2f(0.0f, 0.0f);
      glVertex2f(18.0f, 0.0f);
      glVertex2f(18.0f, 18.0f);
      glVertex2f(0.0f, 18.0f);
    glEnd();

    // optional grid lines (light)
    glColor3f(0.25f, 0.15f, 0.08f);
    for (int i = 0; i <= 9; ++i) {
        float x = i * 2.0f;
        glBegin(GL_LINES);
          glVertex2f(x, 0.0f);
          glVertex2f(x, 18.0f);
        glEnd();
        float y = i * 2.0f;
        glBegin(GL_LINES);
          glVertex2f(0.0f, y);
          glVertex2f(18.0f, y);
        glEnd();
    }

    // holes (draw small pale circles on holes positions)
    glColor3f(0.9f, 0.9f, 0.9f);
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (i==3 || i==4 || i==5 || j==3 || j==4 || j==5) {
                float cx = i*2.0f + 1.0f;
                float cy = j*2.0f + 1.0f;
                drawFilledCircle(cx, cy, 0.5f, 32);
            }
        }
    }

    // marbles (filled circles)
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (marble[i][j] == 1) {
                float cx = i*2.0f + 1.0f;
                float cy = j*2.0f + 1.0f;
                glColor3f(0.0f, 0.2f, 0.8f); // blue marble
                drawFilledCircle(cx, cy, 0.7f, 40);
                // small highlight
                glColor3f(1.0f, 1.0f, 1.0f);
                drawFilledCircle(cx - 0.25f, cy + 0.25f, 0.18f, 20);
            }
        }
    }
}

void display(void) {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up simple 2D orthographic view matching board coordinates
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 18.0, 0.0, 18.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw everything (no lighting)
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
        // Convert window coords to board coords
        float bx = (float)x / (float)winW * 18.0f;
        // note: GLUT y=0 at top, our ortho y=0 bottom -> invert
        float by = (float)(winH - y) / (float)winH * 18.0f;
        printf("Mouse click window: %d,%d -> board: %.2f, %.2f\n", x, y, bx, by);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Marble Solitaire (2D, no lighting)");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
