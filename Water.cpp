#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <stb_image.h>


#include <fstream>
#include <sstream>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FrameBuffer.h"



using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

// camera
Camera camera(glm::vec3(2.0f, 5.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
// Second camera used for reflection
Camera cameraReflection(glm::vec3(2.0f, 5.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// waves
float waveSpeed = 0.3f;
float waterOffset = 0;

// projection matrix
mat4 projectionMatrix;

// cursor state
bool cursorHidden = true;

// Ray State
bool rayExists = false;
vec3 ray;

// Epsilon
float INTERSECT_EPSILON = 0.5f;

// Intersection Triangle Verticies
vec3 vertice1;
vec3 vertice2;
vec3 vertice3;
vec3 vertice4;

vec3 intersection = vec3(5.0f, 0, 5.0f);


// Structure Surface
struct Surface
{
	GLfloat* coordinates; //array holding the vertex information.
	int size; // the generated coordinates array size.
	int* indexBuffer; // array holding the indices of the triangle strips
	int indexCount;
	void print() { cout << "coordinates " << coordinates << " coordinate_size " << size; };

};

// Returns number of vertices needed
int getVerticesCount(int hVertices, int vVertices) {
	return hVertices * vVertices * 5;
}

// Returns number of indices needed
int getIndicesCount(int hVertices, int vVertices) {
	int numStripsRequired = vVertices - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * hVertices;

	return (verticesPerStrip * numStripsRequired) + numDegensRequired;
}

// Generate coordinates for vertices
GLfloat* generateVerticies(int hVertices, int vVertices, int size, int verticeCount) {

	GLfloat* surfaceVertices = new GLfloat[verticeCount];

	GLfloat cellSize = (float) (size) / (float)(hVertices);
	int verticeIndex = 0;
	
	// Looping over all points and generating coordinates
	for (int col = vVertices-1; col >= 0; col--) {
		float ratioC = (float)col / (float)vVertices;
		for (int row = 0; row < hVertices; row++) {
			float ratioR = (float)row / (float)(hVertices-1);
				surfaceVertices[verticeIndex] = (float) (ratioR * size);
				surfaceVertices[verticeIndex + 2] = (float) (ratioC * size);
				surfaceVertices[verticeIndex + 1] = 0.0f;
				surfaceVertices[verticeIndex + 3] = 0.0f + (float)(cellSize * row) / size;
				surfaceVertices[verticeIndex + 4] = 0.0f + (float)(cellSize * col) / size;

				// Add $ vertices to global for check of rayCasting
				if (row == 0 && col == vVertices - 1) {
					vertice1 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
				}
				else if (row == hVertices - 1 && col == vVertices - 1) {
					vertice2 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
				}
				else if (col == 0 && row == 0) {
					vertice3 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
				}
				else if(col == 0 && row == hVertices - 1) {
					vertice3 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
				}
				
				verticeIndex += 5;

		}
	}

	return surfaceVertices;

}

// Generates the sequence of indices
int* getIndices(int width, int height, int indicesCount) {

	int* indices = new int[indicesCount];

	int offset = 0;

	for (int y = 0; y < height - 1; y++) {
		if (y > 0) {
			// Degenerate begin: repeat first vertex
			indices[offset++] = (int)(y * height);
		}

		for (int x = 0; x < width; x++) {
			// One part of the strip
			indices[offset++] = (int)((y * height) + x);
			indices[offset++] = (int)(((y + 1) * height) + x);
		}

		if (y < height - 2) {
			// Degenerate end: repeat last vertex
			indices[offset++] = (int)(((y + 1) * height) + (width - 1));
		}
	}


	/*for (int y = 0; y < indicesCount; y++) {

		cout << indices[y] << endl;
	}*/

	return indices;

}

Surface GenerateIndexedTriangleStripPlane(int hVertices, int vVertices, float size) {

	// Get number of Indices and Vertices
	int verticeCount = getVerticesCount(hVertices, vVertices);
	int indicesCount = getIndicesCount(hVertices, vVertices);

	// Generate vecs
	GLfloat* surfaceVerticies = generateVerticies(hVertices, vVertices, size, verticeCount);
	int* surfaceIndices = getIndices(hVertices, vVertices, indicesCount);


	Surface surfaceData = {};

	surfaceData.coordinates = surfaceVerticies;
	surfaceData.size = verticeCount;
	surfaceData.indexBuffer = surfaceIndices;
	surfaceData.indexCount = indicesCount;

	return surfaceData;
};

bool IntersectTriangle(vec3 dir, vec3 v0, vec3 v1, vec3 v2, vec3 orig)
{
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 N = cross(v0v2, v0v1); // N 
	float denom = dot(N, N);

	float NdotRayDirection = dot(N, dir);
	if (fabs(NdotRayDirection) < INTERSECT_EPSILON) // almost 0 
		return false; // they are parallel so they don't intersect ! 

	float d = dot(N, v0);

	float t = -((dot(N, orig) - d) / NdotRayDirection);
	if (t < 0) return false; // the triangle is behind 

	intersection = orig + t * dir;

    //inside-outside test
	vec3 C;

	vec3 edge0 = v1 - v0;
	vec3 vp0 = intersection - v0;
	C = cross( vp0, edge0);
	//if (dot(N, C) < 0) return false; // P is on the right side 

	vec3 edge1 = v2 - v1;
	vec3 vp1 = intersection - v1;
	C = cross(vp1, edge1);
	//if ((dot(N, C)) < 0)  return false; // P is on the right side 

	vec3 edge2 = v0 - v2;
	vec3 vp2 = intersection - v2;
	C = cross(vp2, edge2);
	//if ((dot(N, C)) < 0) return false; // P is on the right side; 

	return true; // this ray hits the triangle 
}

int main(void)
{

	// Generate Ground Surface
	Surface ground = GenerateIndexedTriangleStripPlane(500, 500, 10);
	ground.print();

	// Generate Water Surface
	Surface water = GenerateIndexedTriangleStripPlane(500, 500, 10);
	water.print();


	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1000, 1000, "Scene - Hovhannes Zohrabyan", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// call glewInit after creating the context...
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Add water surface to the scene
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, water.size * sizeof(GLfloat), water.coordinates, GL_STATIC_DRAW);

	// position attribute
	GLintptr vertex_position_offset = 0 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)vertex_position_offset);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, water.indexCount * sizeof(int), water.indexBuffer, GL_STATIC_DRAW);

	// Add ground surface to the scene
	unsigned int VBO_G, VAO_G;
	glGenVertexArrays(1, &VAO_G);
	glGenBuffers(1, &VBO_G);

	glBindVertexArray(VAO_G);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_G);
	glBufferData(GL_ARRAY_BUFFER, ground.size * sizeof(GLfloat), ground.coordinates, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)vertex_position_offset);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint index_buffer_g;
	glGenBuffers(1, &index_buffer_g);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.indexCount * sizeof(int), ground.indexBuffer, GL_STATIC_DRAW);

	// Add textures to bind them afterwards
	Texture texture = Texture("C:/Users/johnz/Desktop/Computer Graphics/Computer Graphics Final Project/WaterDiffuse.png");
	Texture texture_dudv = Texture("C:/Users/johnz/Desktop/Computer Graphics/Computer Graphics Final Project/dudv_map.png");
	Texture texture_ground = Texture("C:/Users/johnz/Desktop/Computer Graphics/Computer Graphics Final Project/TerrainDiffuse.png");
	Texture texture_ground_map = Texture("C:/Users/johnz/Desktop/Computer Graphics/Computer Graphics Final Project/TerrainHeightMap.png");

	Shader ground_program("ground_vertex.shader", "ground_fragment.shader");
	ground_program.use();
	ground_program.setInt("textureMainGround", 0);
	ground_program.setInt("heightMap", 1);

	ground_program.setVec3("viewPos", camera.Position);


	Shader water_program("water_vertex.shader", "water_fragment.shader", "water_geometry.shader");
    water_program.use();
	water_program.setInt("textureMain", 2);
	water_program.setInt("DudvMap", 3);
	water_program.setInt("Reflection", 4);
	water_program.setVec3("viewPos", camera.Position);

	FrameBuffer reflection = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		cameraReflection.Position = vec3(cameraReflection.Position[0], -cameraReflection.Position[1], cameraReflection.Position[2]);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 projectionReflection = glm::perspective(glm::radians(cameraReflection.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		projectionMatrix = projection;
		water_program.setMat4("projection", projection);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 viewReflection = cameraReflection.GetViewMatrix();

		water_program.setMat4("view", view);
		water_program.setFloat("time", currentFrame);

		// Render into custom FrameBuffer
		reflection.Bind(SCR_WIDTH, SCR_HEIGHT);

		glBindVertexArray(VAO_G);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
		ground_program.use();
		
		ground_program.setMat4("projection", projectionReflection);
		ground_program.setMat4("view", viewReflection);

		texture_ground.Bind(GL_TEXTURE0);
		texture_ground_map.Bind(GL_TEXTURE1);

		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		ground_program.setMat4("model", model);


		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);

		reflection.Unbind();
		glDisable(GL_CULL_FACE);

		// Start Rendering to normal Shader
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO_G);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
		ground_program.use();

		ground_program.setMat4("projection", projection);
		ground_program.setMat4("view", view);

		texture_ground.Bind(GL_TEXTURE0);
		texture_ground_map.Bind(GL_TEXTURE1);

		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		ground_program.setMat4("model", model);

		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);

		GLuint unifr_loc = glGetUniformLocation(water_program.ID, "rippleCenter");
		glUniform3f(unifr_loc, intersection[0], intersection[1], intersection[2]);


		if (rayExists) {
			cout << "Checking Ray" << endl;
			bool performWave = false;
			if (IntersectTriangle(ray, vertice1, vertice2, vertice3, camera.Position)) {
				performWave = true;
				cout << "HIT" << endl;
				glfwSetTime(0);
			}
			else if (IntersectTriangle(ray, vertice3, vertice2, vertice4, camera.Position)) {
				performWave = true;
				cout << "HIT" << endl;
				glfwSetTime(0);
			}
			water_program.setBool("performWave", performWave);
			water_program.setVec3("rippleCenter", intersection);
			rayExists = false;
		}

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		water_program.use();

		waterOffset = currentFrame * waveSpeed;
		GLint myUniformLocation = glGetUniformLocation(water_program.ID, "offset");
		glUniform1f(myUniformLocation, waterOffset);

		if (waterOffset > 1) {
			waterOffset = 0.0f;
		}

		processInput(window);

		
		texture.Bind(GL_TEXTURE2);
		texture_dudv.Bind(GL_TEXTURE3);
		reflection.BindTexture(GL_TEXTURE4);
		

		model = glm::mat4(1.0f);
		water_program.setMat4("model", model);

		glDrawElements(GL_TRIANGLE_STRIP, water.indexCount, GL_UNSIGNED_INT, nullptr);

		//Swap front and back buffers 
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

vec2 getNormalizedCoordinates(vec2 pixel, int screenWidth, int screenHeight) {

	float x = (2.0f * pixel[0]) / screenWidth - 1;
	float y = (2.0f * pixel[1]) / screenHeight - 1;

	return vec2(x, -y);

};

vec4 convertToEyeSpace(vec4 clipSpace, mat4 projection) {

	mat4 invertedProjection = glm::inverse(projection);
	vec4 eyeCoordinates = invertedProjection * clipSpace;

	return vec4(eyeCoordinates[0], eyeCoordinates[1], -1.0f, 0.0f);
}

vec3 convertToWorldCoordinates(vec4 eyeCoordinates, mat4 viewMatrix) {
	mat4 invertedViewMatrix = glm::inverse(viewMatrix);
	vec4 ray = invertedViewMatrix * eyeCoordinates;

	vec3 result = vec3(ray[0], ray[1], ray[2]);
	result = glm::normalize(result);

	return result;
}


vec3 ConstructRayFromPixel(float fov, vec2 pixel) {
	
	vec2 normalized = getNormalizedCoordinates(pixel, SCR_WIDTH, SCR_HEIGHT);

	float mouseX = normalized[0];
	float mouseY = normalized[1];

	vec4 clippedCoordinates = vec4(mouseX, mouseY, -1.0f, 1.0f);
	vec4 eyeCoordinates = convertToEyeSpace(clippedCoordinates, projectionMatrix);

	vec3 ray = convertToWorldCoordinates(eyeCoordinates, camera.GetViewMatrix());

	//cout << ray[0]<< " x " << ray[1] << " y " << ray[2] << endl;

	rayExists = true;
	return ray;

};


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
	//	if (cursorHidden) {
	//		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//	}
	//	else {
	//		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//	}

	//	cursorHidden = !cursorHidden;
	//}



	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
		cameraReflection.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		cameraReflection.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
		cameraReflection.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
		cameraReflection.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
	cameraReflection.ProcessMouseMovement(xoffset, -yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
	//cameraReflection.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		ray = ConstructRayFromPixel(12.0f, vec2(xpos, ypos));
		
	}
}