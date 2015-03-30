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

