// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
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

/* Kat added includes */
#include <string>
using namespace std;

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;


const GLuint NumObjects = 30;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects], VertexBufferId[NumObjects], IndexBufferId[NumObjects];

size_t NumIndices[NumObjects], VertexBufferSize[NumObjects], IndexBufferSize[NumObjects];

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
/* Kat Modified for Task 5 */
GLuint LightID2;
GLuint selectedColorID;
vec3 selectedColor;

GLint gX = 0.0;
GLint gZ = 0.0;

// animation control
bool animation = false;
GLfloat phi = 0.0;

/*----------------------- Kat Modified Global Variables----------------------*/
float w = 1.0f;
const int gridLength = 11;		// 10 pts -> length of 11
Vertex gridVertsZ[gridLength][gridLength], gridVertsX[gridLength][gridLength];

/* Task 2 - Camera Rotations */
int lastPressedKey = -1;
bool moveHorizontal = false;

float radius = 17.0f;
float cameraVerticleAngle = 45.0f;
float cameraHorizontalAngle = 30.0f;

/* Initialize the view - taken from openGL Tutorial 6 */
float viewX = radius * cos(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
float viewY = radius * cos(radians(cameraVerticleAngle));
float viewZ = radius * sin(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));

/* Camera lookAt Function */
vec3 center = vec3(0.0, 0.0, 0.0);
vec3 up = vec3(0.0, 1.0, 0.0);
vec3 eye = vec3(viewX, viewY, viewZ);

/* Task 3 & 4 -  Haptic object data with translate & rotate vectors */
int baseSize = 0, topSize = 0, arm1Size = 0, jointSize = 0, arm2Size = 0, penSize = 0, buttonSize = 0;

typedef struct {
	vec3 translate;
	vec3 rotate;
} Haptic;

Haptic haptic[7] = {
	{ center, center },
	{ center, center },
	{ center, center },
	{ center, center },
	{ center, center },
	{ center, center },
	{ center, center },
};

char *path = "models/";

Vertex* Vertices;
Vertex* Vertices1;
Vertex* Vertices2;
Vertex* Vertices3;
Vertex* Vertices4;
Vertex* Vertices5;
Vertex* Vertices6;
Vertex* Vertices7;

GLushort* Indices;
GLushort* Indices1;
GLushort* Indices2;
GLushort* Indices3;
GLushort* Indices4;
GLushort* Indices5;
GLushort* Indices6;
GLushort* Indices7;

vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
vec4 pink = vec4(1.0, 0.0, 1.0, 1.0);
vec4 cyan = vec4(0.0, 1.0, 1.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);

vec3 red_ = vec3(1.0, 0.0, 0.0);
vec3 green_ = vec3(0.0, 1.0, 0.0);
vec3 blue_ = vec3(0.0, 0.0, 1.0);
vec3 pink_ = vec3(1.0, 0.0, 1.0);
vec3 cyan_ = vec3(0.0, 1.0, 1.0);
vec3 yellow_ = vec3(1.0, 1.0, 0.0);

/* Task 5 - Light up the scene variables */
GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 0.2f };
GLfloat lightPosition[2][4] = { { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f} };

// function prototypes
int initWindow(void);
void initOpenGL(void);
int loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);

/*-------------Kat added function prototypes ----------------*/
/* Task 1 - Grid */
void createGridVertices(Vertex gridVertsZ[gridLength][gridLength], Vertex gridVertsX[gridLength][gridLength]);
/* Task 2 & 4 - Camera */
void updateView(bool moveHorizontal, float &viewX, float &viewY, float &viewZ);
void moveUp(int lastKeyPressed);
void moveDown(int lastKeyPressed);
void moveLeft(int lastKeyPressed);
void moveRight(int lastKeyPressed);
/* Task 5 - Color */
void activeColor(Vertex*, Vertex*, Vertex*, Vertex*, Vertex*, Vertex*, Vertex*);


int loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort)* idxCount;

	return (int)indexed_vertices.size();
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	/*------- create grid vertices --------*/
	createGridVertices(gridVertsZ, gridVertsX);

	//-- .OBJs --//

	// ATTN: load your models here

	/* RED BASE */
	baseSize = loadObject("models/base.obj", red, Vertices, Indices, 23);
	createVAOs(Vertices, Indices, 23);

	haptic[0].translate = vec3(0.0f, 0.4f, 0.0f);

	/* GREEN BASE */
	topSize = loadObject("models/top.obj", green, Vertices2, Indices2, 24);
	createVAOs(Vertices2, Indices2, 24);

	haptic[1].translate = vec3(0.0f, 0.9f, 0.0f);

	/* BLUE ARM */
	arm1Size = loadObject("models/arm1.obj", blue, Vertices3, Indices3, 25);
	createVAOs(Vertices3, Indices3, 25);

	haptic[2].translate = vec3(0.0f, 2.0f, 0.0f);
	haptic[2].rotate = vec3(0.0f, 0.0f, -45.0f);

	/* PINK JOINT */
	jointSize = loadObject("models/joint.obj", pink, Vertices4, Indices4, 26);
	createVAOs(Vertices4, Indices4, 26);

	haptic[3].translate = vec3(0.0f, 1.0f, 0.0f);

	/* CYAN ARM */
	arm2Size = loadObject("models/arm2.obj", cyan, Vertices5, Indices5, 27);
	createVAOs(Vertices5, Indices5, 27);

	haptic[4].translate = vec3(0.0f, 1.0f, 0.0f);

	/* YELLOW PEN */
	penSize = loadObject("models/pen.obj", yellow, Vertices6, Indices6, 28);
	createVAOs(Vertices6, Indices6, 28);

	haptic[5].translate = vec3(0.0f, 0.2f, 0.0f);
	haptic[5].rotate = vec3(0.0f, 0.0f, 45.0f);

	/* RED BUTTON */
	buttonSize = loadObject("models/button.obj", red, Vertices7, Indices7, 29);
	createVAOs(Vertices7, Indices7, 29);

	haptic[6].translate = vec3(0.0f, -0.05f, 0.0f);
	haptic[6].rotate = vec3(0.0f, 0.0f, 90.0f);

}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		/* Kat modified light */
		vec3 lightPos2 = vec3(-1, 1, 1);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);

		glUniform3f(selectedColorID, selectedColor[0], selectedColor[1], selectedColor[2]);
		/**********************/
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		/* Kat Modified Grid */
		for (int i = 0; i < gridLength; ++i){
			glBindVertexArray(VertexArrayId[i + 1]);	/* Grid Z */
			glDrawArrays(GL_LINE_LOOP, 0, 11);
			glBindVertexArray(VertexArrayId[i + 12]);	/* Grid X */
			glDrawArrays(GL_LINE_LOOP, 0, 11);
		}


		//Draw objects

		/* Base - 23 */
		ModelMatrix = translate(ModelMatrix, haptic[0].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[23]);
		glDrawElements(GL_TRIANGLES, baseSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Top - 24 */
		ModelMatrix = translate(ModelMatrix, haptic[1].translate);
		ModelMatrix = rotate(ModelMatrix, haptic[1].rotate[1], vec3(0, 1, 0));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[24]);
		glDrawElements(GL_TRIANGLES, topSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Arm1 - 25 */
		ModelMatrix = rotate(ModelMatrix, haptic[2].rotate[2], vec3(0, 0, 1));
		ModelMatrix = translate(ModelMatrix, haptic[2].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[25]);
		glDrawElements(GL_TRIANGLES, arm1Size * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Joint - 26 */
		ModelMatrix = translate(ModelMatrix, haptic[3].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[26]);
		glDrawElements(GL_TRIANGLES, jointSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Arm2 - 27 */
		ModelMatrix = rotate(ModelMatrix, haptic[4].rotate[2], vec3(0, 0, 1));
		ModelMatrix = translate(ModelMatrix, haptic[4].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[27]);
		glDrawElements(GL_TRIANGLES, arm2Size * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Pen - 28 */
		ModelMatrix = translate(ModelMatrix, vec3(0.0f, 0.6f, 0.0f));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[2], vec3(0, 0, 1));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[1], vec3(0, 1, 0));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[0], vec3(1, 0, 0));
		ModelMatrix = translate(ModelMatrix, haptic[5].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[28]);
		glDrawElements(GL_TRIANGLES, penSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Button - 29 */
		ModelMatrix = rotate(ModelMatrix, haptic[6].rotate[2], vec3(0, 0, 1));
		ModelMatrix = translate(ModelMatrix, haptic[6].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[29]);
		glDrawElements(GL_TRIANGLES, buttonSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		// always last
		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		mat4 ModelMatrix = mat4(1.0); // TranslationMatrix * RotationMatrix;
		mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		//// draw Base
		//glBindVertexArray(X);	
		//	glUniform1f(pickingColorID, Y / 255.0f); // here we pass in the picking marker
		//	glDrawElements(Z);
		//// draw Top
		//glBindVertexArray(XX);	
		//	glUniform1f(pickingColorID, YY / 255.0f); // here we pass in the picking marker
		//	glDrawElements(ZZ);
		//glBindVertexArray(0);
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

	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
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
	window = glfwCreateWindow(window_width, window_height, "Lew, Kathleen 1218-8386", NULL, NULL);
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
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	/* Kat modified */
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace2");
	selectedColorID = glGetUniformLocation(programID, "selectedColor");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

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

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_C:			// select Camera
			lastPressedKey = key;
			
			break;
		case GLFW_KEY_1:			// select Base
			lastPressedKey = key;
			selectedColor = red_;
			break;
		case GLFW_KEY_2:
			lastPressedKey = key;	// select Top
			selectedColor = green_;
			break;
		case GLFW_KEY_3:
			lastPressedKey = key;	// select arm1
			selectedColor = blue_;
			break;
		case GLFW_KEY_4:
			lastPressedKey = key;	// select arm2
			selectedColor = cyan_;
			break;
		case GLFW_KEY_5:
			lastPressedKey = key;	// select pen
			selectedColor = yellow_;
			break;
		/*case GLFW_KEY_UP:
			moveUp(lastPressedKey);
			break;
		case GLFW_KEY_DOWN:
			moveDown(lastPressedKey);
			break;
		case GLFW_KEY_LEFT:
			moveLeft(lastPressedKey);
			break;
		case GLFW_KEY_RIGHT:
			moveRight(lastPressedKey);
			break;*/
		default:
			break;

		}
	}

	
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

/*------------ Update viewX, viewY, viewZ function taken from Tutorial 6 openGL------------*/
void updateView(bool moveHorizontal, float &viewX, float &viewY, float &viewZ){
	if (!moveHorizontal){
		viewY = radius * cos(radians(cameraVerticleAngle));
	}
	viewX = radius * cos(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
	viewZ = radius * sin(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
}
/* ---------------Camera Action -----------------*/
void moveUp(int lastPressedKey) {
	switch (lastPressedKey) {
		case GLFW_KEY_C :
			moveHorizontal = false;
			/* go in reverse direction */
			cameraVerticleAngle -= 5.0f;
			updateView(moveHorizontal, viewX, viewY, viewZ);
			/* update the eye, center is always 0, up is always +y */
			eye = vec3(viewX, viewY, viewZ);
			gViewMatrix = lookAt(eye, center, up);
			break;

		case GLFW_KEY_1:
			haptic[0].translate -= vec3(0.0f, 0.0f, 0.25f);
			break;

		case GLFW_KEY_3:
			haptic[2].rotate -= vec3(0.0f, 0.0f, 5.0f);
			break;

		case GLFW_KEY_4:
			haptic[4].rotate -= vec3(0.0f, 0.0f, 5.0f);
			break;

		case GLFW_KEY_5:
			haptic[5].rotate -= vec3(0.0f, 0.0f, 5.0f);
			break;

		default:
			break;
	}

}

void moveDown(int lastPressedKey) {
	switch (lastPressedKey) {
		case GLFW_KEY_C:
			moveHorizontal = false;
			/* go in reverse direction */
			cameraVerticleAngle += 5.0f;
			updateView(moveHorizontal, viewX, viewY, viewZ);
			/* update the eye */
			eye = vec3(viewX, viewY, viewZ);
			gViewMatrix = lookAt(eye, center, up);
			break;

		case GLFW_KEY_1:
			haptic[0].translate += vec3(0.0f, 0.0f, 0.25f);
			break;

		case GLFW_KEY_3:
			haptic[2].rotate += vec3(0.0f, 0.0f, 5.0f);
			break;

		case GLFW_KEY_4:
			haptic[4].rotate += vec3(0.0f, 0.0f, 5.0f);
			break;

		case GLFW_KEY_5:
			haptic[5].rotate += vec3(0.0f, 0.0f, 5.0f);
			break;

		default:
			break;
	}
}

void moveLeft(int lastPressedKey) {
	switch (lastPressedKey) {
		case GLFW_KEY_C:
			moveHorizontal = true;
			/* go in reverse direction */
			cameraHorizontalAngle += 5.0f;
			updateView(moveHorizontal, viewX, viewY, viewZ);
			/* update the eye */
			eye = vec3(viewX, viewY, viewZ);
			gViewMatrix = lookAt(eye, center, up);
			break;

		case GLFW_KEY_1:
			haptic[0].translate += vec3(0.25f, 0.0f, 0.0f);
			break;

		case GLFW_KEY_2:
			haptic[1].rotate -= vec3(0.0f, 25.0f, 0.0f);
			break;

		case GLFW_KEY_5:
			haptic[5].rotate -= vec3(5.0f, 0.0f, 0.0f);
			break;

		default:
			break;
	}

}

void moveRight(int lastPressedKey) {
	switch (lastPressedKey) {
		case GLFW_KEY_C:
			moveHorizontal = true;
			cameraHorizontalAngle -= 5.0f;
			updateView(moveHorizontal, viewX, viewY, viewZ);
			/* update the eye */
			eye = vec3(viewX, viewY, viewZ);
			gViewMatrix = lookAt(eye, center, up);
			break;

		case GLFW_KEY_1:
			haptic[0].translate -= vec3(0.25f, 0.0f, 0.0f);
			break;

		case GLFW_KEY_2:
			haptic[1].rotate += vec3(0.0f, 25.0f, 0.0f);
			break;

		case GLFW_KEY_5:
			haptic[5].rotate += vec3(5.0f, 0.0f, 0.0f);
			break;

		default:
			break;
	}
}

/* -----------------Create Grid Vertices-------------- */
void createGridVertices(Vertex gridVertsZ[gridLength][gridLength], Vertex gridVertsX[gridLength][gridLength]){
	int x_ = 0, z_ = 0;
	for (float x = -5.0f; x <= 5.0f; x += 1.0f){
		for (float z = -5.0f; z <= 5.0f; z += 1.0f){
			gridVertsZ[x_][z_] = { { x, 0.0, z, w }, { 1.0, 1.0, 1.0, w }, { 0.0, 0.0, 1.0 } };
			++z_;
		}
		++x_;
		z_ = 0;
	}

	for (int x = 0; x < 11; ++x) {
		for (int z = 0; z < 11; ++z){
			gridVertsX[x][z] = gridVertsZ[z][x];
		}
	}

	for (int i = 1; i <= 11; ++i) {
		VertexBufferSize[i] = sizeof(gridVertsZ[i - 1]);
		createVAOs(gridVertsZ[i - 1], NULL, i);

		VertexBufferSize[i + 11] = sizeof(gridVertsX[i - 1]);
		createVAOs(gridVertsX[i - 1], NULL, i + 11);
	}
}

int main(void)
{
	/* Kat Modification - Initialize NumObjects */
	for (int i = 0; i < NumObjects; ++i) {
		VertexArrayId[i] = i;
		VertexBufferId[i] = i;
		IndexBufferId[i] = i;
		NumIndices[i] = i;
		VertexBufferSize[i] = i;
		IndexBufferSize[i] = i;
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
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}

		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}
		int movementKey = -1;
		if (GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_UP))
				moveUp(lastPressedKey);
			if (glfwGetKey(window, GLFW_KEY_DOWN))
				moveDown(lastPressedKey);
			if (glfwGetKey(window, GLFW_KEY_LEFT))
				moveLeft(lastPressedKey);
			if (glfwGetKey(window, GLFW_KEY_RIGHT))
				moveRight(lastPressedKey);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				if (lastPressedKey == GLFW_KEY_5) {
					haptic[5].rotate -= vec3(0.0f, 5.0f, 0.0f);
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				if (lastPressedKey == GLFW_KEY_5) {
					haptic[5].rotate += vec3(0.0f, 5.0f, 0.0f);
				}
			}
		}

		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
