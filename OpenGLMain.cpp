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
   PROJECTION, WIREFRAME, MU_X, MU_Y, S_X, S_Y, C
};
#define MENU_SIZE 7;
bool menuStates[] = { true, true};
unsigned int marked = PROJECTION;

// shader  ----------------------
GLint splattingShader = 0;

// vbos ----------------------------------
GLuint gaussVBO = 0;
GLint quadLoc = 0;
GLint muLoc = 0;
GLint sLoc = 0;
GLint cLoc = 0;
GLint weightLoc = 0;

GLfloat s_x = 1;
GLfloat s_y = 1;
GLfloat s_z = 1;
GLfloat c_1 = 0;
GLfloat c_2 = 0;
GLfloat c_3 = 0;
GLfloat mu_x = 0;
GLfloat mu_y = 0;
GLfloat mu_z = -1;
GLfloat weight = 1;

struct GaussVertex {
   float x, y, id;
   float s_x, s_y, s_z;
   float c_1, c_2, c_3;
   float mu_x, mu_y, mu_z;
   float weight;
};

GaussVertex pGauss[4];

// camera coords ------------------------
GLfloat camera[3] = { 0, 1, 5 };
GLfloat rotAngle = 0;

// misc ---------------------------------
const float PI = 3.14159265;
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

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

void setupShaders() {
   // compile and link the shader program
   const char* vertexSrc = readShaderFromFile("Splatting.vert");
   const char* fragSrc = readShaderFromFile("Splatting.frag");
   int isCompiledVS, isCompiledFS, maxLength, isLinked;

   splattingShader = glCreateProgram();
   // compile the vertex shader
   int vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, (const GLchar**) &vertexSrc, 0);
   glCompileShader(vertexShader);
   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiledVS);
   if (!isCompiledVS) {
      glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* vertexInfoLog = new char[maxLength];

      glGetShaderInfoLog(vertexShader, maxLength, &maxLength, vertexInfoLog);
      printf("vertex shader: compile error: ");
      printf("%s", vertexInfoLog);
      delete[] vertexInfoLog;
      return;
   }

   // compile the fragment shader
   int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const GLchar**) &fragSrc, 0);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiledFS);
   if (!isCompiledFS) {
      glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* fragInfoLog = new char[maxLength];

      glGetShaderInfoLog(fragShader, maxLength, &maxLength, fragInfoLog);
      printf("fragment shader: compile error: ");
      printf("%s", fragInfoLog);
      delete[] fragInfoLog;
      return;
   }

   // attach and link the shaders to the program
   glAttachShader(splattingShader, vertexShader);
   glAttachShader(splattingShader, fragShader);

   glLinkProgram(splattingShader);
   glGetProgramiv(splattingShader, GL_LINK_STATUS, (int*) &isLinked);
   if (!isLinked) {
      glGetProgramiv(splattingShader, GL_INFO_LOG_LENGTH, &maxLength);

      char* splattingInfoLog = new char[maxLength];

      glGetProgramInfoLog(splattingShader, maxLength, &maxLength,
            splattingInfoLog);
      fprintf(stdout, "shader: link error: ");
      printf("%s", splattingInfoLog);
      delete[] splattingInfoLog;
      return;
   }

   quadLoc = glGetAttribLocation(splattingShader, "quadVert");
   sLoc = glGetAttribLocation(splattingShader, "s");
   cLoc = glGetAttribLocation(splattingShader, "c");
   muLoc = glGetAttribLocation(splattingShader, "mu");
   weightLoc = glGetAttribLocation(splattingShader, "weight");
}

void setupVBO() {
   pGauss[0].x = 1;
   pGauss[0].y = 1;
   pGauss[1].x = 1;
   pGauss[1].y = -1;
   pGauss[2].x = -1;
   pGauss[2].y = -1;
   pGauss[3].x = -1;
   pGauss[3].y = 1;
   for (int i = 0; i < 4; i++) {
      pGauss[i].id = i;
      pGauss[i].s_x = s_x;
      pGauss[i].s_y = s_y;
      pGauss[i].s_z = s_z;
      pGauss[i].c_1 = c_1;
      pGauss[i].c_2 = c_2;
      pGauss[i].c_3 = c_3;
      pGauss[i].mu_x = mu_x;
      pGauss[i].mu_y = mu_y;
      pGauss[i].mu_z = mu_z;
      pGauss[i].weight = weight;
   }

   glGenBuffers(1, &gaussVBO);

   glBindBuffer(GL_ARRAY_BUFFER, gaussVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GaussVertex)*4, &pGauss[0].x, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(quadLoc);
   glVertexAttribPointer(quadLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(0));
   glEnableVertexAttribArray(sLoc);
   glVertexAttribPointer(sLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(3*sizeof(GL_FLOAT)));
   glEnableVertexAttribArray(cLoc);
   glVertexAttribPointer(cLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(6*sizeof(GL_FLOAT)));
   glEnableVertexAttribArray(muLoc);
   glVertexAttribPointer(muLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(9*sizeof(GL_FLOAT)));
   glEnableVertexAttribArray(weightLoc);
   glVertexAttribPointer(weightLoc, 1, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(12*sizeof(GL_FLOAT)));
}

/*
 * Initializes the OpenGL parameters
 */
void initOpenGL(int width, int height) {
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

   setupShaders();
   setupVBO();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   gluPerspective(45.0f, (GLdouble) width / (GLdouble) height, 1, 1000.0f);

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
   gluPerspective(45.0f, (GLdouble) width / (GLdouble) height, 1, 1000.0f);

   glMatrixMode(GL_MODELVIEW);
}

void keyPressed(unsigned char key, int x, int y) {
   if (key == 27) {
      exit(0);
   } else if (key == 'q') {
      rotAngle -= 5;
   } else if (key == 'e') {
      rotAngle += 5;
   } else if (key == 'a') {
      camera[0]--;
   } else if (key == 'd') {
      camera[0]++;
   } else if (key == 'w') {
      camera[2]--;
   } else if (key == 's') {
      camera[2]++;
   } else if (key == 'y') {
      camera[1]++;
   } else if (key == 'c') {
      camera[1]--;
   }
}

/*
 * handles special key-pressed events
 */
void specialKeyPressed(int key, int x, int y) {
   switch (key) {
   case GLUT_KEY_UP:
      marked = (marked + 1) % MENU_SIZE;
      break;
   case GLUT_KEY_DOWN:
      marked = (marked - 1) % MENU_SIZE;
      break;
   case GLUT_KEY_RIGHT:
      switch (marked) {
      case MU_X:
         mu_x++;
         break;
      case MU_Y:
         mu_y++;
         break;
      case S_X:
         if (s_x >= -0.1 && s_x < 0) // make sure we don't divide by zero
            s_x += 0.2;
         else
            s_x += 0.1;
         break;
      case S_Y:
         if (s_y >= -0.1 && s_y < 0) // make sure we don't divide by zero
            s_y += 0.2;
         else
            s_y += 0.1;
         break;
      case C:
         c_1 += 0.01;
         break;
      default:
         if (menuStates[marked])
            menuStates[marked] = false;
         else
            menuStates[marked] = true;
         break;
      }
      break;
   case GLUT_KEY_LEFT:
      switch (marked) {
      case MU_X:
         mu_x--;
         break;
      case MU_Y:
         mu_y--;
         break;
      case S_X:
         if (s_x <= 0.1 && s_x > 0) // make sure we don't divide by zero
            s_x -= 0.2;
         else
            s_x -= 0.1;
         break;
      case S_Y:
         if (s_y <= -0.1 && s_y > 0) // make sure we don't divide by zero
            s_y -= 0.2;
         else
            s_y -= 0.1;
         break;
      case C:
         c_1 -= 0.01;
         break;
      default:
         if (menuStates[marked])
            menuStates[marked] = false;
         else
            menuStates[marked] = true;
         break;
      }
   }
}

/*
 * draws a String in the OpenGL window
 */
void drawString(string str, float x, float y, void* font, bool marked) {
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT),
         -1.0, 1.0);

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
      float fps = frame * 1000.0 / (currenttime - timebase);
      ostringstream o;
      o << "FPS: " << (long) fps;
      strFPS = o.str();

      timebase = currenttime;
      frame = 0;
   }

   int width = glutGet(GLUT_WINDOW_WIDTH);
   drawString(strFPS, width - ((width / 100) * 15), 10, font, false);

   // draw the menu
   if (menuStates[PROJECTION]) //
      drawString("Projection: Perspective", width - ((width / 100) * 15), 30,
            font, marked == PROJECTION);
   else
      drawString("Projection: Orthogonal", width - ((width / 100) * 15), 30,
            font, marked == PROJECTION);

   if (menuStates[WIREFRAME])
      drawString("Wireframe: On", width - ((width / 100) * 15), 50, font,
            marked == WIREFRAME);
   else
      drawString("Wireframe: Off", width - ((width / 100) * 15), 50, font,
            marked == WIREFRAME);

   ostringstream muX, muY, sX, sY, c;
   muX << "mu_x: " << mu_x;
   muY << "mu_y: " << mu_y;
   sX << "s_x: " << s_x;
   sY << "s_y: " << s_y;
   c << "c: " << c_1;
   drawString(muX.str(), width - ((width / 100) * 15), 70, font, marked == MU_X);
   drawString(muY.str(), width - ((width / 100) * 15), 90, font, marked == MU_Y);
   drawString(sX.str(), width - ((width / 100) * 15), 110, font, marked == S_X);
   drawString(sY.str(), width - ((width / 100) * 15), 130, font, marked == S_Y);
   drawString(c.str(), width - ((width / 100) * 15), 150, font, marked == C);
}

void drawGrid() {
   glBegin(GL_LINES);
   for (int i = -10; i <= 10; i++) {
      if (i == 0) {
         glColor3f(.6, .3, .3);
      } else {
         glColor3f(.25, .25, .25);
      };
      glVertex3f(i, 0, -10);
      glVertex3f(i, 0, 10);
      if (i == 0) {
         glColor3f(.3, .3, .6);
      } else {
         glColor3f(.25, .25, .25);
      };
      glVertex3f(-10, 0, i);
      glVertex3f(10, 0, i);
   };
   glEnd();
}

void drawOpenGLScene() {
   if (menuStates[PROJECTION]) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(45.0f, (GLdouble) WIDTH / (GLdouble) HEIGHT, 1, 1000.0f);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(camera[0], camera[1], camera[2], 0, 0, -1, 0, 1, 0);
      glRotatef(rotAngle, 0, 1, 0);
   } else {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-10, 10, -10, 10, 1.0, 1000.0);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
   }

   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   if (menuStates[WIREFRAME])
      drawGrid();

   glUseProgram(splattingShader);

   glBindBuffer(GL_ARRAY_BUFFER, gaussVBO);
   glEnableVertexAttribArray(quadLoc);
   glEnableVertexAttribArray(sLoc);
   glEnableVertexAttribArray(cLoc);
   glEnableVertexAttribArray(muLoc);
   glEnableVertexAttribArray(weightLoc);
   glVertexAttribPointer(quadLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(0));
   glVertexAttribPointer(sLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(3*sizeof(GL_FLOAT)));
   glVertexAttribPointer(cLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(6*sizeof(GL_FLOAT)));
   glVertexAttribPointer(muLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(9*sizeof(GL_FLOAT)));
   glVertexAttribPointer(weightLoc, 1, GL_FLOAT, GL_FALSE, sizeof(GaussVertex), BUFFER_OFFSET(12*sizeof(GL_FLOAT)));

   glDrawArrays(GL_QUADS, 0, 4);

   glDisableVertexAttribArray(weightLoc);
   glDisableVertexAttribArray(muLoc);
   glDisableVertexAttribArray(cLoc);
   glDisableVertexAttribArray(sLoc);
   glDisableVertexAttribArray(quadLoc);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glUseProgram(0);

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

   glutKeyboardFunc(&keyPressed);

   glutSpecialFunc(&specialKeyPressed);

   glutDisplayFunc(&drawOpenGLScene);

   glutFullScreen();

   glutIdleFunc(&drawOpenGLScene);

   glutReshapeFunc(&resizeOpenGLScene);

   glewInit();
   if (glewIsSupported("GL_VERSION_3_0"))
      printf("Ready for OpenGL 3.0\n");
   else {
      printf("OpenGL 3.0 not supported\n");
      exit(1);
   }

   initOpenGL(WIDTH, HEIGHT);

   timebase = glutGet(GLUT_ELAPSED_TIME);

   glutMainLoop();

   return 0;
}
