#include <GL/glut.h>
#include "Class.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

vector<Triangle*> triangles;
bool flat, wireframe, hidden_line;
// min(x, y, z) to max(x, y, z)
float boundingBox[6];
float cameraMovement = 0.1;
Vector3f camera(0, 0, 0);
float rotateIncrement = 2;
Vector3f rotation(0, 0, 0);

void keyPressed(unsigned char key, int x, int y) {
  switch (key) {
    case '-': // Zoom out
    camera[2] += cameraMovement;
    break;
    case '=': // Zoom in
    case '+':
    camera[2] -= cameraMovement;
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
    hidden_line = !hidden_line;
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
      camera[0] += cameraMovement;
    } else { // Rotate upwards
      rotation[1] -= rotateIncrement;
      rotation[2] -= rotateIncrement;
    }
    break;
    case GLUT_KEY_RIGHT:
    if (glutGetModifiers() && GLUT_ACTIVE_SHIFT > 0) { // Translate upwards
      camera[0] -= cameraMovement;
    } else { // Rotate upwards
      rotation[1] += rotateIncrement;
      rotation[2] += rotateIncrement;
    }
    break;
  }
  glutPostRedisplay();
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glTranslatef(camera[0], camera[1], camera[2]);
  glRotatef(rotation[0], 1, 0, 0);
  glRotatef(rotation[1], 0, 1, 0);
  glRotatef(rotation[2], 0, 0, 1);

  if (wireframe) {
    glDisable(GL_LIGHTING);
  } else {
    glEnable(GL_LIGHTING);
  }

  glBegin(GL_TRIANGLES);
  for (int i = 0; i < triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      Vector3f v;
      if (flat) {
        v = triangles[i]->getFlatNormal();
      } else {
        v = triangles[i]->normals[j];
      }
      glNormal3f(v(0), v(1), v(2));
      v = triangles[i]->vertices[j];
      glVertex3f(v(0), v(1), v(2));

      if (wireframe && hidden_line) {
        // deep magic
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.5, 1.0);
        glColor3f(1, 0, 0);
        glNormal3f(v(0), v(1), v(2));
        v = triangles[i]->vertices[j];
        glVertex3f(v(0), v(1), v(2));
        glColor3f(1, 1, 1);
        glPolygonOffset(0.0, 0.0);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }
    }
  }
  glEnd();

  glPopMatrix();

  glutSwapBuffers();
}

void init(void) {
  GLfloat light_diffuse0[] = {0.3, 0.3, 0.3, 1.0};
  GLfloat light_diffuse1[] = {0.3, 0.3, 0.3, 1.0};
  GLfloat light_diffuse2[] = {0.3, 0.3, 0.3, 1.0};
  GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
  // Put three directional lights in
  GLfloat light0_position[] = { 1.0,  1.0,  1.0, 0.0};
  GLfloat light1_position[] = {-1.0, -1.0,  1.0, 0.0};
  GLfloat light2_position[] = { 0.0,  0.0, -1.0, 0.0};

  GLfloat mat_diffuse[] = { 1, 1, 1, 1.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glEnable(GL_LIGHT1);

  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_LIGHT2);

  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, // FOV
    1.0, // Aspect ratio
    boundingBox[5], // Near z
    boundingBox[2]); // Far z
  glMatrixMode(GL_MODELVIEW);
  gluLookAt((boundingBox[0] + boundingBox[3]) / 2, (boundingBox[1] + boundingBox[4]) / 2, boundingBox[2] - 10,  // eye is at center of the x/y face of the bounding box
    (boundingBox[0] + boundingBox[3]) / 2, (boundingBox[1] + boundingBox[4]) / 2, (boundingBox[2] + boundingBox[5]) / 2,      // center is at (0,0,0)
    0.0, 1.0, 0.0);      // up is in positive Y direction

}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  if (argc < 3) {
    cout << "Usage: as3 file.bez threshold [-a]" << endl;
  }
  float threshold = atof(argv[2]);
  bool adaptive = false;
  if (argc == 4 && strcmp("-a", argv[3]) == 0) {
    adaptive = true;
  }
  fstream input_file (argv[1]);
  vector<BezierPatch*> patches;
  int numPatches = 0;
  if (input_file.is_open()) {
    cout << "File opened successfully!" << endl;
    string line;
    getline(input_file, line);
    // Read in the patches
    numPatches = atoi(line.c_str());
    for (int i = 0; i < numPatches; i++) {
      BezierCurve curves[4];
      for (int j = 0; j < 4; j++) {
        Vector3f points[4];
        getline(input_file, line);
        stringstream ss(line);
        for (int k = 0; k < 4; k++) {
          for (int l = 0; l < 3; l++) {
            string coord;
            do {
              getline(ss, coord, ' ');
            } while (coord.length() == 0);
            points[k](l) = stof(coord);
          }
        }
        curves[j] = BezierCurve(points);
      }
      patches.push_back(new BezierPatch(curves));
      getline(input_file, line);
    }


    // for (int i = 0; i < numPatches; i++) {
    //   BezierCurve curves[4];
    //   // Read in the 4 curves
    //   for (int j = 0; j < 4; j++) {
    //     Vector3f points[4];
    //     // Read in the 4 control points
    //     do {
    //       getline(input_file, line);
    //     } while (line[0] == '\r');
    //     stringstream ss(line);
    //     for (int k = 0; k < 4; k++) {
    //       // Read in the 3 coordinates
    //       float coords[3];
    //       for (int l = 0; l < 3; l++) {
    //         string coord;
    //         do {
    //           getline(ss, coord, ' ');
    //         } while (coord.length() == 0);
    //         coords[l] = stof(coord);
    //       }
    //       points[k] = (Vector3f() << coords[0], coords[1], coords[2]).finished();
    //     }
    //     curves[j] = BezierCurve(points);
    //   }
    //   patches.push_back(new BezierPatch(curves));
    // }
    input_file.close();
  } else{
    cout << "Unable to open file" << endl;
    return 0;
  }
  // Pass the patches to the tesselator
  BezierPatchTesselator tesselator(&patches);
  cout << adaptive << endl;
  triangles = *tesselator.tesselate(adaptive ? BezierPatchTesselator::ADAPTIVE_MODE : BezierPatchTesselator::UNIFORM_MODE, false, threshold);
  cout << "SIZE " << triangles.size() << endl;
  // for (int i = 0; i < triangles.size(); i++) {
  //   Vector3f vertices[3] = triangles[i]->vertices;
  //   for (int j = 0; j < 3; j++) {
  //     for (int k = 0; k < 3; k++) {
  //       if (vertices[j](k) < boundingBox[j]) {
  //         boundingBox[j] = vertices[j](k);
  //       }
  //       if (vertices[j](k) > boundingBox[j + 3]) {
  //         boundingBox[j + 3] = vertices[j](k);
  //       }
  //     }
  //   }
  // }
  // for (int i = 0; i < 3; i++) {
  //   boundingBox[i] -= 1;
  //   boundingBox[i + 3] += 1;
  // }
  boundingBox[0] = -1;
  boundingBox[1] = -1;
  boundingBox[2] = -1;
  boundingBox[3] =  1;
  boundingBox[4] =  1;
  boundingBox[5] =  1;
  flat = wireframe = hidden_line = false;
  glutInitWindowSize(500, 500);
  glutCreateWindow("Bezier surface tesselation");
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyPressed);
  glutSpecialFunc(specialKeyPressed);
  init();
  glutMainLoop();
}