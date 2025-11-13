/*
Rolling Stone Game


Authors:
Mukesh Kamath Bola	
Date of Creation: 10th March 2021
Date of Last Modification: 20th March 2021
Purpose: A simple game where a ball is rolled down a slope avoiding obstacles.

*/
#include<stdlib.h>
#include<GL/glut.h>
#include<stdio.h>

#define dx 100
#define dy 20
typedef struct cell
		{
	GLfloat x0,y0,x1,y1;
	int color;
		}cellt;
cellt impediments[10];
GLfloat color[3][3]={{0,0,1},{0,1,0},{1,0,0}};
GLfloat ballx,bally,ty=0;
int mode=0;
void init()
{

	glClearColor(1.0,1.0,1.0,1.0);
	glColor3f(1.0,0.0,0.0);
	glPointSize(5.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0,499.0,0.0,499.0);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();

}
void createrand()
{int i=0,y=-1000;
for(i=0;i<10;i++)
{
	impediments[i].x0=rand()%500;
	impediments[i].x1=impediments[i].x0+50;
	impediments[i].y0=y;
	impediments[i].y1=y+20;
	impediments[i].color=rand()%3;
	y+=100;
}
mode=1;
}
void drawimpediments()
{int i=0;
for(i=0;i<10;i++)
{
	glColor3fv(color[impediments[i].color]);
	glBegin(GL_POLYGON);
	glVertex2f(impediments[i].x0,impediments[i].y0);
	glVertex2f(impediments[i].x0,impediments[i].y1);
	glVertex2f(impediments[i].x1,impediments[i].y1);
	glVertex2f(impediments[i].x1,impediments[i].y0);
	glEnd();
}
}
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT);
if(mode==0)
	createrand();
glLoadIdentity();
glPushMatrix();
glTranslatef(0,ty,0);
drawimpediments();
glPopMatrix();
	glFlush();
	glutSwapBuffers();
}
void mouse(int btn,int status,int x,int y)
{
	y=500-y;

}

void idle(void)
{
ty+=1;
if(ty>1000)
	{
	mode=0;
	ty=0;
	}
glutPostRedisplay();
}
int main(int argc,char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(500,500);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Rolling Stone");
	glutDisplayFunc(display);

	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	init();
	glutMainLoop();
	return(0);
}

