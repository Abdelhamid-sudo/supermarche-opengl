#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"

const char* APP_TITLE = "IOGL - Scene virtuelle : la place du port";
int gWindowWidth = 1280;
int gWindowHeight = 720;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = true;
glm::vec4 gClearColor(0.02f, 0.03f, 0.07f, 1.0f);

FPSCamera fpsCamera(glm::vec3(0.0f, 3.5f, 14.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0;
const float MOUSE_SENSITIVITY = 0.1f;

struct TourWaypoint
{
	glm::vec3 position;
	glm::vec3 target;
	float duration;
};

const TourWaypoint TOUR[] = {
	{ glm::vec3(  0.0f, 9.0f,  20.0f), glm::vec3( 0.0f, 1.5f,  0.0f), 4.0f },
	{ glm::vec3(-12.0f, 6.0f,  10.0f), glm::vec3(-8.0f, 1.0f, -2.0f), 6.0f },
	{ glm::vec3( -5.0f, 2.2f,   5.0f), glm::vec3(-9.0f, 1.0f, -2.5f), 5.0f },
	{ glm::vec3( -4.5f, 3.0f,  -1.5f), glm::vec3( 0.0f, 2.5f,  0.0f), 5.0f },
	{ glm::vec3(  0.0f, 3.2f,  -4.5f), glm::vec3( 0.0f, 2.5f,  0.0f), 4.0f },
	{ glm::vec3(  4.5f, 3.0f,  -1.5f), glm::vec3( 0.0f, 2.5f,  0.0f), 4.0f },
	{ glm::vec3(  4.0f, 1.2f,   4.0f), glm::vec3( 7.3f, 0.6f,  0.5f), 5.0f },
	{ glm::vec3(  2.5f, 1.8f,   7.0f), glm::vec3( 5.0f, 1.3f,  4.0f), 4.0f },
	{ glm::vec3(  0.0f, 12.0f, 22.0f), glm::vec3( 0.0f, 0.5f, -1.0f), 7.0f },
	{ glm::vec3(  0.0f, 12.0f, 22.0f), glm::vec3( 0.0f, 0.5f, -1.0f), 2.0f }
};
const int TOUR_SIZE = sizeof(TOUR) / sizeof(TOUR[0]);

bool gTourActive = false;
int gTourSegment = 0;
float gTourSegmentTime = 0.0f;
glm::vec3 gTourFromPos, gTourFromTarget;

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void startTour();
void stopTour();
void updateTour(float elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

enum ModelId { BARREL, WOODCRATE, CRATE, ROBOT, FLOOR, PIN, BUNNY, LAMP_POST, NUM_MODELS };

struct Instance
{
	ModelId model;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

int main()
{
	if (!initOpenGL())
	{

		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader;
	lightingShader.loadShaders("shaders/lighting_dir_point_spot.vert", "shaders/lighting_dir_point_spot.frag");

	ShaderProgram bulbShader;
	bulbShader.loadShaders("shaders/bulb.vert", "shaders/bulb.frag");

	Mesh mesh[NUM_MODELS];
	Texture2D texture[NUM_MODELS];

	mesh[BARREL].loadOBJ("models/barrel.obj");
	mesh[WOODCRATE].loadOBJ("models/woodcrate.obj");
	mesh[CRATE].loadOBJ("models/crate.obj");
	mesh[ROBOT].loadOBJ("models/robot.obj");
	mesh[FLOOR].loadOBJ("models/floor.obj");
	mesh[PIN].loadOBJ("models/bowling_pin.obj");
	mesh[BUNNY].loadOBJ("models/bunny.obj");
	mesh[LAMP_POST].loadOBJ("models/lampPost.obj");

	texture[BARREL].loadTexture("textures/barrel_diffuse.png", true);
	texture[WOODCRATE].loadTexture("textures/woodcrate_diffuse.jpg", true);
	texture[CRATE].loadTexture("textures/crate.jpg", true);
	texture[ROBOT].loadTexture("textures/robot_diffuse.jpg", true);
	texture[FLOOR].loadTexture("textures/tile_floor.jpg", true);
	texture[PIN].loadTexture("textures/AMF.tga", true);
	texture[BUNNY].loadTexture("textures/bunny_diffuse.jpg", true);
	texture[LAMP_POST].loadTexture("textures/lamp_post_diffuse.png", true);

	Mesh bulbMesh;
	bulbMesh.loadOBJ("models/light.obj");

	const int numPointLights = 3;
	glm::vec3 lampPos[numPointLights] = {
		glm::vec3(-5.5f, 0.0f, -2.0f),
		glm::vec3( 0.0f, 0.0f, -7.0f),
		glm::vec3( 5.5f, 0.0f, -2.0f)
	};
	glm::vec3 lampColor[numPointLights] = {
		glm::vec3(1.0f, 0.65f, 0.3f),
		glm::vec3(1.0f, 0.15f, 0.1f),
		glm::vec3(0.2f, 0.45f, 1.0f)
	};
	const float BULB_HEIGHT = 3.8f;

	const glm::vec3 ONE(1.0f);
	const Instance robotInstance = { ROBOT, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.55f) };
	const Instance scene[] = {
		{ FLOOR,     glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(3.0f, 1.0f, 3.0f) },

		{ LAMP_POST, lampPos[0], glm::vec3(0.0f), ONE },
		{ LAMP_POST, lampPos[1], glm::vec3(0.0f), ONE },
		{ LAMP_POST, lampPos[2], glm::vec3(0.0f), ONE },

		{ WOODCRATE, glm::vec3(-9.5f, 0.0f, -4.0f), glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.8f) },
		{ WOODCRATE, glm::vec3(-9.3f, 0.0f, -2.2f), glm::vec3(0.0f, -6.0f, 0.0f), glm::vec3(0.8f) },
		{ WOODCRATE, glm::vec3(-9.4f, 1.6f, -3.1f), glm::vec3(0.0f, 35.0f, 0.0f), glm::vec3(0.7f) },
		{ WOODCRATE, glm::vec3(-11.2f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.8f) },
		{ WOODCRATE, glm::vec3(-11.2f, 1.6f, -3.0f), glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(0.8f) },
		{ CRATE,     glm::vec3(-7.0f, 0.0f, -6.0f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(0.6f) },
		{ CRATE,     glm::vec3(-7.6f, 0.0f, -0.5f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.45f) },

		{ BARREL,    glm::vec3(-7.5f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.6f) },
		{ BARREL,    glm::vec3(-8.9f, 0.0f, 1.5f), glm::vec3(0.0f, 70.0f, 0.0f), glm::vec3(0.6f) },
		{ BARREL,    glm::vec3(-8.1f, 0.0f, 2.8f), glm::vec3(0.0f, 140.0f, 0.0f), glm::vec3(0.6f) },
		{ BARREL,    glm::vec3(-5.5f, 0.79f, 2.5f), glm::vec3(0.0f, 30.0f, 90.0f), glm::vec3(0.6f) },

		{ PIN,       glm::vec3(7.0f,  0.0f, 0.5f),  glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(7.55f, 0.0f, 0.2f),  glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(7.55f, 0.0f, 0.8f),  glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(8.1f,  0.0f, -0.1f), glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(8.1f,  0.0f, 0.5f),  glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(8.1f,  0.0f, 1.1f),  glm::vec3(0.0f), glm::vec3(0.1f) },
		{ PIN,       glm::vec3(6.2f,  0.24f, 2.0f), glm::vec3(0.0f, 30.0f, 85.0f), glm::vec3(0.1f) },

		{ CRATE,     glm::vec3(5.0f, 0.0f, 3.5f), glm::vec3(0.0f, -15.0f, 0.0f), glm::vec3(0.5f) },
		{ BUNNY,     glm::vec3(5.0f, 1.0f, 3.5f), glm::vec3(0.0f, -30.0f, 0.0f), glm::vec3(0.45f) },
		{ BUNNY,     glm::vec3(3.2f, 0.0f, 5.2f), glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.3f) }
	};
	const int numInstances = sizeof(scene) / sizeof(scene[0]);

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		glfwPollEvents();
		update(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		view = fpsCamera.getViewMatrix();

		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		glm::vec3 viewPos = fpsCamera.getPosition();

		lightingShader.use();
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", viewPos);

		lightingShader.setUniform("sunLight.direction", glm::vec3(-0.3f, -1.0f, -0.4f));
		lightingShader.setUniform("sunLight.ambient", glm::vec3(0.05f, 0.05f, 0.08f));
		lightingShader.setUniform("sunLight.diffuse", glm::vec3(0.15f, 0.18f, 0.3f));
		lightingShader.setUniform("sunLight.specular", glm::vec3(0.2f, 0.2f, 0.3f));

		glm::vec3 bulbColor[numPointLights];
		for (int i = 0; i < numPointLights; i++)
		{
			float flicker = 0.92f + 0.08f * sinf((float)currentTime * (2.5f + i) + i * 1.7f);
			bulbColor[i] = lampColor[i] * flicker;

			std::ostringstream base;
			base << "pointLights[" << i << "].";
			std::string b = base.str();

			lightingShader.setUniform((b + "ambient").c_str(), glm::vec3(0.0f));
			lightingShader.setUniform((b + "diffuse").c_str(), bulbColor[i] * 1.3f);
			lightingShader.setUniform((b + "specular").c_str(), bulbColor[i]);
			lightingShader.setUniform((b + "position").c_str(), lampPos[i] + glm::vec3(0.0f, BULB_HEIGHT, 0.0f));
			lightingShader.setUniform((b + "constant").c_str(), 1.0f);
			lightingShader.setUniform((b + "linear").c_str(), 0.14f);
			lightingShader.setUniform((b + "exponent").c_str(), 0.07f);
		}

		glm::vec3 spotlightPos = fpsCamera.getPosition();

		spotlightPos.y -= 0.5f;

		lightingShader.setUniform("spotLight.ambient", glm::vec3(0.12f, 0.12f, 0.15f));
		lightingShader.setUniform("spotLight.diffuse", glm::vec3(0.9f, 0.9f, 0.8f));
		lightingShader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("spotLight.position", spotlightPos);
		lightingShader.setUniform("spotLight.direction", fpsCamera.getLook());
		lightingShader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(15.0f)));
		lightingShader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(20.0f)));
		lightingShader.setUniform("spotLight.constant", 1.0f);
		lightingShader.setUniform("spotLight.linear", 0.07f);
		lightingShader.setUniform("spotLight.exponent", 0.017f);
		lightingShader.setUniform("spotLight.on", gFlashlightOn);

		lightingShader.setUniform("material.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniformSampler("material.diffuseMap", 0);
		lightingShader.setUniform("material.specular", glm::vec3(0.6f, 0.6f, 0.6f));
		lightingShader.setUniform("material.shininess", 32.0f);

		for (int i = 0; i <= numInstances; i++)
		{
			Instance inst = (i < numInstances) ? scene[i] : robotInstance;
			if (i == numInstances)
				inst.rotation.y = (float)currentTime * 20.0f;

			model = glm::translate(glm::mat4(1.0), inst.position) *
				glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0), inst.scale);
			lightingShader.setUniform("model", model);

			texture[inst.model].bind(0);
			mesh[inst.model].draw();
			texture[inst.model].unbind(0);
		}

		bulbShader.use();
		bulbShader.setUniform("view", view);
		bulbShader.setUniform("projection", projection);
		for (int i = 0; i < numPointLights; i++)
		{
			model = glm::translate(glm::mat4(1.0), lampPos[i] + glm::vec3(0.0f, BULB_HEIGHT, 0.0f)) *
				glm::scale(glm::mat4(1.0), glm::vec3(0.8f));
			bulbShader.setUniform("model", model);
			bulbShader.setUniform("lightColor", glm::min(bulbColor[i] * 1.2f, glm::vec3(1.0f)));
			bulbMesh.draw();
		}

		glfwSwapBuffers(gWindow);

		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

bool initOpenGL()
{

	if (!glfwInit())
	{

		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(gWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h);
    glViewport(0, 0, w, h);

    glEnable(GL_DEPTH_TEST);

	return true;
}

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{

		gFlashlightOn = !gFlashlightOn;
	}

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{

		if (gTourActive)
			stopTour();
		else
			startTour();
	}
}

void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;

    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h);
    glViewport(0, 0, w, h);
}

void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

void startTour()
{
	gTourActive = true;
	gTourSegment = 0;
	gTourSegmentTime = 0.0f;
	gTourFromPos = fpsCamera.getPosition();
	gTourFromTarget = fpsCamera.getPosition() + fpsCamera.getLook();
}

void stopTour()
{
	gTourActive = false;
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);
}

void updateTour(float elapsedTime)
{
	gTourSegmentTime += elapsedTime;

	while (gTourSegmentTime >= TOUR[gTourSegment].duration)
	{
		gTourSegmentTime -= TOUR[gTourSegment].duration;
		gTourFromPos = TOUR[gTourSegment].position;
		gTourFromTarget = TOUR[gTourSegment].target;
		gTourSegment++;

		if (gTourSegment >= TOUR_SIZE)
		{
			fpsCamera.lookAt(gTourFromPos, gTourFromTarget);
			stopTour();
			return;
		}
	}

	float t = gTourSegmentTime / TOUR[gTourSegment].duration;
	t = t * t * (3.0f - 2.0f * t);

	glm::vec3 pos = glm::mix(gTourFromPos, TOUR[gTourSegment].position, t);
	glm::vec3 target = glm::mix(gTourFromTarget, TOUR[gTourSegment].target, t);
	fpsCamera.lookAt(pos, target);
}

void update(double elapsedTime)
{

	double mouseX, mouseY;

	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	if (gTourActive)
	{
		updateTour((float)elapsedTime);
		return;
	}

	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * glm::vec3(0.0f, 1.0f, 0.0f));
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -glm::vec3(0.0f, 1.0f, 0.0f));
}

void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime();

	elapsedSeconds = currentSeconds - previousSeconds;

	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		std::ostringstream outs;
		outs.precision(3);
		outs << std::fixed
			<< APP_TITLE << "    "
			<< "FPS: " << fps << "    "
			<< "Frame Time: " << msPerFrame << " (ms)"
			<< (gTourActive ? "    [VISITE - T pour arreter]" : "    [T : visite auto]");
		glfwSetWindowTitle(window, outs.str().c_str());

		frameCount = 0;
	}

	frameCount++;
}
