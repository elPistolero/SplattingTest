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

float gauss[] = {0, 0, 1, 1, 0};
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
   glClearDepth(1.0);
   glDepthFunc(GL_LESS);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);

   glMatrixMode(GL_MODELVIEW);

   // compile and link the shader program
   const char* vertexSrc = readShaderFromFile("Splatting.vert");
   const char* geomSrc = readShaderFromFile("Splatting.geom");
   const char* fragSrc = readShaderFromFile("Splatting.frag");
   int isCompiledVS, isCompiledFS, isCompiledGEO, maxLength, isLinked;

   splattingShader = glCreateProgram();
   // compile the vertex shader
   int vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, (const GLchar**)&vertexSrc, 0);
   glCompileShader(vertexShader);
   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiledVS);
   if (!isCompiledVS) {
      glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* vertexInfoLog = new char[maxLength];

      glGetShaderInfoLog(vertexShader, maxLength, &maxLength, vertexInfoLog);
      fprintf(stdout, "vertex shader: compile error: ");
      fprintf(stdout, vertexInfoLog);
      delete[] vertexInfoLog;
      return;
   }

   // compile the geometry shader
   int geoShader = glCreateShader(GL_GEOMETRY_SHADER);
   glShaderSource(geoShader, 1, (const GLchar**)&geomSrc, 0);
   glCompileShader(geoShader);
   glGetShaderiv(geoShader, GL_COMPILE_STATUS, &isCompiledGEO);
   if (!isCompiledGEO) {
      glGetShaderiv(geoShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* geoInfoLog = new char[maxLength];

      glGetShaderInfoLog(geoShader, maxLength, &maxLength, geoInfoLog);
      fprintf(stdout, "geometry shader: compile error: ");
      fprintf(stdout, geoInfoLog);
      delete[] geoInfoLog;
      return;
   }

   // compile the fragment shader
   int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const GLchar**)&fragSrc, 0);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiledFS);
   if (!isCompiledFS) {
      glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* fragInfoLog = new char[maxLength];

      glGetShaderInfoLog(fragShader, maxLength, &maxLength, fragInfoLog);
      fprintf(stdout, "fragment shader: compile error: ");
      fprintf(stdout, fragInfoLog);
      delete[] fragInfoLog;
      return;
   }

   // attach and link the shaders to the program
   glAttachShader(splattingShader, vertexShader);
   glAttachShader(splattingShader, geoShader);
   glAttachShader(splattingShader, fragShader);

   glLinkProgram(splattingShader);
   glGetProgramiv(splattingShader, GL_LINK_STATUS, (int*)&isLinked);
   if (!isLinked) {
      glGetProgramiv(splattingShader, GL_INFO_LOG_LENGTH, &maxLength);

      char* splattingInfoLog = new char[maxLength];

      glGetProgramInfoLog(splattingShader, maxLength, &maxLength, splattingInfoLog);
      fprintf(stdout, "shader: link error: ");
      fprintf(stdout, splattingInfoLog);
      delete[] splattingInfoLog;
      return;
   }

   muLoc = glGetAttribLocation(splattingShader, "mu");
   covLoc = glGetAttribLocation(splattingShader, "cov");
   projectionLoc = glGetUniformLocation(splattingShader, "projectionMatrix");
   modelViewLoc = glGetUniformLocation(splattingShader, "modelViewMatrix");

   // passes the projection matrix to the shader
   GLfloat pProj[16];
   glGetFloatv(GL_PROJECTION_MATRIX, pProj);
   glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, pProj);



   // generate buffer objects
   glGenBuffers(1, &gaussVBO);
   glBindBuffer(GL_ARRAY_BUFFER, gaussVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gauss), &gauss, GL_DYNAMIC_DRAW);
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
   gluLookAt(  0, 0, 5,
               0, 0, -1,
               0, 1, 0);

   // passes the modelview matrix to the shader
   GLfloat pModView[16];
   glGetFloatv(GL_MODELVIEW, pModView);
   glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, pModView);

   /*
   // tempory calculation of bounding box (mu is 0,0)
   float trace = gauss[S_X] + gauss[S_Y];
   float det = gauss[S_X] * gauss[S_Y] - pow(gauss[R_C], 2);
   float eigenVal1 = (trace + sqrt(pow(trace, 2) - 4 * det))/2;
   float eigenVal2 = (trace - sqrt(pow(trace, 2) - 4 * det))/2;

   // length of each axis of the ellipse
   float lengthX = sqrt(eigenVal1);
   float lengthY = sqrt(eigenVal2);

   // orientation of the first eigenvector in relation to the x axis
   float angleDeg = 0;
   if (gauss[R_C]) {
      Vector eigenVec1 = Vector(eigenVal1 - gauss[S_Y], gauss[R_C], 0);
      //Vector eigenVec2 = Vector(eigenVal2 - s_y, c, 0);
      float angleRad = acos((eigenVec1 ^ Vector(0, 1, 0))/eigenVec1.length());
      angleDeg = angleRad * 180/PI;
   }

   // draw bounding rectangle
   if (menuStates[WIREFRAME]) {
      glPushMatrix();

      if (gauss[R_C])
         glRotatef(angleDeg, 0, 0, 1);
      glColor3f(1, 0, 0);

      glBegin(GL_LINE_STRIP);
      glVertex3f(-lengthX, -lengthY, 0);
      glVertex3f(lengthX, -lengthY, 0);
      glVertex3f(lengthX, lengthY, 0);
      glVertex3f(-lengthX, lengthY, 0);
      glVertex3f(-lengthX, -lengthY, 0);
      glEnd();

      glPopMatrix();
   }
   */

   glUseProgram(splattingShader);

   glBindBuffer(GL_ARRAY_BUFFER, gaussVBO);
   glEnableVertexAttribArray(muLoc);
   glEnableVertexAttribArray(covLoc);

   glVertexAttribPointer(muLoc, 2, GL_FLOAT, GL_FALSE, sizeof(gauss), (void*)0);
   glVertexAttribPointer(covLoc, 3, GL_FLOAT, GL_FALSE, sizeof(gauss), (void*)(sizeof(GL_FLOAT)*2));

   glDrawArrays(GL_POINTS, 0, 1);
   //GLenum error = glGetError();

   glDisableVertexAttribArray(covLoc);
   glDisableVertexAttribArray(muLoc);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glUseProgram(0);

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
