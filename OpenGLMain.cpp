/*
 * OpenGLMain.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: Isaak Lim
 */

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include "Vector3DMath.hpp"

using namespace std;

// menu params --------------------------
#define WIDTH 800
#define HEIGHT 600
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
enum {
   MU_X,
   MU_Y,
   S_X,
   S_Y,
   R_C
};

float gauss[] = {400, 300, 1, 1, 0};
GLfloat mu[] = {gauss[MU_X], gauss[MU_Y]};
GLfloat cov[] = {gauss[S_X], gauss[R_C], gauss[R_C], gauss[S_Y]};
GLint splattingShader = 0;
GLint gaussLoc = 0;
// --------------------------------------

// vbo ----------------------------------
GLuint gaussVBO = 0;
GLint muLoc = 0;
GLint covLoc = 0;
GLint projectionLoc = 0;
GLint modelViewLoc = 0;
//---------------------------------------

// misc ---------------------------------
#define PI 3.14159265
//---------------------------------------

/*
 * makes a char array from a given file
 */
char* readShaderFromFile(const string& fileName) {
   ifstream temp(fileName.c_str());
   int count = 0;
   char* buf;

   temp.seekg(0, ios::end);
   count = temp.tellg();

   buf = new char[count + 1];
   memset(buf, 0, count);
   temp.seekg(0, ios::beg);
   temp.read(buf, count);
   buf[count] = 0;
   temp.close();

   return buf;
}

/*
 * Initializes the OpenGL parameters
 */
void initOpenGL(int width, int height) {
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 1, 1000.0f);

   glMatrixMode(GL_MODELVIEW);
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
   gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 1, 1000.0f);

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
         if (gauss[S_X] >= -0.1 && gauss[S_X] < 0) // make sure we don't divide by zero
            gauss[S_X] += 0.2;
         else
            gauss[S_X] += 0.1;
         break;
      case SY:
         if (gauss[S_Y] >= -0.1 && gauss[S_Y] < 0) // make sure we don't divide by zero
            gauss[S_Y] += 0.2;
         else
            gauss[S_Y] += 0.1;
         break;
      case C:
         gauss[R_C] += 0.01;
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
         if (gauss[S_X] <= 0.1 && gauss[S_X] > 0) // make sure we don't divide by zero
            gauss[S_X] -= 0.2;
         else
            gauss[S_X] -= 0.1;
         break;
      case SY:
         if (gauss[S_Y] <= 0.1 && gauss[S_Y] > 0) // make sure we don't divide by zero
            gauss[S_Y] -= 0.2;
         else
            gauss[S_Y] -= 0.1;
         break;
      case C:
         gauss[R_C] -= 0.01;
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
      x << "s_x: " << gauss[S_X];
      y << "s_y: " << gauss[S_Y];
      cStr << "c: " << gauss[R_C];
      drawString(y.str(), width-((width/100)*15), 70, font, marked == SY);
      drawString(x.str(), width-((width/100)*15), 90, font, marked == SX);
      drawString(cStr.str(), width-((width/100)*15), 110, font, marked == C);

}

void drawOpenGLScene() {
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
   glLoadIdentity();
   gluLookAt(0, 0, 5,
             0, 0, -1,
             0, 1, 0);

   glutSolidTeapot(1);

   drawMenu();

   glutSwapBuffers();
}

int main(int argc, char **argv) {
   glutInit(&argc, argv);

   // use double buffer
   glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);

   glutInitWindowSize(WIDTH, HEIGHT);

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

   initOpenGL(WIDTH, HEIGHT);

   timebase = glutGet(GLUT_ELAPSED_TIME);

   glutMainLoop();

   return 0;
}
