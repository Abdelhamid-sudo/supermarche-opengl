#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <csignal>
#include <algorithm>
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"
#include "CameraPath.h"

const char* APP_TITLE = "NEON NOIR - 2087";
int gWindowWidth = 1280;
int gWindowHeight = 720;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = false;
bool gFogOn = true;
bool gCinematic = false;
bool gPaused = false;
bool gRecording = false;
bool gOffline = false;
float gTime = 0.0f;
float gTourTime = 0.0f;

glm::vec4 gClearColor(0.035f, 0.03f, 0.06f, 1.0f);

FPSCamera fpsCamera(glm::vec3(0.0f, 1.7f, 22.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0;
const float MOUSE_SENSITIVITY = 0.1f;

const float RECORD_FPS = 30.0f;
const float FOG_DENSITY = 0.035f;

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL(bool visible);

enum MeshId { M_QUAD, M_BOX, M_TUBE, M_CONE, M_ROBOT, M_BARREL, M_WOODCRATE, M_CRATE, M_LAMP_POST, M_BULB, M_BUNNY, NUM_MESHES };
enum TexId { T_FACADE_A, T_FACADE_B, T_FACADE_C, T_ASPHALT, T_ASPHALT_SPEC, T_WHITE,
			 T_SIGN_TITLE, T_SIGN_KANJI, T_SIGN_RAMEN, T_SIGN_BAR,
			 T_ROBOT, T_BARREL, T_WOODCRATE, T_CRATE, T_LAMP_POST, T_METAL, T_HALO, NUM_TEXTURES };

const char* MESH_FILES[NUM_MESHES] = {
	"models/nn_quad.obj", "models/nn_box.obj", "models/nn_tube.obj", "models/nn_cone.obj",
	"models/robot.obj", "models/barrel.obj", "models/woodcrate.obj", "models/crate.obj",
	"models/lampPost.obj", "models/light.obj", "models/bunny.obj"
};
const char* TEXTURE_FILES[NUM_TEXTURES] = {
	"textures/nn_facade_a.png", "textures/nn_facade_b.png", "textures/nn_facade_c.png",
	"textures/nn_asphalt.png", "textures/nn_asphalt_spec.png", "textures/nn_white.png",
	"textures/nn_sign_title.png", "textures/nn_sign_kanji.png", "textures/nn_sign_ramen.png", "textures/nn_sign_bar.png",
	"textures/robot_diffuse.jpg", "textures/barrel_diffuse.png", "textures/woodcrate_diffuse.jpg",
	"textures/crate.jpg", "textures/lamp_post_diffuse.png", "textures/nn_drone.png", "textures/nn_halo.png"
};

Mesh gMesh[NUM_MESHES];
Texture2D gTexture[NUM_TEXTURES];

const int NUM_POINT_LIGHTS = 4;
const glm::vec3 CYAN(0.1f, 0.8f, 1.0f);
const glm::vec3 MAGENTA(1.0f, 0.1f, 0.7f);
const glm::vec3 AMBER(1.0f, 0.6f, 0.15f);
const glm::vec3 LAMP_WARM(1.0f, 0.72f, 0.42f);
const glm::vec3 HOLO_CYAN(0.3f, 0.9f, 1.0f);

glm::vec3 pointLightPos[NUM_POINT_LIGHTS] = {
	glm::vec3(-3.0f, 7.0f,  5.0f),
	glm::vec3( 3.5f, 5.5f, -2.0f),
	glm::vec3(-3.5f, 3.2f, -11.0f),
	glm::vec3( 3.3f, 3.7f,  9.0f)
};
glm::vec3 pointLightColor[NUM_POINT_LIGHTS] = { CYAN, MAGENTA, AMBER, LAMP_WARM };
float pointLightLevel[NUM_POINT_LIGHTS];

struct SceneObject
{
	MeshId mesh;
	TexId texture;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 emission;
	int light;
	float shininess;
	bool worldUV;
};
std::vector<SceneObject> gObjects;

SceneObject& addObject(MeshId mesh, TexId tex, glm::vec3 pos, glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
{
	SceneObject o = { mesh, tex, pos, rot, scale, glm::vec3(0.0f), -1, 32.0f, false };
	gObjects.push_back(o);
	return gObjects.back();
}

struct Halo
{
	glm::vec3 position;
	glm::vec3 color;
	float size;
	int light;
};
std::vector<Halo> gHalos;

void addBuilding(float x, float z, float width, float depth, float height, TexId facade, glm::vec3 windowColor)
{
	SceneObject& b = addObject(M_BOX, facade, glm::vec3(x, 0.0f, z), glm::vec3(0.0f), glm::vec3(width, height, depth));
	b.worldUV = true;
	b.emission = windowColor;
	b.shininess = 16.0f;
	SceneObject& roof = addObject(M_BOX, T_ASPHALT, glm::vec3(x, height, z), glm::vec3(0.0f), glm::vec3(width + 0.4f, 0.4f, depth + 0.4f));
	roof.worldUV = true;
}

void addSign(TexId tex, glm::vec3 pos, float rotY, float width, float height, int light, float haloSize)
{
	SceneObject& s = addObject(M_QUAD, tex, pos, glm::vec3(0.0f, rotY, 0.0f), glm::vec3(width, height, 1.0f));
	s.emission = pointLightColor[light] * 1.8f;
	s.light = light;
	s.shininess = 64.0f;
	Halo h = { pos + glm::vec3(pos.x > 0.0f ? -0.8f : 0.8f, 0.0f, 0.0f), pointLightColor[light], haloSize, light };
	gHalos.push_back(h);
}

const glm::vec3 ANDROID_POS(0.0f, 0.0f, -6.0f);
const glm::vec3 HOLO_POS(2.6f, 0.0f, -9.0f);
const glm::vec3 MOON_POS(-70.0f, 75.0f, -160.0f);

void buildScene()
{

	SceneObject& floor = addObject(M_QUAD, T_ASPHALT, glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(200.0f, 200.0f, 1.0f));
	floor.worldUV = true;

	const glm::vec3 warm(0.9f, 0.7f, 0.5f), cold(0.55f, 0.7f, 0.9f), dim(0.35f, 0.3f, 0.3f);
	addBuilding(-9.5f, 14.0f, 10.0f, 12.0f, 16.0f, T_FACADE_C, warm);
	addBuilding(-9.5f, 1.0f, 10.0f, 13.5f, 24.0f, T_FACADE_A, cold);
	addBuilding(-9.5f, -13.0f, 10.0f, 14.0f, 30.0f, T_FACADE_B, dim);
	addBuilding(-9.5f, -25.0f, 10.0f, 9.5f, 20.0f, T_FACADE_C, warm);
	addBuilding(9.5f, 13.0f, 10.0f, 14.0f, 20.0f, T_FACADE_B, cold);
	addBuilding(9.5f, -1.0f, 10.0f, 13.5f, 14.0f, T_FACADE_C, warm);
	addBuilding(9.5f, -14.5f, 10.0f, 13.0f, 28.0f, T_FACADE_A, warm);
	addBuilding(9.5f, -25.5f, 10.0f, 8.5f, 34.0f, T_FACADE_B, dim);
	addBuilding(0.0f, -36.0f, 24.0f, 8.0f, 45.0f, T_FACADE_B, cold);

	struct Tower { float x, z, w, h; TexId tex; };
	const Tower towers[] = {
		{ -30.0f, -60.0f, 14.0f, 38.0f, T_FACADE_B }, { -16.0f, -82.0f, 12.0f, 52.0f, T_FACADE_A },
		{ 16.0f, -75.0f, 16.0f, 46.0f, T_FACADE_B }, { 32.0f, -55.0f, 12.0f, 34.0f, T_FACADE_A },
		{ 2.0f, -110.0f, 18.0f, 60.0f, T_FACADE_C }, { -48.0f, -100.0f, 16.0f, 50.0f, T_FACADE_C },
		{ 48.0f, -95.0f, 14.0f, 56.0f, T_FACADE_A }, { -28.0f, -20.0f, 12.0f, 30.0f, T_FACADE_A },
		{ 28.0f, -25.0f, 12.0f, 34.0f, T_FACADE_C }, { -30.0f, 10.0f, 12.0f, 22.0f, T_FACADE_B },
		{ 30.0f, 5.0f, 12.0f, 26.0f, T_FACADE_A }, { 75.0f, -130.0f, 20.0f, 64.0f, T_FACADE_C },
		{ -80.0f, -135.0f, 20.0f, 62.0f, T_FACADE_B }
	};
	for (size_t i = 0; i < sizeof(towers) / sizeof(towers[0]); i++)
		addBuilding(towers[i].x, towers[i].z, towers[i].w, towers[i].w, towers[i].h, towers[i].tex, (i % 2) ? warm : cold);

	addSign(T_SIGN_KANJI, glm::vec3(-3.7f, 7.0f, 5.0f), 0.0f, 1.1f, 4.4f, 0, 4.0f);
	addSign(T_SIGN_TITLE, glm::vec3(4.45f, 5.5f, -2.0f), -90.0f, 6.0f, 2.25f, 1, 5.0f);
	addSign(T_SIGN_RAMEN, glm::vec3(-4.45f, 3.2f, -11.0f), 90.0f, 5.0f, 1.56f, 2, 4.5f);
	SceneObject& giant = addObject(M_QUAD, T_SIGN_TITLE, glm::vec3(0.0f, 38.0f, -31.9f), glm::vec3(0.0f), glm::vec3(14.0f, 5.25f, 1.0f));
	giant.emission = glm::vec3(0.25f, 0.6f, 1.0f) * 1.8f;
	SceneObject& bar = addObject(M_QUAD, T_SIGN_BAR, glm::vec3(4.45f, 3.0f, 14.0f), glm::vec3(0.0f, -90.0f, 0.0f), glm::vec3(3.0f, 1.5f, 1.0f));
	bar.emission = glm::vec3(1.0f, 0.15f, 0.12f) * 1.8f;

	SceneObject& tube1 = addObject(M_TUBE, T_WHITE, glm::vec3(-4.4f, 4.4f, -6.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.12f, 10.0f, 0.12f));
	tube1.emission = CYAN * 1.5f;
	tube1.light = 0;
	SceneObject& tube2 = addObject(M_TUBE, T_WHITE, glm::vec3(4.4f, 3.0f, -20.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.12f, 12.0f, 0.12f));
	tube2.emission = MAGENTA * 1.5f;
	tube2.light = 1;

	addObject(M_LAMP_POST, T_LAMP_POST, glm::vec3(3.6f, 0.0f, 9.0f));
	Halo lampHalo = { pointLightPos[3], LAMP_WARM, 2.5f, 3 };
	gHalos.push_back(lampHalo);

	addObject(M_BARREL, T_BARREL, glm::vec3(-3.6f, 0.0f, 8.5f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.45f));
	addObject(M_BARREL, T_BARREL, glm::vec3(-2.9f, 0.0f, 9.6f), glm::vec3(0.0f, 75.0f, 0.0f), glm::vec3(0.45f));
	addObject(M_BARREL, T_BARREL, glm::vec3(3.6f, 0.0f, -15.0f), glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.45f));
	addObject(M_WOODCRATE, T_WOODCRATE, glm::vec3(3.5f, 0.0f, 3.5f), glm::vec3(0.0f, 8.0f, 0.0f), glm::vec3(0.5f));
	addObject(M_WOODCRATE, T_WOODCRATE, glm::vec3(3.6f, 1.0f, 3.6f), glm::vec3(0.0f, 28.0f, 0.0f), glm::vec3(0.42f));
	addObject(M_WOODCRATE, T_WOODCRATE, glm::vec3(-3.5f, 0.0f, -18.0f), glm::vec3(0.0f, 33.0f, 0.0f), glm::vec3(0.55f));
	for (int i = 0; i < 4; i++)
		addObject(M_CRATE, T_CRATE, glm::vec3((i % 2 ? 4.15f : -4.15f), 4.5f + i * 1.2f, 12.0f - i * 8.0f), glm::vec3(0.0f), glm::vec3(0.35f, 0.3f, 0.45f));
	for (int i = 0; i < 3; i++)
		addObject(M_TUBE, T_METAL, glm::vec3(4.5f, 9.0f + i * 1.5f, 10.0f - i * 13.0f), glm::vec3(0.0f, 0.0f, 90.0f), glm::vec3(0.05f, 9.0f, 0.05f));

	SceneObject& android = addObject(M_ROBOT, T_ROBOT, ANDROID_POS, glm::vec3(0.0f), glm::vec3(0.4f));
	android.shininess = 64.0f;

	addObject(M_TUBE, T_METAL, HOLO_POS, glm::vec3(0.0f), glm::vec3(0.8f, 0.2f, 0.8f));
	Halo holoHalo = { HOLO_POS + glm::vec3(0.0f, 0.3f, 0.0f), HOLO_CYAN, 1.8f, -1 };
	gHalos.push_back(holoHalo);
}

CameraPath gTourPath;
CameraPath gDronePath;

void buildPaths()
{

	gTourPath.addKey(0.0f,  glm::vec3(0.6f, 1.2f, 20.0f),  glm::vec3(0.0f, 2.0f, 0.0f));
	gTourPath.addKey(6.0f,  glm::vec3(0.5f, 1.1f, 9.0f),   glm::vec3(0.0f, 1.8f, -6.0f));
	gTourPath.addKey(11.0f, glm::vec3(-3.0f, 1.5f, -2.6f), glm::vec3(0.0f, 1.4f, -6.0f));
	gTourPath.addKey(15.0f, glm::vec3(-0.2f, 1.7f, -1.4f), glm::vec3(0.0f, 1.5f, -6.0f));
	gTourPath.addKey(19.0f, glm::vec3(3.1f, 1.6f, -3.0f),  glm::vec3(0.0f, 1.6f, -6.5f));
	gTourPath.addKey(24.0f, glm::vec3(1.2f, 2.5f, 8.0f),   glm::vec3(0.0f, 3.0f, -8.0f));
	gTourPath.addKey(30.0f, glm::vec3(0.2f, 26.0f, 36.0f), glm::vec3(0.0f, 17.0f, -40.0f));
	gTourPath.addKey(32.0f, glm::vec3(0.15f, 27.0f, 37.5f), glm::vec3(0.0f, 17.5f, -41.0f));

	gDronePath.addKey(0.0f,  glm::vec3(1.2f, 8.5f, -24.0f),  glm::vec3(1.5f, 0.0f, -18.0f));
	gDronePath.addKey(6.0f,  glm::vec3(-0.8f, 8.0f, -14.0f), glm::vec3(-1.0f, 0.0f, -9.0f));
	gDronePath.addKey(10.0f, glm::vec3(0.3f, 7.2f, -10.0f),  glm::vec3(0.0f, 1.5f, -6.0f));
	gDronePath.addKey(14.0f, glm::vec3(-0.2f, 7.2f, -9.6f),  glm::vec3(0.0f, 1.5f, -6.0f));
	gDronePath.addKey(19.0f, glm::vec3(-1.2f, 8.0f, -1.0f),  glm::vec3(-1.5f, 0.0f, 4.0f));
	gDronePath.addKey(24.0f, glm::vec3(0.8f, 8.5f, 9.0f),    glm::vec3(1.5f, 0.0f, 13.0f));
	gDronePath.addKey(29.0f, glm::vec3(1.6f, 8.5f, 0.0f),    glm::vec3(2.5f, 0.0f, -5.0f));
	gDronePath.addKey(34.0f, glm::vec3(1.0f, 9.0f, -14.0f),  glm::vec3(1.0f, 0.0f, -20.0f));
	gDronePath.addKey(40.0f, glm::vec3(1.2f, 8.5f, -24.0f),  glm::vec3(1.5f, 0.0f, -18.0f));
	gDronePath.setLoop(true);
}

float neonFlicker(int i, float t)
{
	if (i != 1)
		return 1.0f;
	float r = glm::fract(sinf(floorf(t * 8.0f) * 12.9898f) * 43758.5453f);
	return (r < 0.07f) ? 0.3f : 1.0f;
}

glm::mat4 gDroneHead(1.0f);
glm::mat4 gDroneBody(1.0f);
float gDroneYaw = 0.0f;

void updateDrone(float t)
{
	glm::vec3 pos(0.0f), target(0.0f, 0.0f, -1.0f);
	gDronePath.sample(t, pos, target);
	pos.y += 0.12f * sinf(t * 1.8f);

	glm::vec3 toTarget = target - pos;
	float horizontal = sqrtf(toTarget.x * toTarget.x + toTarget.z * toTarget.z);
	if (horizontal > 0.3f)
		gDroneYaw = glm::degrees(atan2f(-toTarget.x, -toTarget.z));
	float pitch = glm::degrees(atan2f(toTarget.y, horizontal));

	gDroneBody = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), glm::radians(gDroneYaw), glm::vec3(0.0f, 1.0f, 0.0f));
	gDroneHead = gDroneBody * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.35f, -0.5f))
							* glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
}

const glm::vec3 SPOT_COLOR(0.85f, 0.92f, 1.0f);

void setLightUniforms(ShaderProgram& shader)
{

	shader.setUniform("moonLight.direction", glm::vec3(-0.3f, -1.0f, -0.4f));
	shader.setUniform("moonLight.ambient", glm::vec3(0.05f, 0.05f, 0.08f));
	shader.setUniform("moonLight.diffuse", glm::vec3(0.16f, 0.21f, 0.42f));
	shader.setUniform("moonLight.specular", glm::vec3(0.2f, 0.25f, 0.4f));

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		std::ostringstream base;
		base << "pointLights[" << i << "].";
		glm::vec3 color = pointLightColor[i] * 4.0f * pointLightLevel[i];
		shader.setUniform((base.str() + "ambient").c_str(), glm::vec3(0.0f));
		shader.setUniform((base.str() + "diffuse").c_str(), color);
		shader.setUniform((base.str() + "specular").c_str(), color);
		shader.setUniform((base.str() + "position").c_str(), pointLightPos[i]);
		shader.setUniform((base.str() + "constant").c_str(), 1.0f);
		shader.setUniform((base.str() + "linear").c_str(), 0.22f);
		shader.setUniform((base.str() + "exponent").c_str(), 0.07f);
	}

	glm::vec3 spotPos = glm::vec3(gDroneHead * glm::vec4(0.0f, 0.0f, -0.3f, 1.0f));
	glm::vec3 spotDir = glm::vec3(gDroneHead * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	shader.setUniform("spotLights[0].ambient", glm::vec3(0.0f));
	shader.setUniform("spotLights[0].diffuse", SPOT_COLOR * 6.0f);
	shader.setUniform("spotLights[0].specular", SPOT_COLOR * 6.0f);
	shader.setUniform("spotLights[0].position", spotPos);
	shader.setUniform("spotLights[0].direction", spotDir);
	shader.setUniform("spotLights[0].cosInnerCone", glm::cos(glm::radians(9.0f)));
	shader.setUniform("spotLights[0].cosOuterCone", glm::cos(glm::radians(13.0f)));
	shader.setUniform("spotLights[0].constant", 1.0f);
	shader.setUniform("spotLights[0].linear", 0.02f);
	shader.setUniform("spotLights[0].exponent", 0.004f);
	shader.setUniform("spotLights[0].on", 1);

	glm::vec3 flashPos = fpsCamera.getPosition();
	flashPos.y -= 0.5f;
	shader.setUniform("spotLights[1].ambient", glm::vec3(0.0f));
	shader.setUniform("spotLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setUniform("spotLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setUniform("spotLights[1].position", flashPos);
	shader.setUniform("spotLights[1].direction", fpsCamera.getLook());
	shader.setUniform("spotLights[1].cosInnerCone", glm::cos(glm::radians(15.0f)));
	shader.setUniform("spotLights[1].cosOuterCone", glm::cos(glm::radians(20.0f)));
	shader.setUniform("spotLights[1].constant", 1.0f);
	shader.setUniform("spotLights[1].linear", 0.07f);
	shader.setUniform("spotLights[1].exponent", 0.017f);
	shader.setUniform("spotLights[1].on", gFlashlightOn ? 1 : 0);

	shader.setUniform("fogColor", glm::vec3(gClearColor));
	shader.setUniform("fogDensity", gFogOn ? FOG_DENSITY : 0.0f);
}

void drawMesh(ShaderProgram& shader, MeshId mesh, TexId tex, const glm::mat4& model,
			  glm::vec3 emission = glm::vec3(0.0f), float shininess = 32.0f, bool worldUV = false)
{
	bool isFloor = (tex == T_ASPHALT && mesh == M_QUAD);
	shader.setUniform("model", model);
	shader.setUniform("material.ambient", glm::vec3(1.0f));
	shader.setUniformSampler("material.diffuseMap", 0);
	shader.setUniformSampler("material.specularMap", 1);
	shader.setUniform("material.specular", glm::vec3(0.6f));
	shader.setUniform("material.shininess", shininess);
	shader.setUniform("material.emission", emission);
	shader.setUniform("material.wet", isFloor ? 1.0f : 0.0f);
	shader.setUniform("worldUV", worldUV ? 1 : 0);
	shader.setUniform("uvScale", 0.125f);

	gTexture[tex].bind(0);
	gTexture[isFloor ? T_ASPHALT_SPEC : T_WHITE].bind(1);
	gMesh[mesh].draw();
}

glm::mat4 objectMatrix(const SceneObject& o)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), o.position);
	model = glm::rotate(model, glm::radians(o.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(o.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(o.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return glm::scale(model, o.scale);
}

void drawDrone(ShaderProgram& shader, float t)
{

	drawMesh(shader, M_BOX, T_METAL, gDroneBody * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.15f, 0.0f))
									 * glm::scale(glm::mat4(1.0f), glm::vec3(0.9f, 0.3f, 1.1f)), glm::vec3(0.0f), 48.0f);

	for (int i = 0; i < 4; i++)
	{
		glm::mat4 arm = gDroneBody * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f + 90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
		drawMesh(shader, M_BOX, T_METAL, arm * glm::translate(glm::mat4(1.0f), glm::vec3(0.55f, -0.04f, 0.0f))
										 * glm::scale(glm::mat4(1.0f), glm::vec3(1.1f, 0.08f, 0.1f)));
		glm::mat4 rotor = arm * glm::translate(glm::mat4(1.0f), glm::vec3(1.1f, 0.08f, 0.0f))
							  * glm::rotate(glm::mat4(1.0f), t * 25.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		drawMesh(shader, M_BOX, T_METAL, rotor * glm::scale(glm::mat4(1.0f), glm::vec3(0.9f, 0.02f, 0.08f)));
		drawMesh(shader, M_BOX, T_METAL, rotor * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
											   * glm::scale(glm::mat4(1.0f), glm::vec3(0.9f, 0.02f, 0.08f)));
	}

	drawMesh(shader, M_BOX, T_METAL, gDroneHead * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.15f, 0.05f))
									 * glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.3f, 0.45f)), glm::vec3(0.0f), 48.0f);
}

void drawGlow(ShaderProgram& glow, MeshId mesh, const glm::mat4& model, int mode, glm::vec3 color, float haloSize = 1.0f)
{
	glow.setUniform("model", model);
	glow.setUniform("mode", mode);
	glow.setUniform("color", color);
	glow.setUniform("haloSize", haloSize);
	gMesh[mesh].draw();
}

void drawAdditivePass(ShaderProgram& glow, const glm::mat4& view, const glm::mat4& projection, float t)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glow.use();
	glow.setUniform("view", view);
	glow.setUniform("projection", projection);
	glow.setUniform("viewPos", fpsCamera.getPosition());
	glow.setUniform("time", t);
	glow.setUniform("fogDensity", gFogOn ? FOG_DENSITY : 0.0f);
	glow.setUniformSampler("haloMap", 0);
	gTexture[T_HALO].bind(0);

	glm::mat4 beam = gDroneHead * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.3f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.4f, 2.4f, 14.0f));
	drawGlow(glow, M_CONE, beam, 0, SPOT_COLOR * 0.35f);
	drawGlow(glow, M_QUAD, gDroneHead * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.35f)), 1, SPOT_COLOR, 1.5f);

	float holo = 0.8f + 0.2f * sinf(t * 3.0f);
	glm::mat4 bunny = glm::translate(glm::mat4(1.0f), HOLO_POS + glm::vec3(0.0f, 0.5f + 0.05f * sinf(t * 1.5f), 0.0f))
					* glm::rotate(glm::mat4(1.0f), t * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.4f));
	drawGlow(glow, M_BUNNY, bunny, 2, HOLO_CYAN * holo);

	for (size_t i = 0; i < gHalos.size(); i++)
	{
		float level = (gHalos[i].light >= 0) ? pointLightLevel[gHalos[i].light] : holo;
		drawGlow(glow, M_QUAD, glm::translate(glm::mat4(1.0f), gHalos[i].position), 1, gHalos[i].color * level * 1.3f, gHalos[i].size);
	}

	glow.setUniform("fogDensity", 0.0f);
	drawGlow(glow, M_QUAD, glm::translate(glm::mat4(1.0f), MOON_POS), 1, glm::vec3(0.75f, 0.85f, 1.0f), 22.0f);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

FILE* startFFmpeg(int w, int h)
{
	if (system("command -v ffmpeg > /dev/null 2>&1") != 0)
	{
		std::cerr << "WARNING: ffmpeg is not installed, the sequence is played but no video is written." << std::endl
				  << "         Install it with: sudo apt install ffmpeg" << std::endl;
		return NULL;
	}
	signal(SIGPIPE, SIG_IGN);
	std::ostringstream cmd;
	cmd << "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24 -video_size " << w << "x" << h
		<< " -framerate " << (int)RECORD_FPS << " -i - -vf vflip -c:v libx264 -crf 18 -pix_fmt yuv420p neon_noir_2087.mp4";
	FILE* pipe = popen(cmd.str().c_str(), "w");
	if (pipe)
		std::cout << "Recording neon_noir_2087.mp4 (" << w << "x" << h << ", " << (int)RECORD_FPS << " FPS)..." << std::endl;
	else
		std::cerr << "WARNING: cannot start ffmpeg, no video is written." << std::endl;
	return pipe;
}

void saveScreenshot(const std::vector<unsigned char>& pixels, int w, int h, float t)
{
	std::ostringstream name;
	name << "shot_" << (int)(t * 10.0f + 0.5f) / 10.0f << ".ppm";
	FILE* f = fopen(name.str().c_str(), "wb");
	if (!f)
		return;
	fprintf(f, "P6\n%d %d\n255\n", w, h);
	for (int y = h - 1; y >= 0; y--)
		fwrite(&pixels[(size_t)y * w * 3], 1, (size_t)w * 3, f);
	fclose(f);
	std::cout << "  " << name.str() << std::endl;
}

int main(int argc, char** argv)
{

	std::vector<float> shots;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "--record")
			gRecording = true;
		else if (arg == "--shots" && i + 1 < argc)
		{
			std::stringstream list(argv[++i]);
			std::string item;
			while (std::getline(list, item, ','))
				shots.push_back((float)atof(item.c_str()));
		}
		else
		{
			std::cerr << "Usage: " << argv[0] << " [--record] [--shots t1,t2,...]" << std::endl;
			return -1;
		}
	}
	std::sort(shots.begin(), shots.end());
	gOffline = gRecording || !shots.empty();

	if (!initOpenGL(shots.empty()))
	{

		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader, bulbShader, glowShader;
	lightingShader.loadShaders("shaders/neon.vert", "shaders/neon.frag");
	bulbShader.loadShaders("shaders/bulb.vert", "shaders/bulb.frag");
	glowShader.loadShaders("shaders/glow.vert", "shaders/glow.frag");

	bool ok = true;
	for (int i = 0; i < NUM_MESHES && ok; i++)
		ok = gMesh[i].loadOBJ(MESH_FILES[i]);
	for (int i = 0; i < NUM_TEXTURES && ok; i++)
		ok = gTexture[i].loadTexture(TEXTURE_FILES[i], true);
	if (!ok)
	{
		std::cerr << "Missing model or texture: run the program from the project folder" << std::endl;
		glfwTerminate();
		return -1;
	}

	buildScene();
	buildPaths();
	if (gRecording)
		gCinematic = true;

	int fbWidth, fbHeight;
	glfwGetFramebufferSize(gWindow, &fbWidth, &fbHeight);
	std::vector<unsigned char> pixels((size_t)fbWidth * fbHeight * 3);
	FILE* ffmpeg = gRecording ? startFFmpeg(fbWidth, fbHeight) : NULL;
	size_t nextShot = 0;

	std::cout << "Mouse + W/S/A/D/Z/X: move, wheel: zoom, F: flashlight, F1: wireframe, G: fog, Space: pause, T: camera tour, ESC: quit" << std::endl;

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		if (gOffline)
			deltaTime = 1.0 / RECORD_FPS;

		glfwPollEvents();
		update(deltaTime);
		float t = gTime;

		for (int i = 0; i < NUM_POINT_LIGHTS; i++)
			pointLightLevel[i] = neonFlicker(i, t);
		updateDrone(t);

		bool takeShot = nextShot < shots.size() && t >= shots[nextShot] - 0.001f;
		if (!shots.empty() && !takeShot)
			continue;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		view = fpsCamera.getViewMatrix();

		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 300.0f);

		lightingShader.use();
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", fpsCamera.getPosition());
		setLightUniforms(lightingShader);

		for (size_t i = 0; i < gObjects.size(); i++)
		{
			const SceneObject& o = gObjects[i];
			glm::vec3 emission = o.emission * (o.light >= 0 ? pointLightLevel[o.light] : 1.0f);
			drawMesh(lightingShader, o.mesh, o.texture, objectMatrix(o), emission, o.shininess, o.worldUV);
		}
		drawDrone(lightingShader, t);

		bulbShader.use();
		bulbShader.setUniform("view", view);
		bulbShader.setUniform("projection", projection);
		bulbShader.setUniform("model", glm::translate(glm::mat4(1.0f), pointLightPos[3]));
		bulbShader.setUniform("lightColor", LAMP_WARM * pointLightLevel[3]);
		gMesh[M_BULB].draw();
		bulbShader.setUniform("model", gDroneHead * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.25f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f)));
		bulbShader.setUniform("lightColor", SPOT_COLOR);
		gMesh[M_BULB].draw();

		drawAdditivePass(glowShader, view, projection, t);

		if (ffmpeg || takeShot)
		{
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadBuffer(GL_BACK);
			glReadPixels(0, 0, fbWidth, fbHeight, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
			if (ffmpeg && fwrite(&pixels[0], 1, pixels.size(), ffmpeg) != pixels.size())
			{
				std::cerr << "WARNING: ffmpeg stopped, the video is incomplete." << std::endl;
				pclose(ffmpeg);
				ffmpeg = NULL;
			}
			if (takeShot)
			{
				saveScreenshot(pixels, fbWidth, fbHeight, t);
				nextShot++;
			}
		}

		glfwSwapBuffers(gWindow);

		if ((gRecording && !gCinematic) || (!shots.empty() && nextShot >= shots.size()))
			break;
	}

	if (ffmpeg)
	{
		if (pclose(ffmpeg) == 0)
			std::cout << "Video written: neon_noir_2087.mp4" << std::endl;
		else
			std::cerr << "WARNING: ffmpeg reported an error." << std::endl;
	}

	glfwTerminate();

	return 0;
}

bool initOpenGL(bool visible)
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
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, visible ? GL_TRUE : GL_FALSE);

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
		glfwTerminate();
		return false;
	}

	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

	int w, h;
	glfwGetFramebufferSize(gWindow, &w, &h);
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

	if (gRecording)
		return;

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{

		gCinematic = !gCinematic;
		gTourTime = 0.0f;
		glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		gPaused = !gPaused;

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		gFogOn = !gFogOn;
}

void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;

	int w, h;
	glfwGetFramebufferSize(gWindow, &w, &h);
	glViewport(0, 0, w, h);
}

void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

void update(double elapsedTime)
{

	if (!gPaused)
		gTime += (float)elapsedTime;

	if (gCinematic)
	{
		gTourTime += (float)elapsedTime;
		glm::vec3 camPos(0.0f), camTarget(0.0f, 0.0f, -1.0f);
		gTourPath.sample(gTourTime, camPos, camTarget);
		fpsCamera.lookAt(camPos, camTarget);
		if (gTourTime >= gTourPath.endTime())
			gCinematic = false;
		glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);
		return;
	}
	if (gOffline)
		return;

	double mouseX, mouseY;

	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

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
			<< "Frame Time: " << msPerFrame << " (ms)    "
			<< (gCinematic ? "[camera tour]" : "[manual]") << (gPaused ? "  [pause]" : "");
		glfwSetWindowTitle(window, outs.str().c_str());

		frameCount = 0;
	}

	frameCount++;
}
