/*
Student Information
Student ID: 202201130304
Student Name: 黄玺嘉
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/glfw/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 2000;
const int SCR_HEIGHT = 1500;

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

// 全局变量
Model penguinModel, snowModel;
unsigned int VAOpenguin, VBOpenguin, EBOpenguin;
unsigned int VAOsnow, VBOsnow, EBOsnow;

Shader myShader;
Texture penguinTexture1, penguinTexture2, snowTexture1, snowTexture2;

int currentPenguinTexture = 2;
int currentSnowTexture = 2;

float lightIntensity = 1.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.3f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 penguinPos = glm::vec3(0.0f, 0.0f, 0.0f);
float penguinRotation = 0.0f;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
float yaw = -90.0f, pitch = 0.0f;

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	//Load textures
	penguinModel = loadOBJ("./resources/penguin/penguin.obj");
	snowModel = loadOBJ("./resources/snow/snow.obj");

	glGenVertexArrays(1, &VAOpenguin);
	glGenBuffers(1, &VBOpenguin);
	glGenBuffers(1, &EBOpenguin);

	glBindVertexArray(VAOpenguin);
	glBindBuffer(GL_ARRAY_BUFFER, VBOpenguin);
	glBufferData(GL_ARRAY_BUFFER, penguinModel.vertices.size() * sizeof(Vertex), &penguinModel.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOpenguin);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, penguinModel.indices.size() * sizeof(unsigned int), &penguinModel.indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glGenVertexArrays(1, &VAOsnow);
	glGenBuffers(1, &VBOsnow);
	glGenBuffers(1, &EBOsnow);

	glBindVertexArray(VAOsnow);
	glBindBuffer(GL_ARRAY_BUFFER, VBOsnow);
	glBufferData(GL_ARRAY_BUFFER, snowModel.vertices.size() * sizeof(Vertex), &snowModel.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOsnow);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, snowModel.indices.size() * sizeof(unsigned int), &snowModel.indices[0], GL_STATIC_DRAW);	

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	penguinTexture1.setupTexture("./resources/penguin/penguin_01.png");
	penguinTexture2.setupTexture("./resources/penguin/penguin_02.png");
	snowTexture1.setupTexture("./resources/snow/snow_01.jpg");
	snowTexture2.setupTexture("./resources/snow/snow_02.jpg");
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader
	myShader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures
	myShader.use();

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	myShader.setMat4("projection", projection);
	myShader.setMat4("view", view);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f));
	myShader.setVec3("lightDir", lightDir);
	myShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	myShader.setFloat("lightIntensity", lightIntensity);
	myShader.setVec3("cameraPos", cameraPos);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, penguinPos);
	model = glm::rotate(model, glm::radians(penguinRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
	myShader.setMat4("model", model);

	if (currentPenguinTexture == 1) {
		penguinTexture1.bind(0);
	}
	else {
		penguinTexture2.bind(0);
	}
	myShader.setInt("texture1", 0);

	glBindVertexArray(VAOpenguin);
	glDrawElements(GL_TRIANGLES, penguinModel.indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 0.2f, 1.0f));
	myShader.setMat4("model", model);

	if (currentSnowTexture == 1) {
		snowTexture1.bind(0);
	}
	else {
		snowTexture2.bind(0);
	}
	myShader.setInt("texture1", 0);

	glBindVertexArray(VAOsnow);
	glDrawElements(GL_TRIANGLES, snowModel.indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
bool leftMousePressed = false;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Sets the mouse-button callback for the current window.
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		firstMouse = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS)
			leftMousePressed = true;
		else if (action == GLFW_RELEASE)
			leftMousePressed = false;
	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	// Sets the cursor position callback for the current window
	if (!leftMousePressed)
		return;
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}
	float xoffset = x - lastX;
	float yoffset = lastY - y;
	lastX = x;
	lastY = y;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}
const float moveSpeed = 0.4f;
const float turnAngle = 3.0f;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Sets the Keyboard callback for the current window.
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W)
			lightIntensity += 0.1f;
		if (key == GLFW_KEY_S)
			lightIntensity = std::max(0.0f, lightIntensity - 0.1f);

		if (key == GLFW_KEY_1)
			currentPenguinTexture = 1;
		if (key == GLFW_KEY_2)
			currentPenguinTexture = 2;
		
		if (key == GLFW_KEY_3)
			currentSnowTexture = 1;
		if (key == GLFW_KEY_4)
			currentSnowTexture = 2;

		if (key == GLFW_KEY_LEFT)
			penguinRotation += turnAngle;
		if (key == GLFW_KEY_RIGHT)
			penguinRotation -= turnAngle;
		if (key == GLFW_KEY_UP) {
			float rad = glm::radians(penguinRotation);
			glm::vec3 forward;
			forward.x = sin(rad);
			forward.y = 0.0f;
			forward.z = cos(rad);
			penguinPos += moveSpeed * forward;
		}

		if (key == GLFW_KEY_DOWN) {
			float rad = glm::radians(penguinRotation);
			glm::vec3 forward;
			forward.x = sin(rad);
			forward.y = 0.0f;
			forward.z = cos(rad);
			penguinPos -= moveSpeed * forward;
		}
	}
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  //    
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






