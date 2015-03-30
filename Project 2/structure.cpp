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
	}
}

int main(void) {
	do{
		if (GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_UP))
				moveUp(lastPressedKey);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				if (lastPressedKey == GLFW_KEY_5) {
					haptic[5].rotate -= vec3(0.0f, 5.0f, 0.0f);
				}
			}
		}
		// DRAWING POINTS
		renderScene();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);
}

/* StandardShading.fragmentshader */
#version 330 core

// Interpolated values from the vertex shaders
in vec4 vs_vertexColor;
in vec3 Position_worldspace, Normal_cameraspace, EyeDirection_cameraspace, LightDirection_cameraspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform mat4 MV;
uniform vec3 LightPosition_worldspace, selectedColor;

void main(){

	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(0,1,0);
	float LightPower = 29.0f;
	
	vec3 MaterialDiffuseColor = vs_vertexColor.rgb;
	vec3 MaterialAmbientColor = vec3(0.2,0.2,0.2) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vs_vertexColor.rgb * vec3(.1,.1,.1);
	
	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );
	float distance2 = length( LightPosition_worldspace2 - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);

	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color = vs_vertexColor.rgb;


	color = 
		// Ambient : simulates indirect lighting
		(MaterialAmbientColor +
		// Diffuse : "color" of the object
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance)*cosTheta2 / (distance2*distance2) +
		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance)*pow(cosAlpha2,5) / (distance2*distance2)) +
		
		(MaterialAmbientColor +
		// Diffuse : "color" of the object
		MaterialDiffuseColor * LightColor2 * LightPower2 * cosTheta2 / (distance2*distance2) +
		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor2 * LightPower2 * pow(cosAlpha2,5) / (distance2*distance2))
		;	
}

/* StandardShading.vertexshader */
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;

// Output data ; will be interpolated for each fragment.
out vec4 vs_vertexColor;
out vec3 Position_worldspace, Normal_cameraspace, EyeDirection_cameraspace, LightDirection_cameraspace;

// Values that stay constant for the whole mesh.
uniform mat4 M, V, P;
uniform vec3 LightPosition_worldspace, LightPosition_worldspace2, selectedColor;

void main(){
	gl_PointSize = 5.0;
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  P * V * M * vertexPosition_modelspace;
	
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vertexPosition_modelspace).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vertexPosition_modelspace).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	/* Kat modified V to M because it was doing it relative to the camera */
	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( M * vec4(LightPosition_worldspace,1)).xyz;

	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( V * M * vec4(1.0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	
	// UV of the vertex. No special space for this one.
	vs_vertexColor = vertexColor;
}
