#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <AntTweakBar.h>
#include <math.h>
#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <iostream>							// Files
#include <fstream>
#include <string>
#include <string.h>
		// Ray Casting
using namespace std; using namespace glm;
const int window_width = 1200, window_height = 800;
typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	float UVs[2];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0f;
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
	void SetUVs(float *uvs) {
		UVs[0] = uvs[0];
		UVs[1] = uvs[1];
	}
};

/*----------------------- Kat Modified Global Variables----------------------*/
const GLuint NumObjects = 68; string gMessage; GLFWwindow* window; mat4 gProjectionMatrix, gViewMatrix;
GLuint programID, pickingProgramID, meshProgramID, gPickedIndex = -1, gPickedIndexG = -1, VertexArrayId[NumObjects], VertexBufferId[NumObjects], IndexBufferId[NumObjects];
size_t NumIndices[NumObjects], VertexBufferSize[NumObjects], IndexBufferSize[NumObjects];
GLuint MatrixID, ModelMatrixID, ViewMatrixID, ProjMatrixID, PickingMatrixID, pickingColorID, pickingColorIDG, LightID, selectedColorID;
// animation control
bool animation = false; GLfloat phi = 0.0; GLint gX = 0.0, gZ = 0.0;
/*----------- Grid -----------*/
float w = 1.0f; const int gridLength = 21;              // 20 pts -> length of 11
Vertex gridVertsZ[gridLength][gridLength], gridVertsX[gridLength][gridLength];
void createGridVertices(Vertex gridVertsZ[gridLength][gridLength], Vertex gridVertsX[gridLength][gridLength]);
void bindWhiteGrid();
/*----------- Control Mesh -----------*/
const int controlMeshSize = 13*13;
Vertex standardMesh[controlMeshSize];
/*----------- Camera Rotations -----------*/
float radius = 20.0f, cameraHorizontalAngle = 45.0f, cameraVerticleAngle = 45.0f;
float viewX = radius * cos(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
float viewZ = radius * sin(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
float viewY = radius * cos(radians(cameraVerticleAngle));
/* Camera lookAt Function */
bool moveHorizontal = true;
vec3 center = vec3(0.0, 0.0, 0.0), up = vec3(0.0, 1.0, 0.0), eye = vec3(viewX, viewY, viewZ);
void updateView(bool moveHorizontal, float &viewX, float &viewY, float &viewZ);
/* Camera Funcction Prototypes */
void moveUp(); void moveDown(); void moveLeft(); void moveRight();
// Selection Color
vec3 selectedColor = vec3(0.0f, 0.0f, 0.0f);
/*----------- Face Texture -----------*/
GLuint textureID, texture; bool showTexture = false; int faceTexture = 0; GLushort* textureIndices; Vertex* textureVertices;
// Copied directly out of  Tutorial 5 openGL Function
GLuint loadBMPImage(const char * imagepath);
void loadFaceTexture();
void saveIntoStandardMesh();
void saveIntoTextureVertices();
vec3 yRotate = vec3(0, 1, 0);
vec3 xRotate = vec3(1, 0, 0);
/*----------- Blender Head -----------*/
bool showHead = false; int head = 0; GLushort* headIndices; Vertex* headVertices;
void loadHead();
bool showEye = false;
int eyeSize = 0; GLushort* eyeIndices; Vertex* eyeVertices;
void loadEyes();
/*----------- Move the Vertex -----------*/
void moveVertex(void);
bool shiftPressed = false;
/*----------- Load File -----------*/
ifstream loadedFile; Vertex * loadFile(char * path);
/*----------- Save File -----------*/
ofstream savedFile; void saveFile(char * path);

/**************** Function Prototypes *******************/
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
	window = glfwCreateWindow(window_width, window_height, "", NULL, NULL);
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
	glDeleteProgram(meshProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

void bindWhiteGrid() {
	for (int i = 0; i < gridLength; ++i){
		glBindVertexArray(VertexArrayId[i + 1]);        /* Grid Z */
		glDrawArrays(GL_LINE_LOOP, 0, gridLength);
		glBindVertexArray(VertexArrayId[i + gridLength + 1]);   /* Grid X */
		glDrawArrays(GL_LINE_LOOP, 0, gridLength);
	}
}
void loadHead() {
	vec4 xyzVec = vec4(1.0, 1.0, 1.0, 1.0);
	head = loadObject("models/lewFace.obj", xyzVec, headVertices, headIndices, 66);
	createVAOs(headVertices, headIndices, 66);
}
void loadEyes() {
	vec4 xyzVec = vec4(0.0, 1.0, 0.0, 1.0);
	eyeSize = loadObject("models/eye.obj", xyzVec, eyeVertices, eyeIndices, 67);
	createVAOs(eyeVertices, eyeIndices, 67);
}
void loadFaceTexture() {
	vec4 yVec = vec4(0.0, 1.0, 0.0, 1.0);
	faceTexture = loadObject("models/plane2.obj", yVec, textureVertices, textureIndices, 65);
	createVAOs(textureVertices, textureIndices, 65);
	/* lew bitmap image is put on the subdivided plane I made in blender */
	textureID = loadBMPImage("models/lew.bmp");
}
void saveIntoStandardMesh(){
	for (int i = 0; i < faceTexture; ++i) {
		standardMesh[i] = textureVertices[i];
	}
}

void saveIntoTextureVertices(){
	for (int i = 0; i < faceTexture; ++i) {
		textureVertices[i] = standardMesh[i];
	}
}

/* ----------------------------------Create Grid Vertices------------------------------- */
void createGridVertices(Vertex gridVertsZ[gridLength][gridLength], Vertex gridVertsX[gridLength][gridLength]){
	int x_ = 0, z_ = 0;
	for (float x = -5.0f; x <= 5.0f; x += 0.5f){
		for (float z = -5.0f; z <= 5.0f; z += 0.5f){
			gridVertsZ[x_][z_] = { { x, 0.0, z, w }, { 1.0, 1.0, 1.0, w }, { 0.0, 0.0, 1.0 } };
			++z_;
		}
		++x_;
		z_ = 0;
	}

	for (int x = 0; x < gridLength; ++x) {
		for (int z = 0; z < gridLength; ++z){
			gridVertsX[x][z] = gridVertsZ[z][x];
		}
	}

	for (int i = 1; i <= gridLength; ++i) {
		VertexBufferSize[i] = sizeof(gridVertsZ[i - 1]);
		createVAOs(gridVertsZ[i - 1], NULL, i);

		VertexBufferSize[i + gridLength] = sizeof(gridVertsX[i - 1]);
		createVAOs(gridVertsX[i - 1], NULL, i + gridLength);
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
void moveUp() {
	moveHorizontal = false;
	/* go in reverse direction */
	cameraVerticleAngle -= 5.0f;
	updateView(moveHorizontal, viewX, viewY, viewZ);
	/* update the eye, center is always 0, up is always +y */
	eye = vec3(viewX, viewY, viewZ);
	gViewMatrix = lookAt(eye, center, up);
}

void moveDown() {
	moveHorizontal = false;
	/* go in reverse direction */
	cameraVerticleAngle += 5.0f;
	updateView(moveHorizontal, viewX, viewY, viewZ);
	/* update the eye */
	eye = vec3(viewX, viewY, viewZ);
	gViewMatrix = lookAt(eye, center, up);
}

void moveLeft() {
	moveHorizontal = true;
	/* go in reverse direction */
	cameraHorizontalAngle += 5.0f;
	updateView(moveHorizontal, viewX, viewY, viewZ);
	/* update the eye */
	eye = vec3(viewX, viewY, viewZ);
	gViewMatrix = lookAt(eye, center, up);

}
void moveRight() {
	moveHorizontal = true;
	cameraHorizontalAngle -= 5.0f;
	updateView(moveHorizontal, viewX, viewY, viewZ);
	/* update the eye */
	eye = vec3(viewX, viewY, viewZ);
	gViewMatrix = lookAt(eye, center, up);
}

/* Load File */
Vertex* loadFile(char * path) {
	size_t position = 0;
	loadedFile.open(path);
	string line; // line by line each file
	string delimiter = ",";
	string token;

	Vertex* loadedmesh = new Vertex[faceTexture];
	int index = 0;
	int axis = 0;

	// get one line, store in variable 'line'
	while (getline(loadedFile, line)) {
		while ((position = line.find(delimiter)) != string::npos) {
			token = line.substr(0, position);
			loadedmesh[index].Position[axis] = (float) ::atof(token.c_str());
			line.erase(0, position + delimiter.length());
			++axis;
		}
		token = line;
		loadedmesh[index].Position[axis] = (float) ::atof(token.c_str());
		++index;
		axis = 0;
	}
	loadedFile.close();
	return loadedmesh;
}

/* Save File */
void saveFile(char * path) {
	savedFile.open(path);
	float x, y, z;
	for (int i = 0; i < faceTexture; ++i) {
		x = textureVertices[i].Position[0];
		y = textureVertices[i].Position[1];
		z = textureVertices[i].Position[2];
		savedFile << x << "," << y << "," << z << endl;
	}
	savedFile.close();
}

/* Copied from Tutorial 5 Textured Cube  */
GLuint loadBMPImage(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height, bpp;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file)							    { printf("Image could not be opened\n"); return 0; }

	// Read the header, i.e. the 54 first bytes

	// If less than 54 byes are read, problem
	if (fread(header, 1, 54, file) != 54){
		printf("Not a correct BMP file\n");
		return false;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M'){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0)         { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24)         { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	bpp = 3;

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54;

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	// Swap Red and Blue component for each texel of the image
	unsigned char t;
	for (unsigned int i = 0; i < imageSize; i += 3)
	{
		t = data[i];
		data[i] = data[i + 2];
		data[i + 2] = t;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	// "Bind" the newly created texture as a 2D texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (bpp == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);
	// Return the ID of the texture we just created
	return textureID;
}

int loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId) {
	// Read our .obj file
	vector<vec3> vertices;
	vector<vec2> uvs;			// added uvs
	vector<vec3> normals;

	/* Load Object */
	loadOBJ(file, vertices, uvs, normals);

	vector<GLushort> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;	// added indexed_uvs
	vector<vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();
	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
		out_Vertices[i].SetUVs(&indexed_uvs[i].x);	// added setUVs
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
void createObjects(void) {
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { .99, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { .99, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, .99, 0.0, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, .99, 0.0, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, .99, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, .99, 1.0 }, { 0.0, 0.0, 1.0 }, { -2.0, -2.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	/* Kat Modified */
	createGridVertices(gridVertsZ, gridVertsX); // white grid 
	loadHead();									// blender head
	loadEyes();
	loadFaceTexture();							// face texture
	saveIntoStandardMesh();
}


void renderScene(void) {
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glEnable(GL_PROGRAM_POINT_SIZE);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(selectedColorID, selectedColor[0], selectedColor[1], selectedColor[2]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		/* ------------------- Kat Modified XZ Grid  -------------------*/
		bindWhiteGrid();
		if (showHead) {
			/* vec3 xDown = vec3(-2.6, -2.0, 0);
			vec3 pushBack = vec3(0, 0, 2.0); */
			vec3 xDown = vec3(-1.9, -2.0, 0);
			vec3 pushBack = vec3(0, 0, 3.0);
			/* Model Matrix - adjust for head from blender */
			ModelMatrix = rotate(ModelMatrix, -143.0f, yRotate);
			ModelMatrix = translate(ModelMatrix, xDown);
			ModelMatrix = translate(ModelMatrix, pushBack);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[66]);
			glDrawElements(GL_TRIANGLES, head * 10, GL_UNSIGNED_SHORT, (void*)0);
			ModelMatrix = mat4(1.0);	// resetting
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


			
		}
		if (showEye) {
			vec3 xDown = vec3(-1.7, 0.2, 0);
			vec3 pushBack = vec3(0, 0, -1.0); 

			ModelMatrix = translate(ModelMatrix, xDown);
			ModelMatrix = translate(ModelMatrix, pushBack);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[67]);
			glDrawElements(GL_TRIANGLES, head * 10, GL_UNSIGNED_SHORT, (void*)0);
			ModelMatrix = mat4(1.0);	// resetting
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			vec3 xDown2 = vec3(0.0, 0.2, 0);
			ModelMatrix = translate(ModelMatrix, xDown2);
			ModelMatrix = translate(ModelMatrix, pushBack);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		
			glDrawElements(GL_TRIANGLES, head * 10, GL_UNSIGNED_SHORT, (void*)0);
			ModelMatrix = mat4(1.0);	// resetting
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		}
		if (showTexture) {
			ModelMatrix = rotate(ModelMatrix, 90.0f, xRotate);
			ModelMatrix = rotate(ModelMatrix, 90.0f, yRotate);
			// VERTICES
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[65]);
			glDrawElements(GL_POINTS, faceTexture * 10, GL_UNSIGNED_SHORT, (void*)0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, faceTexture * 10, GL_UNSIGNED_SHORT, (void*)0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			ModelMatrix = mat4(1.0);	// resetting
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		}
		glBindVertexArray(0);
	}

	/* Render control mesh */
	glUseProgram(meshProgramID);
	{
		mat4x4 ModelMatrix = mat4(1.0);
		glUniform3f(selectedColorID, selectedColor[0], selectedColor[1], selectedColor[2]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	
		if (showTexture) {
			ModelMatrix = rotate(ModelMatrix, 90.0f, xRotate);
			ModelMatrix = rotate(ModelMatrix, 90.0f, yRotate);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[65]);
			glDrawElements(GL_TRIANGLES, faceTexture * 10, GL_UNSIGNED_SHORT, (void*)0);
			ModelMatrix = mat4(1.0);	// resetting
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		}
		glBindVertexArray(0);
	}

	glUseProgram(0);
	// Draw GUI
	TwDraw();
	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glEnable(GL_PROGRAM_POINT_SIZE);
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		ModelMatrix = rotate(ModelMatrix, 90.0f, xRotate);
		ModelMatrix = rotate(ModelMatrix, 90.0f, yRotate);
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glBindVertexArray(VertexArrayId[65]);

		float pickedID;
		for (int i = 0; i < faceTexture; ++i) {
			if (i < 255) {
				pickedID = (float)i / 255.0f;	
			}
			else {
				pickedID = (float)(1.0f + i - 255.0) / 255.0f;
			}
			glUniform1f(pickingColorID, pickedID);
			glUniform1f(pickingColorIDG, 0.0);
			glDrawArrays(GL_POINTS, i, 1);
		}
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
	gPickedIndexG = int(data[1]);

	if (gPickedIndex == 255 && gPickedIndexG == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else if (gPickedIndexG == 0){
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}
	else if (gPickedIndex == 0){
		std::ostringstream oss;
		oss << "point " << gPickedIndexG + 255;
		gPickedIndex = gPickedIndexG + 255;
		gMessage = oss.str();
	}
	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//std::cin.ignore();
	//continue; // skips the normal rendering
}

/* Move the vertices of the control mesh */
void moveVertex(void) {
	// Stayed the same from HW 1C
	double xpos, ypos;
	mat4 ModelMatrix = mat4(1.0);
	glfwGetCursorPos(window, &xpos, &ypos);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	vec4 vp = vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	vec3 worldCoords = unProject(vec3(window_width - xpos, window_height - ypos, 0.0), ModelMatrix, gProjectionMatrix, vp);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS){
		if (gPickedIndex < faceTexture) {
			textureVertices[gPickedIndex].Position[0] = worldCoords[0] * 100;
			textureVertices[gPickedIndex].Position[2] = worldCoords[1] * 100;
		}
		shiftPressed = false;
	}
	else {
		if (gPickedIndex < faceTexture) {
			textureVertices[gPickedIndex].Position[1] = worldCoords[1] * 100;
		}
		shiftPressed = true;
	}
}

void initOpenGL(void) {
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
	gViewMatrix = glm::lookAt(glm::vec3(viewX, viewY, viewZ),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");
	meshProgramID = LoadShaders("meshShading.vertexshader", "meshShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	pickingColorIDG = glGetUniformLocation(pickingProgramID, "PickingColorG");
	// Get a handle for our "LightPosition" uniform

	selectedColorID = glGetUniformLocation(programID, "selected_color");
	textureID = glGetUniformLocation(meshProgramID, "texture");
	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
	const size_t UVoffset = sizeof(Vertices[0].Normal) + Normaloffset;

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
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)UVoffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal
	glEnableVertexAttribArray(3);	// UV

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


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Vertex * newMesh;
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_E:
			showEye = (showTexture == true ? false : true);
			break;
			case GLFW_KEY_C:
				showTexture = (showTexture == true ? false : true);
				break;
			case GLFW_KEY_F:
				showHead = (showHead == true ? false : true);
				break;
			case GLFW_KEY_S:
				saveFile("save.hw3");
				cout << "Saved" << endl;
				break;
			case GLFW_KEY_L:
				newMesh = loadFile("save.hw3");
				for (int i = 0; i < faceTexture; ++i) {
					textureVertices[i].Position[0] = newMesh[i].Position[0];
					textureVertices[i].Position[1] = newMesh[i].Position[1];
					textureVertices[i].Position[2] = newMesh[i].Position[2];
				}
				cout << "Loaded" << endl;
				break;
			case GLFW_KEY_R:
				moveHorizontal = false;
				cameraHorizontalAngle = 45.0f;
				cameraVerticleAngle = 45.0f;
				updateView(moveHorizontal, viewX, viewY, viewZ);
				eye = vec3(viewX, viewY, viewZ);
				gViewMatrix = lookAt(eye, center, up);
				saveIntoTextureVertices();
				cout << "Reset" << endl;
				break;
			default:
				break;
			}
	}

}
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
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
		if (GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_UP))
				moveUp();
			if (glfwGetKey(window, GLFW_KEY_DOWN))
				moveDown();
			if (glfwGetKey(window, GLFW_KEY_LEFT))
				moveLeft();
			if (glfwGetKey(window, GLFW_KEY_RIGHT))
				moveRight();
		}
		// if you press the left button move hte vertex 
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
			moveVertex();
		}
		createVAOs(textureVertices, textureIndices, 65);
		renderScene();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);
	cleanup();
	return 0;
}