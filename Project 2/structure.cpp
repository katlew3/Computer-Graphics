typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
}

GLuint VertexArrayId[NumObjects], VertexBufferId[NumObjects], IndexBufferId[NumObjects];
size_t NumIndices[NumObjects], VertexBufferSize[NumObjects], IndexBufferSize[NumObjects];

/* Initialize the view - taken from openGL Tutorial 6 */
float viewX = radius * cos(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));
float viewY = radius * cos(radians(cameraVerticleAngle));
float viewZ = radius * sin(radians(cameraHorizontalAngle)) * sin(radians(cameraVerticleAngle));

/* Camera lookAt Function */
vec3 center = vec3(0.0, 0.0, 0.0);
vec3 up = vec3(0.0, 1.0, 0.0);
vec3 eye = vec3(viewX, viewY, viewZ);

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

void createObjects(void) {
	baseSize = loadObject("models/base.obj", red, Vertices, Indices, 23);
	createVAOs(Vertices, Indices, 23);
	haptic[0].translate = vec3(0.0f, 0.4f, 0.0f);
}

void renderScene(void) {
	glUseProgram(programID);
	{
		vec3 lightPos = vec3(4,4,4);
		mat4x4 ModelMatrix = mat4(1.0);
		/* pass extra data to a shader for a specific primitive */
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		/*Uniform*/
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		/* Draw Objects */
		/* Base - 23 */
		ModelMatrix = translate(ModelMatrix, haptic[0].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[23]);
		glDrawElements(GL_TRIANGLES, baseSize * 4, GL_UNSIGNED_SHORT, (void*)0);

		/* Pen - 28 */
		ModelMatrix = translate(ModelMatrix, vec3(0.0f, 0.6f, 0.0f));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[2], vec3(0, 0, 1));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[1], vec3(0, 1, 0));
		ModelMatrix = rotate(ModelMatrix, haptic[5].rotate[0], vec3(1, 0, 0));
		ModelMatrix = translate(ModelMatrix, haptic[5].translate);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[28]);
		glDrawElements(GL_TRIANGLES, penSize * 4, GL_UNSIGNED_SHORT, (void*)0);
	}
}

void initOpenGL(void) {
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	// Camera matrix
	gViewMatrix = lookAt(vec3(10.0, 10.0, 10.0f),	// eye
		vec3(0.0, 0.0, 0.0),	// center
		vec3(0.0, 1.0, 0.0));	// up
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	selectedColorID = glGetUniformLocation(programID, "selectedColor");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);
	
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

	}

}
