.PHONY: all

all:
	g++ -g -DGL_GLEXT_PROTOTYPES -I../lib/glut-3.7.6-bin -c main.cpp -o ../exec/main.o
	g++ -g -DGL_GLEXT_PROTOTYPES -I../lib/glut-3.7.6-bin -o ../exec/as3 ../exec/main.o -lglut -lGL -lGLU
