//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
glm::mat4 model2;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;
float valDensity;
GLuint valDensityLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 color;

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 2.5f), glm::vec3(0.0f, 0.0f, -10.0f));
float cameraSpeed = 0.5f;

glm::vec2 position, lastPosition;
bool pressedKeys[1024];
float angle = 0.0f;
float angleY = 0.0f;
GLfloat lightAngle;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::Model3D tent;
gps::Model3D lake;
gps::Model3D campfire;
gps::Model3D tree;
gps::Model3D lamp;
gps::Model3D eagle;
gps::Model3D mountain;
gps::Model3D guitar;
gps::Model3D bench;
gps::Model3D picnictable;
gps::Model3D ponton;
gps::Model3D rocks;

gps::SkyBox mySkyBox;
gps::SkyBox mySkyBox1;
std::vector<const GLchar*> faces, faces1;

gps::Shader myCustomShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

bool firstMouse = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	float xoffset, yoffset;
	position.x = xpos;
	position.y = ypos;

	if (firstMouse){
		lastPosition.x = position.x;
		lastPosition.y = position.y;
		firstMouse = false;
	}
	
	xoffset = position.x - lastPosition.x;
	yoffset = lastPosition.y - position.y;

	lastPosition.x = position.x;
	lastPosition.y = position.y;

	myCamera.rotate(yoffset, xoffset);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.1f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_O]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_I]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_M]) {
		faces.push_back("skybox/ame_siege/right.tga");
		faces.push_back("skybox/ame_siege/left.tga");
		faces.push_back("skybox/ame_siege/top.tga");
		faces.push_back("skybox/ame_siege/bottom.tga");
		faces.push_back("skybox/ame_siege/back.tga");
		faces.push_back("skybox/ame_siege/front.tga");
		mySkyBox.Load(faces);
		mySkyBox.Draw(skyboxShader, view, projection);
		lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		myCustomShader.useShaderProgram();
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}

	if (pressedKeys[GLFW_KEY_N]) {
		faces1.push_back("skybox/rt.tga");
		faces1.push_back("skybox/lf.tga");
		faces1.push_back("skybox/up.tga");
		faces1.push_back("skybox/dn.tga");
		faces1.push_back("skybox/bk.tga");
		faces1.push_back("skybox/ft.tga");
		mySkyBox1.Load(faces1);
		mySkyBox = mySkyBox1;
		lightColor = glm::vec3(0.0f, 0.0f, 1.0f);
		myCustomShader.useShaderProgram();
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		valDensity = 0.005f;
		myCustomShader.useShaderProgram();
		glUniform1f(valDensityLoc, valDensity);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		valDensity = 0.0001f;
		myCustomShader.useShaderProgram();
		glUniform1f(valDensityLoc, valDensity);
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", glfwGetPrimaryMonitor(), NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels()
{
	tent = gps::Model3D("objects/tent1/tent2.obj", "objects/tent1/");
	lake = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	campfire = gps::Model3D("objects/fire/fire3.obj", "objects/fire/");
	tree = gps::Model3D("objects/Tree02/Tree.obj", "objects/Tree02/");
	lamp = gps::Model3D("objects/lamp/camping_lamp2.obj", "objects/lamp/");
	eagle = gps::Model3D("objects/bird/Eagle.obj", "objects/bird/");
	mountain = gps::Model3D("objects/mountain/M5_2.obj", "objects/mountain/");
	guitar = gps::Model3D("objects/guitar/guitar.obj", "objects/guitar/");
	bench = gps::Model3D("objects/bench/bench.obj", "objects/bench/");
	picnictable = gps::Model3D("objects/table/table.obj", "objects/table/");
	ponton = gps::Model3D("objects/ponton/ponton.obj", "objects/ponton/");
	rocks = gps::Model3D("objects/rocks/rocks3.obj", "objects/rocks/");
}

void initFramebuffer()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//GLuint depthMapTexture;
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(myCamera.getCameraTarget() + 1.0f * lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initSkyBox()
{
	faces.push_back("skybox/ame_siege/right.tga");
	faces.push_back("skybox/ame_siege/left.tga");
	faces.push_back("skybox/ame_siege/top.tga");
	faces.push_back("skybox/ame_siege/bottom.tga");
	faces.push_back("skybox/ame_siege/back.tga");
	faces.push_back("skybox/ame_siege/front.tga");
	mySkyBox.Load(faces);
}


void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	valDensity = 0.0001f;
	valDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "valDensity");
	glUniform1f(valDensityLoc, valDensity);
}

float delta = 0;
float movementSpeed = 2; // units per second

void updateDelta(double elapsedSeconds) {
	delta = delta + movementSpeed * elapsedSeconds;
}

double lastTimeStamp = glfwGetTime();
void computeShadow() {
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//--------------------------------------------------------------------
	glm::mat4 model2 = model;

	model = model2;
	model = glm::translate(model, glm::vec3(35, -5, 15));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -5, 100));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-15, -3, 30));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(230.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-12, -3.5, 19));
	model = glm::scale(model, glm::vec3(85, 85, 85));
	model = glm::rotate(model, glm::radians(140.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	guitar.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(28, -4, -6));//23
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -4, -20));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -6, 7));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(5, -5, 5));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	campfire.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-3, -5, 0));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	glm::mat4 model4 = model2;
	model2 = glm::translate(model, glm::vec3(23, 5, 80));
	model = model2;
	model = glm::translate(model, glm::vec3(35, -3, 15));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(0, 0, 1));
	model = glm::rotate(model, glm::radians(-3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(10, -3, -5));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-7.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(15, 0, 20));
	model = glm::rotate(model, glm::radians(-2.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	campfire.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(7, -1, 15));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(1, 0, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model2 = model4;
	model = model2;
	model = glm::translate(model, glm::vec3(-4, -5, 10));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(30, -8.5, -320));
	model = glm::scale(model, glm::vec3(11, 11, 11));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lake.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-42, -10, -230));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	rocks.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-40, -10, -233));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	rocks.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-44, -10, -233));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	rocks.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-10, -5, -300));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ponton.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(0, -2, -230));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(-40.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-50, 0, -120));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-35, -5, -95));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-6.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-35, -5, -150));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(-250.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tent.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(5, -6, -95));
	model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	picnictable.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(15, -5, -80));
	model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
	model = glm::rotate(model, glm::radians(-3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	picnictable.Draw(depthMapShader);

	glm::mat4 model3 = model2;

	for (int i = 1; i <= 5; i++) {
		if (i == 2) {
			model2 = glm::translate(model2, glm::vec3(-90, -7, 0));
		}
		else if (i == 3) {
			model2 = glm::translate(model2, glm::vec3(80, 0, 200));
		}
		else if (i == 4) {
			model2 = glm::rotate(model2, glm::radians(-150.0f), glm::vec3(0, 1, 0));
			model2 = glm::translate(model2, glm::vec3(-120, 0, 50));
		}
		else if (i == 5) {
			model2 = glm::translate(model2, glm::vec3(-120, 0, 250));
		}
		model = model2;
		model = glm::translate(model, glm::vec3(50, 0, -30));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, -2, 10));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(40, 0, -70));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 3, -80));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(70, 0, -55));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(65, 0, 40));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(35, -7, 50));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(35, 0, -50));
		model = glm::scale(model, glm::vec3(4, 4, 4));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(65, 0, -57));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 0, -5));
		model = glm::scale(model, glm::vec3(7, 7, 7));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(30, 0, -20));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 0, 0));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(50, -3, 70));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);

		model = model2;
		model = glm::translate(model, glm::vec3(50, 0, 40));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		tree.Draw(depthMapShader);
	}

	model2 = model3;
	model = model2;

	double currentTimeStamp = glfwGetTime();
	updateDelta(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;

	model = glm::translate(model, glm::vec3(0, 30, 0));
	model = glm::rotate(model, glm::radians(-delta * 4), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(20, 30, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	eagle.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(0, 30, 0));
	model = glm::rotate(model, glm::radians(delta * 4), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(40, 30, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	eagle.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(410, 80, -200));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(depthMapShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-400, 80, 200));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(depthMapShader);
	//------------------------------------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();
	computeShadow();

	myCustomShader.useShaderProgram();

	//--------------------------------------------------------------

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glm::mat4 model2 = model;

	model = model2;
	model = glm::translate(model, glm::vec3(35, -5, 15));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -5, 100));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-15, -3, 30));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(230.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-12, -3.5, 19));
	model = glm::scale(model, glm::vec3(85, 85, 85));
	model = glm::rotate(model, glm::radians(140.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	guitar.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(28, -4, -6));//23
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	lamp.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -4, -20));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(20, -6, 7));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	lamp.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(5, -5, 5));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	campfire.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-3, -5, 0));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	bench.Draw(myCustomShader);

	glm::mat4 model4 = model2;
	model2 = glm::translate(model, glm::vec3(23, 5, 80));
	model = model2;
	model = glm::translate(model, glm::vec3(35, -3, 15));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(0, 0, 1));
	model = glm::rotate(model, glm::radians(-3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(10, -3, -5));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-7.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(15, 0, 20));
	model = glm::rotate(model, glm::radians(-2.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	campfire.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(7, -1, 15));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(1, 0, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	bench.Draw(myCustomShader);
	
	model2 = model4;
	model = model2;
	model = glm::translate(model, glm::vec3(-4, -5, 10));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	bench.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(30, -8.5, -320));
	model = glm::scale(model, glm::vec3(11, 11, 11));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	lake.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-42, -10, -230));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rocks.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-40, -10, -233));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rocks.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-44, -10, -233));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rocks.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-10, -5, -300));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ponton.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(0, -2, -230));
	model = glm::scale(model, glm::vec3(240, 240, 240));
	model = glm::rotate(model, glm::radians(-40.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-50, 0, -120));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-35, -5, -95));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-6.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-35, -5, -150));
	model = glm::scale(model, glm::vec3(320, 320, 320));
	model = glm::rotate(model, glm::radians(-250.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tent.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(5, -6, -95));
	model = glm::scale(model, glm::vec3(1.5,1.5, 1.5));
	model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	picnictable.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(15, -5, -80));
	model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
	model = glm::rotate(model, glm::radians(-3.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	picnictable.Draw(myCustomShader);

	glm::mat4 model3 = model2;

	for (int i = 1; i <= 5; i++) {
		if (i == 2) {
			model2 = glm::translate(model2, glm::vec3(-90, -7, 0));
		} 
		else if (i == 3) {
			model2 = glm::translate(model2, glm::vec3(80, 0, 200));
		}
		else if (i == 4) {
			model2 = glm::rotate(model2, glm::radians(-150.0f), glm::vec3(0, 1, 0));
			model2 = glm::translate(model2, glm::vec3(-120, 0, 50));
		} 
		else if (i == 5) {
			model2 = glm::translate(model2, glm::vec3(-120, 0, 250));
		}
		model = model2;
		model = glm::translate(model, glm::vec3(50, 0, -30));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, -2, 10));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(40, 0, -70));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 3, -80));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(70, 0, -55));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(65, 0, 40));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(35, -7, 50));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(35, 0, -50));
		model = glm::scale(model, glm::vec3(4, 4, 4));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(65, 0, -57));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 0, -5));
		model = glm::scale(model, glm::vec3(7, 7, 7));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(30, 0, -20));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(60, 0, 0));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(50, -3, 70));
		model = glm::scale(model, glm::vec3(6, 6, 6));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);

		model = model2;
		model = glm::translate(model, glm::vec3(50, 0, 40));
		model = glm::scale(model, glm::vec3(5, 5, 5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		tree.Draw(myCustomShader);
	}
	model2 = model3;
	model = model2;
	
	double currentTimeStamp = glfwGetTime();
	updateDelta(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;
	
	model = glm::translate(model, glm::vec3(0, 30, 0));
	model = glm::rotate(model, glm::radians(-delta*4), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(20, 30, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	eagle.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(0, 30, 0));
	model = glm::rotate(model, glm::radians(delta * 4), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(40, 30, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	eagle.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(410, 80, -200));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	mountain.Draw(myCustomShader);

	model = model2;
	model = glm::translate(model, glm::vec3(-400, 80, 200));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	mountain.Draw(myCustomShader);

	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	mySkyBox.Draw(skyboxShader, view, projection);
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFramebuffer();
	initModels();
	initShaders();
	initSkyBox();
	myCustomShader.useShaderProgram();
	initUniforms();
	//glCheckError();
	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
