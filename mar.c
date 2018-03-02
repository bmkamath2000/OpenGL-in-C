#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
int marble[9][9]={{0,0,0,1,1,1,0,0,0},
					{0,0,0,1,1,1,0,0,0},
					{0,0,0,1,1,1,0,0,0},
					{1,1,1,1,1,1,1,1,1},
					{1,1,1,1,1,1,1,1,1},
					{1,1,1,1,1,1,1,1,1},
					{0,0,0,1,1,1,0,0,0},
					{0,0,0,1,1,1,0,0,0},
					{0,0,0,1,1,1,0,0,0}};
int wh=500,firstx=1,firsty=1,secondx,secondy,cfx=4,cfy=4,clickcount=-1;
GLuint startList;

void errorCallback(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   fprintf(stderr, "Quadric Error: %s\n", estring);
   exit(0);
}

void init(void)
{
   GLUquadricObj *qobj;
   GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
   GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

   glClearColor(0.0, 0.0, 0.0, 0.0);

   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);

/* Create 4 display lists, each with a different quadric object.
 * Different drawing styles and surface normal specifications
 * are demonstrated.
 */
   startList = glGenLists(5);
   qobj = gluNewQuadric();
   gluQuadricCallback(qobj, GLU_ERROR, errorCallback);

   gluQuadricDrawStyle(qobj, GLU_FILL); /* wireframe */
   gluQuadricNormals(qobj, GLU_SMOOTH);
   glNewList(startList, GL_COMPILE);
      gluDisk(qobj, 0, 1.0, 20, 4);
   glEndList();
   gluQuadricDrawStyle(qobj, GLU_FILL); /* wireframe */
      gluQuadricNormals(qobj, GLU_SMOOTH);
      glNewList(startList+1, GL_COMPILE);
         gluDisk(qobj, 0, 10.0, 50, 4);
      glEndList();

        gluQuadricDrawStyle(qobj, GLU_FILL); /* wireframe */
              gluQuadricNormals(qobj, GLU_SMOOTH);

              glNewList(startList+2, GL_COMPILE);
              glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glPushMatrix();
                    mat_ambient[0] = 1.0;
                    mat_ambient[1]=1.0;
                    mat_ambient[2] = 1.0;
                      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
                      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
                      glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        gluDisk(qobj, 0, 0.5, 20, 4);
        glPopMatrix();
              glPopAttrib();
        glEndList();
        gluQuadricDrawStyle(qobj, GLU_FILL); /* wireframe */
                      gluQuadricNormals(qobj, GLU_SMOOTH);

                      glNewList(startList+3, GL_COMPILE);
                      glPushAttrib(GL_ALL_ATTRIB_BITS);
                            glPushMatrix();
                            mat_ambient[1] = 0.0;
                            mat_ambient[2] = 1.0;
                              glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
                              glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
                              glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
                gluDisk(qobj, 0, 0.45, 20, 4);
                glPopMatrix();
                      glPopAttrib();
                glEndList();
                glNewList(startList+4, GL_COMPILE);
                                      glPushAttrib(GL_ALL_ATTRIB_BITS);
                                            glPushMatrix();
                                            mat_ambient[0] = 0.0;
                                            mat_ambient[2] = 1.0;
                                              glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
                                              glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
                                              glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
                                              glBegin(GL_POLYGON);
                                              glVertex3f(0,0,0);
                                              glVertex3f(2,0,0);
                                              glVertex3f(2,2,0);
                                              glVertex3f(0,2,0);
                                              glEnd();
                                glPopMatrix();
                                      glPopAttrib();
                                glEndList();


}
void drawboard()
{int i,j;
	glEnable(GL_LIGHTING);
		   	   glShadeModel (GL_SMOOTH);
		   	   glCallList(startList+1);
		   	   glPushMatrix();
		   	glTranslatef(-8.0, -8.0, 1.0);
for(i=0;i<9;i++)
	for(j=0;j<9;j++)
		{
		if(i==3||i==4||i==5||j==3||j==4||j==5)
		{
		glPushMatrix();
		glTranslatef(i*2, j*2, 0.0);
		glCallList(startList+2);
		glPopMatrix();
		}
		}
glTranslatef(0,0,1);
for(i=0;i<9;i++)
	for(j=0;j<9;j++)
		{
		if(marble[i][j]==1)
		{
		glPushMatrix();
		glTranslatef(i*2, j*2, 1.0);
		glCallList(startList+3);
		glPopMatrix();
		}
		}
glTranslatef(cfx*2-1,cfy*2-1,-1.5);
glCallList(startList+4);

glPopMatrix();
}
void display(void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glPushMatrix();

drawboard();

   glPopMatrix();
   glFlush();
   glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      glOrtho(-10, 10, -10*(GLfloat)h/(GLfloat)w,
         10*(GLfloat)h/(GLfloat)w, -10.0, 10.0);
   else
      glOrtho(-10*(GLfloat)w/(GLfloat)h,
         10*(GLfloat)w/(GLfloat)h, -10, 10, -10.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   wh=h;
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         exit(0);
         break;
   }
}
void mouse(int btn,int status,int x,int y)
{int mid;
	y=wh-y;

if(btn==GLUT_LEFT_BUTTON&&status==GLUT_DOWN)
{
	clickcount=(clickcount+1)%2;

		cfx=x/55;
		cfy=y/55;
		printf("%d %d %d %d\n",x,y,cfx,cfy);
	if((marble[x/55][y/55]==1)&&clickcount==0)
	{
		firstx=x/55;
		firsty=y/55;
	}
	if((marble[x/55][y/55]==0)&&clickcount==1)
	{
		secondx=x/55;
		secondy=y/55;
		if((firsty==secondy)&&(abs(firstx-secondx)==2))
		{mid=(firstx+secondx)/2;
			if(marble[mid][firsty]==1)
				{marble[firstx][firsty]=0;
				marble[mid][firsty]=0;
				marble[secondx][secondy]=1;
				}
		}
		else if((firstx==secondx)&&(abs(firsty-secondy)==2))
		{
			mid=(firsty+secondy)/2;
						if(marble[firstx][mid]==1)
							{marble[firstx][firsty]=0;
							marble[firstx][mid]=0;
							marble[secondx][secondy]=1;
							}
		}
	}
}
glutPostRedisplay();
}
int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow(argv[0]);
   init();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMouseFunc(mouse);
   marble[4][4]=0;
   glutMainLoop();
   return 0;
}
