CC=g++
CFLAGS= -g

ifeq ($(shell uname),Darwin)
	LDFLAGS=-framework GLUT -framework OpenGL
else
	LDFLAGS= -lglut -lGL -lGLU -lm
endif

all: hw1a

hw1a: hw1a.o
	$(CC) hw1a.o $(LDFLAGS) -o hw1a


