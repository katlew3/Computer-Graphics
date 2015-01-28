#include <stdio.h>		// Include standard headers
#include <stdlib.h>
#include <GL/glew.h>	// Include GLEW
#include <glfw3.h>		// Include GLFW
GLFWwindow* window;
#include <glm/glm.hpp>	// Include GLM
using namespace glm;
#include <common/shader.hpp>
#include <vector>		// display points

// CONSTANTS
const int ncpoints = 6;
const float point_color[] = { 0.0f, 1.0f, 1.0f };

// RUNTIME VARIABLES
float cpoints[ncpoints][3];
float cpointsColor[ncpoints][3];

// start of functions
int initializeGLFW();
int openWindow();
int initializeGLEW();

void initialize();
float frand();
void draw_objects();
void idToRGB(int i, int &r, int &g, int &b);

int main(void)
{
	initializeGLFW();
	openWindow();
	initializeGLEW();

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// set black background
	glClearColor(0.3f, 0.3f, 0.3f, 0.3f);

	// Create vertex array object
	GLuint VertexArrayID;

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	initialize();
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};
	
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cpoints), cpoints, GL_STATIC_DRAW);

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		draw_objects();

		// Draw the triangle !
		// glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

		glDrawArrays(GL_POINTS, 0, 6);
		glEnable(GL_PROGRAM_POINT_SIZE);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

int initializeGLFW() {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

}
int openWindow() {
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Kathleen Lew 1218-8386 HW 1A", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
}
int initializeGLEW() {
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
}

float frand(){
	return (rand() % 1000) / 1000.0f;
}
// Set up runtime variables
void initialize(){
	// spread points on the screen
	// x,y,z

	int red, green, blue;
	for (int i = 0; i < ncpoints; i++){
		cpoints[i][0] = frand() * 2 - 1;
		cpoints[i][1] = frand() * 2 - 1;
		cpoints[i][2] = 0;

		idToRGB(i, red, green, blue);

		// initialize points w/ random colors
		cpointsColor[i][0] = red;
		cpointsColor[i][1] = green;
		cpointsColor[i][2] = blue;
	}


}

void draw_objects(){
	// reassign value to point color 
	// randomize the point color object
	// change point_color 
	glColor3fv(point_color);
	glBegin(GL_POINTS);
	for (int i = 0; i < ncpoints; i++){
		glVertex3fv(cpoints[i]);
	}
	glEnd();
}

void idToRGB(int i, int &r, int &g, int &b) {
	// Convert "i", the integer mesh ID, into an RGB color
	
	// DOESN'T WORK 
	/*r = (i && 0x000000FF) >> 0;
	 g = (i && 0x0000FF00) >> 8;
	 b = (i && 0x00FF0000) >> 16;*/

	 r = rand() % 255;
	 g = rand() % 255;
	 b = rand() % 255;

}
/*void intToFloat(int pickingColorID, ){
	glUniform3f(pickingColorID, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}*/