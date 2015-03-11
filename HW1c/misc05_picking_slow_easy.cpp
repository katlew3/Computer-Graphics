// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

typedef struct Vertex {
	float XYZW[4];
	float RGBA[4];
	void SetCoords(float *coords) {
		XYZW[0] = coords[0];
		XYZW[1] = coords[1];
		XYZW[2] = coords[2];
		XYZW[3] = coords[3];
	}
	void SetColor(float *color) {
		RGBA[0] = color[0];
		RGBA[1] = color[1];
		RGBA[2] = color[2];
		RGBA[3] = color[3];
	}
};

// ATTN: USE POINT STRUCTS FOR EASIER COMPUTATIONS
typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z){};
	point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]){};
	point operator -(const point& a)const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a)const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a)const {
		return point(x*a, y*a, z*a);
	}
	point operator /(const float& a)const {
		return point(x / a, y / a, z / a);
	}
	float* toArray() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
	Vertex getVertex(point p) {
		return Vertex{ { p.x, p.y, p.z, 1.0f }, { 0.5f, 0.0f, 0.5f, 1.0f } };
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], unsigned short[], size_t, size_t, int);
void createObjects(void);
void pickVertex(void);
void moveVertex(void);
void drawScene(void);
void cleanup(void);
static void mouseCallback(GLFWwindow*, int, int, int);
static void key(GLFWwindow*, int, int, int, int);
void kFunction(int k, int size, int nextSize, point subPoints1[], point subPoints2[], Vertex subVertices[], unsigned short subVertexIndices[]);
point deCasteljau(point coefficients[4], float t);
void catMull(point c[][4], int i, int j, float x, float y, float z);
void copyObjectCoefficients(Vertex rotatedCoefficients[], Vertex deCasteljauCurveCopy[]);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 30;	// number of different "objects" to be drawn
GLuint VertexArrayId[NumObjects], VertexBufferId[NumObjects], IndexBufferId[NumObjects];
size_t NumVert[NumObjects];
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;

/** You have a unit circle length 1.0. (0,0) is in the middle of the screen
* Multiply everything times 2.3 to make the circle a bigger radius
**/
float x = (float)(sqrt(2.0) / 2);
float y = (float)(sqrt(2.0) / 2);
float m = 1.0f;
Vertex Vertices[] = {
	/* ORDER MATTERS - depending on how this was set up - must go in correct order */
	{ { m, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 0
	{ { x * m, y * m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 1
	{ { 0.0f, m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 2
	{ { -x * m, y * m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3
	{ { -m, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 4
	{ { -x*m, -y * m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 5
	{ { 0.0f, -m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 6
	{ { x * m, -y * m, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 7

};
// now change in Picking.vertexshader
unsigned short Indices[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

const size_t IndexCount = sizeof(Indices) / sizeof(unsigned short);
// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS
float pickingColor[IndexCount] = { 0 / 255.0f, 1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f, 5 / 255.0f, 6 / 255.0f, 7 / 255.0f };

//---------------------------- - START OF GLOBAL VARIABLES----------------------------
/* Subdivision Global Variables */
int subDivisionLevel = 0;
const int numPoints = 8, size8 = 8, size16 = 16, size32 = 32, size64 = 64, maxSubdivisions = 5;
Vertex subVertices8[size8], subVertices16[size16], subVertices32[size32], subVertices64[size64];
unsigned short subVertexIndices8[size8], subVertexIndices16[size16], subVertexIndices32[size32], subVertexIndices64[size64];
point subPoints8[size8], subPoints16[size16], subPoints32[size32], subPoints64[size64];

/* Catmull Variables*/
Vertex coefficientVertices[numPoints][4];
unsigned short coefficientVertices_indices[numPoints][4];

Vertex catVerticesFINAL[32];
unsigned short catmullIndicesFINAL[32];

Vertex deCasteljauCurve[120];
unsigned short deCasteljauCurve_indices[120];
bool showTask2 = false;
point coeffPts[numPoints][4];

/* Bezier Curves */

const int numCoefficients = 4;
Vertex bezierCoefficientVertices[numPoints][numCoefficients];
unsigned short bezierCoefficientVertices_indices[numPoints][numCoefficients];
bool showTask3 = false;
point bezierCoefficientPts[numPoints][numCoefficients], tangent;
Vertex bzCoeffVertFINAL[numPoints*numCoefficients];
unsigned short bzCoeffIndicesFINAL[numPoints*numCoefficients];


/* HW 1 c*/
bool shiftPressed = false, showTask4 = false, showTask5;
/* Task 2 */
Vertex rotatedCoefficients[size32];
Vertex deCasteljauCurveCopy[120];

/* Yellow Point Move Along Curve */
Vertex moveAlongCurve[] = { { { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } } };
unsigned short moveAlongIndices[1] = { 0.0f };
Vertex moveAlongCurve2[] = { { { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } } };
unsigned short moveAlongIndices2[1] = { 0.0f };
int pointIDValue = 0;

//---------------------------- - END OF GLOBAL VARIABLES----------------------------
void createObjects(void)
{

	// ATTN: DERIVE YOUR NEW OBJECTS HERE:
	// each has one vertices {pos;color} and one indices array (no picking needed here)

	/* Subdivision */
	//k=0
	for (int i = 0; i < size8; ++i) {

		subVertices8[i] = { { Vertices[i].XYZW[0], Vertices[i].XYZW[1], 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } };
		subVertexIndices8[i] = i;
		subPoints8[i] = point(subVertices8[i].XYZW[0], subVertices8[i].XYZW[1], 0.0f);
	}
	//k=1 
	kFunction(1, size8, size16, subPoints8, subPoints16, subVertices16, subVertexIndices16);
	//k=2
	kFunction(2, size16, size32, subPoints16, subPoints32, subVertices32, subVertexIndices32);
	//k=3
	kFunction(3, size32, size64, subPoints32, subPoints64, subVertices64, subVertexIndices64);

	/*********************************** Catmull-Rom Curves*****************************************/
	
	// c(i,0)
	for (int i = 0; i < numPoints; ++i) {
		catMull(coeffPts, i, 0, Vertices[i].XYZW[0], Vertices[i].XYZW[1], Vertices[i].XYZW[2]);
	}
	
	// c(i,3)
	for (int i = 0; i < numPoints; ++i) {
		if (i == numPoints-1)
			catMull(coeffPts, i, 3, coeffPts[0][0].x, coeffPts[0][0].y, coeffPts[0][0].z);
		else
			catMull(coeffPts, i, 3, coeffPts[i + 1][0].x, coeffPts[i + 1][0].y, coeffPts[i + 1][0].z);
	}

	// c(i,1)
	for (int i = 0; i < numPoints; ++i) {
		if (i == 0){
			tangent = coeffPts[i + 1][0] - coeffPts[numPoints - 1][0];//i-1
		}

		else if (i == numPoints-1)
			tangent = coeffPts[0][0] - coeffPts[i - 1][0]; //i+1 = 0
		else 
			tangent = coeffPts[i + 1][0] - coeffPts[i - 1][0];

		coeffPts[i][1] = coeffPts[i][0] + tangent * 0.2f; // jorg said to multiply * 0.2f in class
	}

	//Find all c(i,2)
	for (int i = 0; i < numPoints; ++i) {
		if (i == numPoints-1)
			coeffPts[i][2] = coeffPts[0][0] * 2 - coeffPts[0][1];
		else
			coeffPts[i][2] = coeffPts[i + 1][0] * 2 - coeffPts[i + 1][1];
	}

	// Initialize coefficient vertices and indices
	for (int i = 0; i < numPoints; ++i) {
		for (int c = 0; c < numCoefficients; ++c) {
			// red line & points
			catVerticesFINAL[numCoefficients * i + c] = { { coeffPts[i][c].x, coeffPts[i][c].y, coeffPts[i][c].z, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } };
			catmullIndicesFINAL[numCoefficients * i + c] = numCoefficients * i + c;
		}
	}

	// Initialize deCasteljau vertices and indices
	for (int i = 0; i < numPoints; ++i) {
		for (int t = 0; t < 15; t++) {
			int currentIndex = 15 * i + t;
			point p = deCasteljau(coeffPts[i], t / 15.0f);
			deCasteljauCurve[currentIndex] = { { p.x, p.y, p.z, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };	// green line
			deCasteljauCurve_indices[currentIndex] = currentIndex;
		}
	}

	/* Bezier */
	// Initialize ci,1 ci,2
	for (int pointVal = 0; pointVal < numPoints; ++pointVal) {
		if (pointVal == numPoints-1) {
			bezierCoefficientPts[pointVal][1] = (point(Vertices[pointVal].XYZW[0], Vertices[pointVal].XYZW[1], 0.0f) * 2 + point(Vertices[0].XYZW[0], Vertices[0].XYZW[1], 0.0f)) / 3;
			bezierCoefficientPts[pointVal][2] = (point(Vertices[pointVal].XYZW[0], Vertices[pointVal].XYZW[1], 0.0f) + point(Vertices[0].XYZW[0], Vertices[0].XYZW[1], 0.0f) * 2) / 3;
		}
		else {
			bezierCoefficientPts[pointVal][1] = (point(Vertices[pointVal].XYZW[0], Vertices[pointVal].XYZW[1], 0.0f) * 2 + point(Vertices[pointVal + 1].XYZW[0], Vertices[pointVal + 1].XYZW[1], 0.0f)) / 3;
			bezierCoefficientPts[pointVal][2] = (point(Vertices[pointVal].XYZW[0], Vertices[pointVal].XYZW[1], 0.0f) + point(Vertices[pointVal + 1].XYZW[0], Vertices[pointVal + 1].XYZW[1], 0.0f) * 2) / 3;
		}
	}
	// Initialize ci,3
	for (int i = 0; i < numPoints; ++i) {
		if (i == numPoints - 1)
			bezierCoefficientPts[i][3] = bezierCoefficientPts[0][0];
		else
			bezierCoefficientPts[i][3] = bezierCoefficientPts[i + 1][0];
	}

	// Initialize ci,0 after knowing ci,1 & ci,2
	for (int i = 0; i < numPoints; ++i) {
		if (i == 0)
			bezierCoefficientPts[i][0] = (bezierCoefficientPts[i][1] + bezierCoefficientPts[numPoints - 1][2]) / 2;
		else 
			bezierCoefficientPts[i][0] = (bezierCoefficientPts[i][1] + bezierCoefficientPts[i - 1][2]) / 2;
	}
	
	
	// Initialize bezier vertices and indices
	for (int i = 0; i < numPoints; ++i) {
		for (int j = 0; j < numCoefficients; ++j) {
			// yellow points
			bezierCoefficientVertices[i][j] = { { bezierCoefficientPts[i][j].x, bezierCoefficientPts[i][j].y, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } };
			bezierCoefficientVertices_indices[i][j] = j;
		}
	}

	// Initialize coefficient vertices and indices
	for (int i = 0; i < numPoints; ++i) {
		for (int c = 0; c < numCoefficients; ++c) {
			// red line & points
			bzCoeffVertFINAL[numCoefficients * i + c] = { { bezierCoefficientPts[i][c].x, bezierCoefficientPts[i][c].y, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } };
			bzCoeffIndicesFINAL[numCoefficients * i + c] = numCoefficients * i + c;
		}
	}

	/*********************************************************************
	 * Make copy Vertices of HW 1 C
	 *********************************************************************/
	copyObjectCoefficients(rotatedCoefficients, deCasteljauCurveCopy);
}

void drawScene(void)
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		//mat4 ModelMatrix = mat4(1.0);
		mat4 ModelMatrix = mat4(1.0); // TranslationMatrix * RotationMatrix;
		ModelMatrix = translate(ModelMatrix, vec3(0, 1.0f, 0.0f));
		mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glEnable(GL_PROGRAM_POINT_SIZE);

		glBindVertexArray(VertexArrayId[0]);	// draw Vertices
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
		//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
		switch (subDivisionLevel) {
			case 1 :
				glBindVertexArray(VertexArrayId[1]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subVertices8), subVertices8);				// update buffer data
				glDrawElements(GL_POINTS, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
				break;
			case 2:
				glBindVertexArray(VertexArrayId[2]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subVertices16), subVertices16);				// update buffer data
				glDrawElements(GL_POINTS, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);
				break;
			case 3:
				glBindVertexArray(VertexArrayId[3]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subVertices32), subVertices32);				// update buffer data
				glDrawElements(GL_POINTS, NumVert[3], GL_UNSIGNED_SHORT, (void*)0);
				break;
			case 4: 
				glBindVertexArray(VertexArrayId[4]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subVertices64), subVertices64);				// update buffer data
				glDrawElements(GL_POINTS, NumVert[4], GL_UNSIGNED_SHORT, (void*)0);
				glDrawElements(GL_LINE_LOOP, NumVert[4], GL_UNSIGNED_SHORT, (void*)0);
				break;
		}
	
		/* Catmull-Rom Curves*/
		if (showTask2) {
			/* Draw red dots & lines */
			glBindVertexArray(VertexArrayId[22]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[22]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(catVerticesFINAL), catVerticesFINAL);		// update buffer data
			glDrawElements(GL_POINTS, NumVert[22], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_LINE_LOOP, NumVert[22], GL_UNSIGNED_SHORT, (void*)0);

			/* draw green circle on control points */
			glBindVertexArray(VertexArrayId[23]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[23]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(deCasteljauCurve), deCasteljauCurve);				// update buffer data
			glDrawElements(GL_LINE_LOOP, NumVert[23], GL_UNSIGNED_SHORT, (void*)0);
		}

		/* Bezier */
		if (showTask3) {

				glBindVertexArray(VertexArrayId[14]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[14]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bzCoeffVertFINAL), bzCoeffVertFINAL);				// update buffer data
				glDrawElements(GL_POINTS, NumVert[14], GL_UNSIGNED_SHORT, (void*)0);
				glDrawElements(GL_LINE_LOOP, NumVert[14], GL_UNSIGNED_SHORT, (void*)0);
		

		}

		/********************************* HW 1 C *******************************/
		// Yellow point
		if (showTask5){
			glBindVertexArray(VertexArrayId[26]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[26]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(moveAlongCurve), moveAlongCurve);		// update buffer data
			glDrawElements(GL_POINTS, NumVert[26], GL_UNSIGNED_SHORT, (void*)0);
		}

		/* You don't want to change the model matrix before you show the curve's yellow point moving around */

		if (showTask4){

			// Translates this about y axis
		
			ModelMatrix = mat4(1.0f);
			ModelMatrix = translate(ModelMatrix, vec3(0.0f, -1.3f, 0.0f));

			// rotate across y axis 
			ModelMatrix = rotate(ModelMatrix, -90.0f, vec3(0, 1, 0));
			MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glm::vec3 lightPos = glm::vec3(4, 4, 4);
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

			glEnable(GL_PROGRAM_POINT_SIZE);

			glBindVertexArray(VertexArrayId[0]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);

			/* Draw red dots & lines */
			glBindVertexArray(VertexArrayId[24]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[24]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rotatedCoefficients), rotatedCoefficients);		// update buffer data
			glDrawElements(GL_POINTS, NumVert[24], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_LINE_LOOP, NumVert[24], GL_UNSIGNED_SHORT, (void*)0);

			/* draw green circle on control points */
			glBindVertexArray(VertexArrayId[25]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[25]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(deCasteljauCurveCopy), deCasteljauCurveCopy);				// update buffer data
			glDrawElements(GL_LINE_LOOP, NumVert[25], GL_UNSIGNED_SHORT, (void*)0);
		}

		/* HW 1 C - Task 4 Bonus - Yellow Point Move Along Vertex */
		if (showTask5) {
		

			if (showTask4){
				/*2nd object */
				glBindVertexArray(VertexArrayId[27]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[27]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(moveAlongCurve2), moveAlongCurve2);		// update buffer data
				glDrawElements(GL_POINTS, NumVert[27], GL_UNSIGNED_SHORT, (void*)0);

			}

		}
		//glBindVertexArray(VertexArrayId[<x>]); etc etc
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickVertex(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		mat4 ModelMatrix = mat4(1.0); // TranslationMatrix * RotationMatrix;
		ModelMatrix = translate(ModelMatrix, vec3(0, 1.0f, 0.0f));
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1fv(pickingColorArrayID, NumVert[0], pickingColor);	// here we pass in the picking marker array

		// Draw the ponts
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(VertexArrayId[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);	// update buffer data
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void moveVertex(void)
{
	mat4 ModelMatrix = mat4(1.0); 
	ModelMatrix = translate(ModelMatrix, vec3(0, 1.0f, 0.0f));
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	glm::vec3 worldCoords = glm::unProject(glm::vec3(window_width - xpos, window_height - ypos, 0.0), ModelMatrix, gProjectionMatrix, vp);

	if (!shiftPressed){
		if (gPickedIndex < IndexCount) {
			Vertices[gPickedIndex].XYZW[0] = worldCoords[0];
			Vertices[gPickedIndex].XYZW[1] = worldCoords[1];
		}

		if (gPickedIndex == 255){ // Full white, must be the background !
			gMessage = "background";
		}
		else {
			std::ostringstream oss;
			oss << "point " << gPickedIndex;
			gMessage = oss.str();
		}
	}

	/* HW 1 C, if shiftPressed == true
	 * Map the z to the y coordinate */
	if (shiftPressed) {
		if (gPickedIndex < IndexCount) {
			// z gets mapped to y
			Vertices[gPickedIndex].XYZW[2] = worldCoords[1];
		}

		if (gPickedIndex == 255){ // Full white, must be the background !
			gMessage = "background";
		}
		else {
			std::ostringstream oss;
			oss << "point " << gPickedIndex;
			gMessage = oss.str();
		}

	}


}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Lew,Kathleen(1218-8386)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(
		glm::vec3(0, 0, -5), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	createVAOs(Vertices, Indices, sizeof(Vertices), sizeof(Indices), 0);
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	/* Subdivision VAOs */
	createVAOs(subVertices8, subVertexIndices8, sizeof(subVertices8), sizeof(subVertexIndices8), 1);
	createVAOs(subVertices16, subVertexIndices16, sizeof(subVertices16), sizeof(subVertexIndices16), 2);
	createVAOs(subVertices32, subVertexIndices32, sizeof(subVertices32), sizeof(subVertexIndices32), 3);
	createVAOs(subVertices64, subVertexIndices64, sizeof(subVertices64), sizeof(subVertexIndices64), 4);

	/* Catmull Rom VAOs */
	createVAOs(catVerticesFINAL, catmullIndicesFINAL, sizeof(catVerticesFINAL), sizeof(catmullIndicesFINAL), 22);
	createVAOs(deCasteljauCurve, deCasteljauCurve_indices, sizeof(deCasteljauCurve), sizeof(deCasteljauCurve_indices), 23);

	/* Bezier VAOs */
	for (int i = 0; i < numPoints; ++i)
		createVAOs(coefficientVertices[i], coefficientVertices_indices[i], sizeof(coefficientVertices[i]), sizeof(coefficientVertices_indices[i]), 5 + i);
	createVAOs(bzCoeffVertFINAL, bzCoeffIndicesFINAL, sizeof(bzCoeffVertFINAL), sizeof(bzCoeffIndicesFINAL), 14);

	/* HW 1 C VAOs Task 2 */
	createVAOs(rotatedCoefficients, catmullIndicesFINAL, sizeof(rotatedCoefficients), sizeof(catmullIndicesFINAL), 24);
	createVAOs(deCasteljauCurveCopy, deCasteljauCurve_indices, sizeof(deCasteljauCurveCopy), sizeof(deCasteljauCurve_indices), 25);

	/* HW 1 C VAOs Task 3 - Move Along Vertex  */
	createVAOs(moveAlongCurve, moveAlongIndices, sizeof(moveAlongCurve), sizeof(moveAlongIndices), 26);
	createVAOs(moveAlongCurve2, moveAlongIndices2, sizeof(moveAlongCurve2), sizeof(moveAlongIndices2), 27);

}

void createVAOs(Vertex Vertices[], unsigned short Indices[], size_t BufferSize, size_t IdxBufferSize, int ObjectId) {

	NumVert[ObjectId] = IdxBufferSize / (sizeof GLubyte);

	GLenum ErrorCheckValue = glGetError();
	size_t VertexSize = sizeof(Vertices[0]);
	size_t RgbOffset = sizeof(Vertices[0].XYZW);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	glGenBuffers(1, &IndexBufferId[ObjectId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IdxBufferSize, Indices, GL_STATIC_DRAW);

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickVertex();
	}
}
static void key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_1 && action == GLFW_PRESS && subDivisionLevel < maxSubdivisions){
		subDivisionLevel++;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		showTask2 = (showTask2 == true ? false : true);
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		showTask3 = (showTask3 == true ? false : true);
	}
	if ((key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) || (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS)){
		shiftPressed = true;
	}
	if ((key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) || (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_RELEASE)){
		shiftPressed = false;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		showTask4 = (showTask4 == true ? false : true);
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		showTask5 = (showTask5 == true ? false : true);
	}
}

int main(void)
{
	/* Initialize NumObjects */
	for (int i = 0; i < NumObjects; ++i) {
		VertexArrayId[i] = i;
		VertexBufferId[i] = i;
		IndexBufferId[i] = i;
		NumVert[i] = i;
	}
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		/* Yellow position increments */
		// 4 = xyzw, only need to do 3 b/c w never changes
			for (int i = 0; i < 3; ++i){
				if (pointIDValue >= 120){
					pointIDValue %= 120;
				}
				moveAlongCurve[0].XYZW[i] = deCasteljauCurve[pointIDValue].XYZW[i];
				moveAlongCurve2[0].XYZW[i] = deCasteljauCurveCopy[pointIDValue].XYZW[i];
			}

			++pointIDValue;
		
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			// printf and reset
			//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;

			/* Task 3 - Yellow Point Move */
			/* Move Along Curve */
			
		}

		// DRAGGING: move current (picked) vertex with cursor
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			moveVertex();
		if (glfwSetKeyCallback(window, &key)){
			/* window, key, scancode, action, mods*/
		}

		// DRAWING SCENE
		createObjects();	// re-evaluate curves in case vertices have been moved
		drawScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}

/* De Casteljau Function

take in 4 coefficients, build the tree w/ the four */
point deCasteljau(point coefficients[4], float t) {
	point deCasteljauResult;
	point a[3] = {
		coefficients[0] * (1.0f - t) + coefficients[1] * t,
		coefficients[1] * (1.0f - t) + coefficients[2] * t,
		coefficients[2] * (1.0f - t) + coefficients[3] * t,
	};
	point b[2] = {
		a[0] * (1.0f - t) + a[1] * t,
		a[1] * (1.0f - t) + a[2] * t,
	};
	deCasteljauResult = b[0] * (1.0f - t) + b[1] * t;
	return deCasteljauResult;
};


/* K Subdivision Level function */
void kFunction(int k, int size, int nextSize, point subPoints1[], point subPoints2[], Vertex subVertices[], unsigned short subVertexIndices[]) {
	for (int i = 0; i < size; ++i){
		subPoints2[2 * i] = (subPoints1[((i - 1) % size + size) % size] * 4 + subPoints1[i] * 4) / 8;
		subPoints2[2 * i + 1] = (subPoints1[((i - 1) % size + size) % size] + subPoints1[i] * 6 + subPoints1[(i + 1) % size]) / 8;
	}
	for (int i = 0; i < nextSize; ++i) {
		subVertices[i] = { { subPoints2[i].x, subPoints2[i].y, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } };
		subVertexIndices[i] = i;
	}
};


void catMull(point c[][4], int i, int j, float x, float y, float z)
{
	c[i][j] = point(x, y, z);
};

/* HW 1 C Make copy of both objects */
void copyObjectCoefficients(Vertex rotatedCoefficients[], Vertex deCasteljauCurveCopy[]){
	for (int i = 0; i < size32; ++i) {
		for (int colorPositionID = 0; colorPositionID < 4; ++colorPositionID) {
			rotatedCoefficients[i].RGBA[colorPositionID] = catVerticesFINAL[i].RGBA[colorPositionID];
			rotatedCoefficients[i].XYZW[colorPositionID] = catVerticesFINAL[i].XYZW[colorPositionID];
		}
	}
	for (int i = 0; i < 120; ++i) {
		for (int colorPositionID = 0; colorPositionID < 4; ++colorPositionID) {
			deCasteljauCurveCopy[i].RGBA[colorPositionID] = deCasteljauCurve[i].RGBA[colorPositionID];
			deCasteljauCurveCopy[i].XYZW[colorPositionID] = deCasteljauCurve[i].XYZW[colorPositionID];
		}
	}
}
