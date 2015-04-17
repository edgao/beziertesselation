#include <GL/glut.h>
#include "Class.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

vector<Triangle*> triangles;
bool flat, wireframe, hidden_line;
// min(x, y, z) to max(x, y, z)
float boundingBox[6];
float cameraMovement = 0.05;
Vector3f camera(0, 0, 0);
float rotateIncrement = 1;
Vector3f rotation(0, 0, 0);

void keyPressed(unsigned char key, int x, int y) {
  switch (key) {
    case '-': // Zoom out
    camera[2] -= cameraMovement;
    break;
    case '=': // Zoom in
    case '+':
    camera[2] += cameraMovement;
    break;
    case 's': // Toggle smooth/flat shading
    flat = !flat;
    if (flat) {
      glShadeModel(GL_FLAT);
    } else {
      glShadeModel(GL_SMOOTH);
    }
    break;
    case 'w': // Toggle filled/wireframe mode
    wireframe = !wireframe;
    if (wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    break;
    case 'h': // Toggle filled/hidden-line mode
    break;
  }
  glutPostRedisplay();
}
void specialKeyPressed(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_UP:
    if (glutGetModifiers() && GLUT_ACTIVE_SHIFT > 0) { // Translate upwards
      camera[1] += cameraMovement;
    } else { // Rotate upwards
      rotation[0] -= rotateIncrement;
    }
    break;
    case GLUT_KEY_DOWN:
    if (glutGetModifiers() && GLUT_ACTIVE_SHIFT > 0) { // Translate upwards
      camera[1] -= cameraMovement;
    } else { // Rotate upwards
      rotation[0] += rotateIncrement;
    }
    break;
    case GLUT_KEY_LEFT:
    if (glutGetModifiers() && GLUT_ACTIVE_SHIFT > 0) { // Translate upwards
      camera[0] -= cameraMovement;
    } else { // Rotate upwards
      rotation[1] -= rotateIncrement;
    }
    break;
    case GLUT_KEY_RIGHT:
    if (glutGetModifiers() && GLUT_ACTIVE_SHIFT > 0) { // Translate upwards
      camera[0] += cameraMovement;
    } else { // Rotate upwards
      rotation[1] += rotateIncrement;
    }
    break;
  }
  glutPostRedisplay();
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_TRIANGLES);
  for (int i = 0; i < triangles.size(); i++) {
    Vector3f flatNormal = triangles[i]->getFlatNormal();
    for (int j = 0; j < 3; j++) {
      Vector3f v = triangles[i]->normals[j];
      if (flat)
        glNormal3f(flatNormal(0), flatNormal(1), flatNormal(2));
      else
        glNormal3f(v(0), v(1), v(2));
      v = triangles[i]->vertices[j];
      glVertex3f(v(0), v(1), v(2));
    }
  }
  glEnd();

  glutSwapBuffers();
}

void init(void) {
  GLfloat light_diffuse[]   = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_specular[]  = {1.0, 1.0, 1.0, 1.0};
  // Put three directional lights in
  GLfloat light0_position[] = { 1.0,  1.0,  1.0, 0.0};
  GLfloat light1_position[] = {-1.0, -1.0,  1.0, 0.0};
  GLfloat light2_position[] = { 0.0,  0.0, -1.0, 0.0};

  GLfloat mat_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glEnable(GL_LIGHT1);

  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_LIGHT2);

  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, // FOV
    1.0, // Aspect ratio
    boundingBox[2], // Near z
    boundingBox[5]); // Far z
  glMatrixMode(GL_MODELVIEW);
  gluLookAt((boundingBox[0] + boundingBox[3]) / 2, (boundingBox[1] + boundingBox[4]) / 2, boundingBox[5] + 10,  // eye is at center of the x/y face of the bounding box, z + 10
    (boundingBox[0] + boundingBox[3]) / 2, (boundingBox[1] + boundingBox[4]) / 2, (boundingBox[2] + boundingBox[5]) / 2,      // center is at (0,0,0)
    0.0, 1.0, 0.0);      // up is in positive Y direction

}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Usage: as3 file.bez threshold [-a]" << endl;
  }
  float threshold = atof(argv[2]);
  bool adaptive = false;
  if (argc < 4 || strcmp("-a", argv[3]) == 0) {
    adaptive = true;
  }
  fstream input_file (argv[1]);
  if (input_file.is_open()) {
    cout << "File opened successfully!" << endl;
    string line;
    getline(input_file, line);
    int numPatches = atoi(line.c_str());
    while (getline(input_file, line)) {
      // TODO Read in the Bezier control points
    }
    input_file.close();
  } else{
    cout << "Unable to open file" << endl;
    return 0;
  }
  // Pass the patches to the tesselator
  // Store the Triangles; modify our boundingBox
  // TODO: Maybe move this stuff into another file?
  flat = wireframe = hidden_line = false;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Bezier surface tesselation");
  glutDisplayFunc(display);
  glutKeyboardFunc(keyPressed);
  glutSpecialFunc(specialKeyPressed);
  init();
  glutMainLoop();
}