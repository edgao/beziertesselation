#include <GL/glut.h>

// TODO: Add in keyboard commands

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // glBegin(GL_TRIANGLES);
  // go through all the triangles, I guess?
  // TODO: Pass triangle polys from the tesselator thing to here
  // glEnd();

  glutSwapBuffers();
}

void init(void) {
  // TODO: Add lights
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
}

int main(int argc, char **argv) {
  // TODO: Parse cmd args
  // TODO: Maybe move this stuff into another file?
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("surfaces");
  glutDisplayFunc(display);
  init();
  glutMainLoop();
}