// homework for computer graphics, UFL

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <iostream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using std::copy;


// CONTSTANTS
const int window_width = 500, window_height = 500;
const char * window_title = "HW1";
const float view_width = 3, view_height = 3;
const float point_size = 10.0, line_width = 1;
const float PI = 3.1415923565;
const float point_color[] = { 0.0f, 1.0f, 1.0f };
const float line_color[]  = { 1.0f, 1.0f, 1.0f };
const float  animation_step = .5;
const int TIMERMSECS = 20;
const float radius = 1;
const int ncpoints =  8;

// RUNTIME VARIABLES
float cpoints[ncpoints][3];

float animation_time = 0;
bool animation_enabled = true;

// some prototypes

void graphics_init();
void initialize();
void draw_objects();


#ifndef min
float min(float a,float b){
	return a < b ? a : b;
}
#endif

// picking and dragging functions  *****************************

void screen_to_point(int x,int y,float d[3]){
	// This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
	double mvmatrix[16];
	double projmatrix[16];
	int viewport[4];
	double dX, dY, dZ, dClickY; // glUnProject uses doubles, but I'm using floats for these 3D vectors

	glGetIntegerv(GL_VIEWPORT, viewport);	
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	int g_WindowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	dClickY = double (g_WindowHeight - y); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	gluUnProject ((double) x, dClickY, 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);

	d[0] = (float) dX;
	d[1] = (float) dY;
	d[2] = (float) dZ;
}


int color_code_pick(int x,int y){
	
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw_objects();
	glFlush();
	GLubyte data;
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT,viewport);
	glReadPixels(x,viewport[3]-y,1,1,GL_GREEN,GL_UNSIGNED_BYTE,&data);
	return data != 0 ? 0 : -1;
}


int current_point_i = 0;
float previous_mouse_point[3];

void mouse(int button, int state, int x, int y ){
	if(state == GLUT_DOWN){
		screen_to_point(x,y,previous_mouse_point);
		current_point_i = color_code_pick(x,y);
	}else{
		current_point_i = -1;
	}
}


void motion( int x, int y){
	float current_mouse_point[3];
	screen_to_point(x,y,current_mouse_point);
	float dx = current_mouse_point[0] - previous_mouse_point[0];
	float dy = current_mouse_point[1] - previous_mouse_point[1];
	float dz = current_mouse_point[2] - previous_mouse_point[2];

	if(current_point_i != -1){
		cpoints[current_point_i][0] += dx;
		cpoints[current_point_i][1] += dy;
		cpoints[current_point_i][2] += dz;
		glutPostRedisplay();
	}

	copy(current_mouse_point,current_mouse_point+3,previous_mouse_point);
}

// KEYBOARD handling *******************************************

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case ' ':
			animation_enabled = ! animation_enabled;
			break;
		case 'r':
			graphics_init();
			initialize();
			glutPostRedisplay();
			break;
		case 27:
			exit(0);
			break;
	}
}

// DISPLAY and RENDERING functions *************************



void draw_objects(){
	// Draw all points
	glPointSize(point_size);

	// reassign value to point color 
	// randomize the point color object
	// change point_color 
	glColor3fv(point_color);
	glBegin(GL_POINTS);
	for(int i = 0; i < ncpoints; i++){
		glVertex3fv( cpoints[i] );
	}
	glEnd();
}

void display(){
	// Clear Viewport
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw_objects();
	glFlush();
	glutSwapBuffers();

}

//! Rotate the given point in place by the angle given around origin using simple matrix rotation.
void rotate_point(float p[3], const float angle){
	float x,y;
	x = p[0] *  cos(angle) + p[1] * sin(angle);
	y = p[0] * -sin(angle) + p[1] * cos(angle);
	p[0] = x; p[1] = y;
}


//! Animation callback function. Called by GLUT periodically
void animate(int t){
	float time_elapsed = TIMERMSECS/1000.0;

	float current_step = animation_step* time_elapsed;

	glutTimerFunc(TIMERMSECS, animate, 0);

	if(animation_enabled){
		if(current_step < animation_step*2) // we never want a big step
		{
			// Rotate all points by the given step
			for(int i=0;i<ncpoints;i++){
				rotate_point(cpoints[i],current_step);
			}
		}
		glutPostRedisplay();
	}

}

void reshape(int width, int height){
	// Clip the view port to match our ratio
	int unit = min(width/view_width,height/view_height);
	int wp =(width-view_width*unit)/2, hp = (height-view_height*unit)/2;
	glViewport(wp,hp,view_width*unit,view_height*unit);
	glutPostRedisplay();
}

void graphics_init(){
	// Initializing Projection Matrix
	// Locating Camera and setting viewport
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-view_width/2,view_width/2,-view_height/2,view_height/2,-view_height,view_height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Antialiasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

float frand(){
	return (rand()%1000)/1000.0f;
}

// Set up runtime variables
void initialize(){
	// spread points on the screen
	for(int i = 0; i < ncpoints; i++){
		cpoints[i][0] = frand()*2-1;
		cpoints[i][1] = frand()*2-1;
		cpoints[i][2] = 0;
	}
	
}

int main(int argc,char * argv[]){
	srand(time(NULL));
	glutInit(&argc,argv);

	// Setup GLUT
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB     | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize (window_width,window_height);

	glutCreateWindow (window_title);

	// Initialize OpenGL
	graphics_init ();

	// Initialize Runtime variables
	initialize();

	// callbacks
	glutReshapeFunc(&reshape);
	glutDisplayFunc(&display);
	glutKeyboardFunc(&keyboard);
	glutMouseFunc(&mouse);
	glutMotionFunc(&motion);
	//glutIdleFunc(&animate);
	glutTimerFunc(TIMERMSECS, animate, 0);



	// main loop
	glutMainLoop();
	return 0;
}
