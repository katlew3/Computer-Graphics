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
