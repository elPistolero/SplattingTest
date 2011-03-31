/*
 * OpenGLMainLoop.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: Isaak Lim
 */

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <math.h>
#include "Vector3DMath.hpp"

using namespace std;

// menu params --------------------------
unsigned int frame = 0;
int timebase = 0;
int currenttime = 0;
string strFPS;
enum {
   PROJECTION,
   WIREFRAME,
   SY,
   SX,
   C
};
#define MENU_SIZE 5;
bool menuStates[] = {false, true, false, false, false};
unsigned int marked = PROJECTION;
// --------------------------------------

// gaussian kernel ----------------------
/*
float mu_x = 0.0;
float mu_y = 0.0;
float s_x = 1.0;
float s_y = 1.0;
float c = 0.0;
*/

struct gaussData {
   float mu_x, mu_y;
   float s_x, s_y, c;
};

gaussData gauss = {0, 0, 1, 1, 0};
// --------------------------------------

// vbo ----------------------------------
GLuint gaussian = 0;
enum {
   COVARIANCE,
   MU
};
//---------------------------------------

// misc ---------------------------------
#define PI 3.14159265
//---------------------------------------

/*
 * Initializes the OpenGL parameters
 */
void initOpenGL(int width, int height) {
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClearDepth(1.0);
   glDepthFunc(GL_LESS);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);

   glMatrixMode(GL_MODELVIEW);

   // generate buffer objects
   glGenBuffers(1, &gaussian);
   glBindBuffer(GL_ARRAY_BUFFER, gaussian);
   //glBufferData(GL_ARRAY_BUFFER, 5*sizeof(float), pGaussian, GL_DYNAMIC_DRAW);
}

/*
 * handle resizing
 */
void resizeOpenGLScene(int width, int height) {
   if (height == 0)
      height = 1;

   glViewport(0, 0, width, height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);

   glMatrixMode(GL_MODELVIEW);
}

/*
 * handles special key-pressed events
 */
void specialKeyPressed (int key, int x, int y) {
   switch (key) {
   case GLUT_KEY_UP:
      marked = (marked + 1) % MENU_SIZE;
      break;
   case GLUT_KEY_DOWN:
      marked = (marked - 1) % MENU_SIZE;
      break;
   case GLUT_KEY_RIGHT:
      switch (marked) {
      case SX:
         if (s_x >= -0.1 && s_x < 0) // make sure we don't divide by zero
            s_x += 0.2;
         else
            s_x += 0.1;
         break;
      case SY:
         if (s_y >= -0.1 && s_y < 0) // make sure we don't divide by zero
            s_y += 0.2;
         else
            s_y += 0.1;
         break;
      case C:
         c += 0.01;
         break;
      default:
         if (menuStates[marked])
            menuStates[marked] = false;
         else
            menuStates[marked] = true;
      }
      break;
   case GLUT_KEY_LEFT:
      switch (marked) {
      case SX:
         if (s_x <= 0.1 && s_x > 0) // make sure we don't divide by zero
            s_x -= 0.2;
         else
            s_x -= 0.1;
         break;
      case SY:
         if (s_y <= 0.1 && s_y > 0) // make sure we don't divide by zero
            s_y -= 0.2;
         else
            s_y -= 0.1;
         break;
      case C:
         c -= 0.01;
         break;
      default:
         if (menuStates[marked])
            menuStates[marked] = false;
         else
            menuStates[marked] = true;
      }
      break;
   }
}

/*
 * draws a String in the OpenGL window
 */
void drawString(string str, float x, float y, void* font, bool marked) {
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   if (marked)
      glColor3f(1.0, 0.0, 0.0);
   else
      glColor3f(0.0, 1.0, 0.0);

   glRasterPos2f(x, y);

   string::iterator it;
   for (it = str.begin(); it < str.end(); it++) {
      glutBitmapCharacter(font, *it);
   }

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}

void drawMenu() {
   // draw FPS rate
   frame++;
   currenttime = glutGet(GLUT_ELAPSED_TIME);

   void* font = GLUT_BITMAP_9_BY_15;

   // check if a second has passed
   if (currenttime - timebase > 1000) {
      float fps = frame*1000.0/(currenttime - timebase);
      ostringstream o;
      o << "FPS: " << (long)fps;
      strFPS = o.str();

      timebase = currenttime;
      frame = 0;
   }

   int width = glutGet(GLUT_WINDOW_WIDTH);
   drawString(strFPS, width-((width/100)*15), 10, font, false);

   // draw the menu
   if (menuStates[PROJECTION])   //
      drawString("Projection: Perspective", width-((width/100)*15), 30, font, marked == PROJECTION);
   else
      drawString("Projection: Orthogonal", width-((width/100)*15), 30, font, marked == PROJECTION);

   if (menuStates[WIREFRAME])
      drawString("Wireframe: On", width-((width/100)*15), 50, font, marked == WIREFRAME);
   else
      drawString("Wireframe: Off", width-((width/100)*15), 50, font, marked == WIREFRAME);

      ostringstream x, y, cStr;
      x << "s_x: " << s_x;
      y << "s_y: " << s_y;
      cStr << "c: " << c;
      drawString(y.str(), width-((width/100)*15), 70, font, marked == SY);
      drawString(x.str(), width-((width/100)*15), 90, font, marked == SX);
      drawString(cStr.str(), width-((width/100)*15), 110, font, marked == C);

}

void drawOpenGLScene() {
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
   glLoadIdentity();

   // tempory calculation of bounding box (mu is 0,0)
   float trace = s_x + s_y;
   float det = s_x * s_y - pow(c, 2);
   float eigenVal1 = (trace + sqrt(pow(trace, 2) - 4 * det))/2;
   float eigenVal2 = (trace - sqrt(pow(trace, 2) - 4 * det))/2;

   // length of each axis of the ellipse
   float lengthX = sqrt(eigenVal1);
   float lengthY = sqrt(eigenVal2);

   // orientation of the first eigenvector in relation to the x axis
   float angleDeg = 0;
   if (c) {
      Vector eigenVec1 = Vector(eigenVal1 - s_y, c, 0);
      //Vector eigenVec2 = Vector(eigenVal2 - s_y, c, 0);
      float angleRad = acos((eigenVec1 ^ Vector(0, 1, 0))/eigenVec1.length());
      angleDeg = angleRad * 180/PI;
   }

   // draw bounding rectangle
   if (menuStates[WIREFRAME]) {
      glPushMatrix();

      glTranslatef(0, 0, -4);
      if (c)
         glRotatef(angleDeg, 0, 0, 1);
      glColor3f(1, 0, 0);

      glBegin(GL_LINE_LOOP);
      glVertex3f(-lengthX, -lengthY, 0);
      glVertex3f(lengthX, -lengthY, 0);
      glVertex3f(lengthX, lengthY, 0);
      glVertex3f(-lengthX, lengthY, 0);
      glEnd();

      glPopMatrix();
   }

   drawMenu();

   glutSwapBuffers();
}

int main(int argc, char **argv) {
   glutInit(&argc, argv);

   // use double buffer
   glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);

   glutInitWindowSize(800, 600);

   glutInitWindowPosition(0, 0);

   glutCreateWindow("");

   glutSpecialFunc(&specialKeyPressed);

   glutDisplayFunc(&drawOpenGLScene);

   glutFullScreen();

   glutIdleFunc(&drawOpenGLScene);

   glutReshapeFunc(&resizeOpenGLScene);

   GLenum err = glewInit();
   if (GLEW_OK != err) {
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }
   fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

   initOpenGL(800, 600);

   timebase = glutGet(GLUT_ELAPSED_TIME);

   glutMainLoop();

   return 0;
}
