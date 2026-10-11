#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"

const char* APP_TITLE = "SUPERMARCHE - OpenGL";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = false;
float gTime = 0.0f;
bool gShotMode = false;
glm::vec4 gClearColor(0.62f, 0.78f, 0.94f, 1.0f);

FPSCamera fpsCamera(glm::vec3(0.0f, 1.7f, 19.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0;
const float MOUSE_SENSITIVITY = 0.1f;

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

enum MeshId { M_BOX, M_TUBE, M_QUAD, M_WOODCRATE, M_ROBOT,
			  M_ROW4_BOX, M_ROW4_CAN, M_ROW4_BOTTLE, M_ROW6_BOX, M_ROW6_CAN, M_ROW6_BOTTLE,
			  M_FRUITS_SMALL, M_FRUITS_BIG,
			  M_BEAM, M_DUST, M_CARTONS_A, M_CARTONS_B,
			  M_CAT_BODY, M_CAT_HEAD, M_CAT_EYES, M_CAT_TAIL, NUM_MESHES };
enum TexId { T_WHITE, T_FLOOR, T_CEILING, T_ASPHALT, T_LABEL_BOX, T_LABEL_CAN, T_PRICE_STRIP, T_SHADOW, T_WOODCRATE, T_ROBOT,
			 T_SIGN_TITLE, T_SIGN_EPICERIE, T_SIGN_PETITDEJ, T_SIGN_SNACKS, T_SIGN_BOISSONS, T_SIGN_HYGIENE,
			 T_SIGN_FRAIS, T_SIGN_FRUITS, T_SIGN_CAISSES, T_POSTER,
			 T_CONCRETE, T_CARDBOARD, T_FRAGILE, T_CAT_FUR, T_EMPLOYEE, T_CROQUETTES, T_SIGN_RESERVE, T_SIGN_EXIT, NUM_TEXTURES };

const char* MESH_FILES[NUM_MESHES] = {
	"models/nn_box.obj", "models/nn_tube.obj", "models/nn_quad.obj", "models/woodcrate.obj", "models/robot.obj",
	"models/sm_row4_box.obj", "models/sm_row4_can.obj", "models/sm_row4_bottle.obj",
	"models/sm_row6_box.obj", "models/sm_row6_can.obj", "models/sm_row6_bottle.obj",
	"models/sm_fruits_small.obj", "models/sm_fruits_big.obj",
	"models/sm_beam.obj", "models/sm_dust.obj", "models/sm_cartons_a.obj", "models/sm_cartons_b.obj",
	"models/sm_cat_body.obj", "models/sm_cat_head.obj", "models/sm_cat_eyes.obj", "models/sm_cat_tail.obj"
};
const char* TEXTURE_FILES[NUM_TEXTURES] = {
	"textures/nn_white.png", "textures/sm_floor.png", "textures/sm_ceiling.png", "textures/nn_asphalt.png",
	"textures/sm_label_box.png", "textures/sm_label_can.png", "textures/sm_price_strip.png", "textures/sm_shadow.png",
	"textures/woodcrate_diffuse.jpg", "textures/robot_diffuse.jpg",
	"textures/sm_sign_title.png", "textures/sm_sign_epicerie.png", "textures/sm_sign_petitdej.png",
	"textures/sm_sign_snacks.png", "textures/sm_sign_boissons.png", "textures/sm_sign_hygiene.png",
	"textures/sm_sign_frais.png", "textures/sm_sign_fruits.png", "textures/sm_sign_caisses.png",
	"textures/sm_poster.png",
	"textures/sm_concrete.png", "textures/sm_cardboard.png", "textures/sm_fragile.png", "textures/sm_cat_fur.png",
	"textures/sm_employee.png", "textures/sm_croquettes.png", "textures/sm_sign_reserve.png", "textures/sm_sign_exit.png"
};

Mesh gMesh[NUM_MESHES];
Texture2D gTexture[NUM_TEXTURES];

const float STORE_X = 12.0f;
const float STORE_BACK = -14.0f;
const float STORE_FRONT = 10.0f;
const float STORE_HEIGHT = 4.5f;
const float WALL = 0.3f;
const float DOOR_HALF = 2.5f;
const float DOOR_HEIGHT = 3.0f;

const int NUM_AISLES = 4;
const float AISLE_X[NUM_AISLES] = { -6.75f, -2.25f, 2.25f, 6.75f };
const float AISLE_Z = -5.5f;
const float AISLE_LENGTH = 9.0f;
const float WALL_SHELF_Z = -6.0f;
const float WALL_SHELF_LENGTH = 10.0f;
const float CHECKOUT_X[2] = { -9.0f, -5.5f };
const float CHECKOUT_Z = 5.0f;
const glm::vec3 FRUIT_STAND_POS(7.5f, 0.0f, 3.8f);
const float PILLAR_X = 2.25f, PILLAR_Z = 2.5f;

const float RES_X0 = 2.0f;
const float RES_FRONT = STORE_BACK - WALL;
const float RES_BACK = -21.0f;
const float RES_HEIGHT = 3.6f;
const float RES_DOOR_X0 = 7.9f;
const float RES_DOOR_X1 = 9.1f;
const float RES_DOOR_H = 2.3f;
const float RES_DOOR_AJAR = 28.0f;
const glm::vec3 SHELF_TOP_POS(2.3f, 1.95f, -17.4f);

const int NUM_LEVELS = 4;
const float SHELF_LEVELS[NUM_LEVELS] = { 0.12f, 0.56f, 1.0f, 1.44f };

const glm::vec3 WALL_COLOR(0.93f, 0.91f, 0.86f);
const glm::vec3 GREEN(0.10f, 0.47f, 0.28f);
const glm::vec3 ORANGE(0.96f, 0.55f, 0.12f);
const glm::vec3 METAL(0.80f, 0.82f, 0.85f);
const glm::vec3 DARK(0.22f, 0.23f, 0.25f);
const glm::vec3 SHELF_COLOR(0.90f, 0.91f, 0.92f);
const glm::vec3 WOOD(0.62f, 0.43f, 0.25f);
const glm::vec3 WHITE(1.0f);
const glm::vec3 BLACK(0.0f);

struct Obstacle
{
	float x, z, w, d;
	bool shadow;
};
const int MAX_OBSTACLES = 48;
Obstacle gObstacles[MAX_OBSTACLES];
int gNumObstacles = 0;

void addObstacle(float x, float z, float w, float d, bool shadow)
{
	if (gNumObstacles < MAX_OBSTACLES)
	{
		Obstacle o = { x, z, w, d, shadow };
		gObstacles[gNumObstacles++] = o;
	}
}

void buildObstacles()
{

	float frontPiece = STORE_X + WALL - DOOR_HALF;
	addObstacle(-STORE_X - WALL / 2, -2.0f, WALL, 24.6f, false);
	addObstacle( STORE_X + WALL / 2, -2.0f, WALL, 24.6f, false);

	addObstacle((-STORE_X - WALL + RES_DOOR_X0) / 2, STORE_BACK - WALL / 2, RES_DOOR_X0 + STORE_X + WALL, WALL, false);
	addObstacle((RES_DOOR_X1 + STORE_X + WALL) / 2, STORE_BACK - WALL / 2, STORE_X + WALL - RES_DOOR_X1, WALL, false);
	addObstacle(-DOOR_HALF - frontPiece / 2, STORE_FRONT + WALL / 2, frontPiece, WALL, false);
	addObstacle( DOOR_HALF + frontPiece / 2, STORE_FRONT + WALL / 2, frontPiece, WALL, false);

	for (int i = 0; i < NUM_AISLES; i++)
		addObstacle(AISLE_X[i], AISLE_Z + 0.35f, 1.0f, AISLE_LENGTH + 0.7f, true);
	addObstacle(-STORE_X + 0.25f, WALL_SHELF_Z, 0.5f, WALL_SHELF_LENGTH, true);
	addObstacle( STORE_X - 0.25f, WALL_SHELF_Z, 0.5f, WALL_SHELF_LENGTH, true);
	addObstacle(-2.0f, STORE_BACK + 0.45f, 16.0f, 0.9f, true);
	for (int i = 0; i < 2; i++)
		addObstacle(CHECKOUT_X[i], CHECKOUT_Z, 0.9f, 3.0f, true);
	addObstacle(CHECKOUT_X[0] - 0.95f, CHECKOUT_Z + 0.55f, 0.5f, 0.5f, false);
	addObstacle(FRUIT_STAND_POS.x, FRUIT_STAND_POS.z, 4.0f, 1.3f, true);
	addObstacle(FRUIT_STAND_POS.x, FRUIT_STAND_POS.z + 3.0f, 4.0f, 1.3f, true);
	addObstacle(-PILLAR_X, PILLAR_Z, 0.5f, 0.5f, true);
	addObstacle( PILLAR_X, PILLAR_Z, 0.5f, 0.5f, true);
	addObstacle(4.0f, 8.4f, 0.6f, 1.8f, true);

	float resDepth = RES_FRONT - RES_BACK;
	addObstacle(RES_X0 - WALL / 2, (RES_FRONT + RES_BACK) / 2, WALL, resDepth, false);
	addObstacle(STORE_X + WALL / 2, (RES_FRONT + RES_BACK) / 2, WALL, resDepth, false);
	addObstacle((RES_X0 + STORE_X) / 2, RES_BACK - WALL / 2, STORE_X - RES_X0 + 2 * WALL, WALL, false);
	addObstacle(SHELF_TOP_POS.x, SHELF_TOP_POS.z, 0.6f, 4.2f, false);
	addObstacle(7.25f, RES_BACK + 0.55f, 8.1f, 1.1f, false);
	addObstacle(5.4f, -18.2f, 1.0f, 1.2f, false);
	addObstacle(11.3f, -15.3f, 0.6f, 0.9f, false);
}

bool isBlocked(float x, float z)
{
	const float RADIUS = 0.3f;
	for (int i = 0; i < gNumObstacles; i++)
	{
		const Obstacle& o = gObstacles[i];
		if (fabsf(x - o.x) < o.w / 2 + RADIUS && fabsf(z - o.z) < o.d / 2 + RADIUS)
			return true;
	}
	return false;
}

float random01(int seed)
{
	return glm::fract(sinf(seed * 12.9898f + 78.233f) * 43758.5453f);
}

void drawMesh(ShaderProgram& shader, MeshId mesh, TexId tex, const glm::mat4& model, glm::vec3 color,
			  float shininess = 16.0f, glm::vec3 emission = glm::vec3(0.0f), float specular = 0.3f)
{
	shader.setUniform("model", model);
	shader.setUniform("material.ambient", glm::vec3(1.0f));
	shader.setUniformSampler("material.diffuseMap", 0);
	shader.setUniform("material.specular", glm::vec3(specular));
	shader.setUniform("material.shininess", shininess);
	shader.setUniform("material.color", color);
	shader.setUniform("material.emission", emission);

	gTexture[tex].bind(0);
	gMesh[mesh].draw();
}

void drawCube(ShaderProgram& shader, const glm::mat4& parent, glm::vec3 pos, glm::vec3 size, glm::vec3 color,
			  TexId tex = T_WHITE, float shininess = 16.0f, glm::vec3 emission = glm::vec3(0.0f))
{
	glm::mat4 model = glm::translate(parent, pos) * glm::scale(glm::mat4(1.0f), size);
	drawMesh(shader, M_BOX, tex, model, color, shininess, emission);
}

void drawCylinder(ShaderProgram& shader, const glm::mat4& parent, glm::vec3 pos, float radius, float height, glm::vec3 color,
				  TexId tex = T_WHITE, float shininess = 32.0f)
{
	glm::mat4 model = glm::translate(parent, pos) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * radius, height, 2.0f * radius));
	drawMesh(shader, M_TUBE, tex, model, color, shininess);
}

void drawPanel(ShaderProgram& shader, const glm::mat4& parent, glm::vec3 pos, float rotY, float width, float height,
			   TexId tex, float glow = 0.25f)
{
	glm::mat4 model = glm::translate(parent, pos)
					* glm::rotate(glm::mat4(1.0f), glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
	drawMesh(shader, M_QUAD, tex, model, WHITE, 32.0f, glm::vec3(glow));
}

enum Shape { S_BOX, S_CAN, S_BOTTLE };
enum ProductKind { P_CEREAL, P_SMALLBOX, P_BAG, P_CAN, P_JAR, P_BOTTLE, P_BIGBOTTLE, P_PACK, P_YOGURT, P_CHEESE, NUM_KINDS };

struct Product
{
	Shape shape;
	glm::vec3 size;
	float tilt;
};

const Product PRODUCTS[NUM_KINDS] = {
	{ S_BOX,    glm::vec3(0.22f, 0.30f, 0.20f),   0.0f },
	{ S_BOX,    glm::vec3(0.14f, 0.18f, 0.14f),   0.0f },
	{ S_BOX,    glm::vec3(0.20f, 0.27f, 0.10f),  -8.0f },
	{ S_CAN,    glm::vec3(0.08f, 0.13f, 0.08f),   0.0f },
	{ S_CAN,    glm::vec3(0.10f, 0.14f, 0.10f),   0.0f },
	{ S_BOTTLE, glm::vec3(0.09f, 0.30f, 0.09f),   0.0f },
	{ S_BOTTLE, glm::vec3(0.12f, 0.34f, 0.12f),   0.0f },
	{ S_BOX,    glm::vec3(0.30f, 0.26f, 0.28f),   0.0f },
	{ S_CAN,    glm::vec3(0.075f, 0.09f, 0.075f), 0.0f },
	{ S_BOX,    glm::vec3(0.15f, 0.07f, 0.15f),   0.0f }
};

void drawProducts(ShaderProgram& shader, const glm::mat4& parent, ProductKind kind, int count, glm::vec3 pos, glm::vec3 color)
{
	const Product& p = PRODUCTS[kind];
	MeshId mesh = (MeshId)((count == 6 ? M_ROW6_BOX : M_ROW4_BOX) + p.shape);
	TexId tex = (p.shape == S_BOX) ? T_LABEL_BOX : T_LABEL_CAN;
	float shininess = (p.shape == S_CAN) ? 96.0f : 48.0f;
	glm::mat4 model = glm::translate(parent, pos)
					* glm::rotate(glm::mat4(1.0f), glm::radians(p.tilt), glm::vec3(1.0f, 0.0f, 0.0f))
					* glm::scale(glm::mat4(1.0f), p.size);
	drawMesh(shader, mesh, tex, model, color, shininess, glm::vec3(0.0f), 0.4f);
}

float rowWidth(ProductKind kind, int count)
{
	return count * 1.1f * PRODUCTS[kind].size.x;
}

void drawProduct(ShaderProgram& shader, const glm::mat4& parent, ProductKind kind, glm::vec3 pos, glm::vec3 color)
{
	const Product& p = PRODUCTS[kind];
	if (p.shape == S_BOX)
		drawCube(shader, parent, pos, p.size, color, T_LABEL_BOX, 48.0f);
	else
		drawCylinder(shader, parent, pos, p.size.x / 2, p.size.y, color, T_LABEL_CAN, 96.0f);
}

enum CategoryId { C_EPICERIE, C_PETITDEJ, C_SNACKS, C_BOISSONS, C_HYGIENE, C_FRAIS, NUM_CATEGORIES };

struct Category
{
	TexId sign;
	glm::vec3 color;
	ProductKind kinds[3];
	glm::vec3 palette[4];
};

const Category CATEGORIES[NUM_CATEGORIES] = {
	{ T_SIGN_EPICERIE, glm::vec3(0.78f, 0.20f, 0.18f), { P_CAN, P_JAR, P_SMALLBOX },
	  { glm::vec3(0.80f, 0.15f, 0.12f), glm::vec3(0.20f, 0.55f, 0.25f), glm::vec3(0.95f, 0.75f, 0.15f), glm::vec3(0.18f, 0.35f, 0.70f) } },
	{ T_SIGN_PETITDEJ, glm::vec3(0.88f, 0.63f, 0.08f), { P_CEREAL, P_JAR, P_SMALLBOX },
	  { glm::vec3(0.95f, 0.55f, 0.12f), glm::vec3(0.98f, 0.82f, 0.25f), glm::vec3(0.55f, 0.32f, 0.15f), glm::vec3(0.85f, 0.20f, 0.20f) } },
	{ T_SIGN_SNACKS, glm::vec3(0.92f, 0.41f, 0.10f), { P_BAG, P_BAG, P_SMALLBOX },
	  { glm::vec3(1.00f, 0.80f, 0.10f), glm::vec3(0.90f, 0.15f, 0.10f), glm::vec3(0.20f, 0.45f, 0.90f), glm::vec3(0.35f, 0.75f, 0.20f) } },
	{ T_SIGN_BOISSONS, glm::vec3(0.12f, 0.39f, 0.78f), { P_BOTTLE, P_BIGBOTTLE, P_CAN },
	  { glm::vec3(0.55f, 0.75f, 0.95f), glm::vec3(0.75f, 0.10f, 0.10f), glm::vec3(1.00f, 0.60f, 0.10f), glm::vec3(0.40f, 0.80f, 0.25f) } },
	{ T_SIGN_HYGIENE, glm::vec3(0.47f, 0.29f, 0.71f), { P_BIGBOTTLE, P_PACK, P_BOTTLE },
	  { glm::vec3(0.95f, 0.55f, 0.70f), glm::vec3(0.50f, 0.85f, 0.75f), glm::vec3(0.65f, 0.55f, 0.90f), glm::vec3(0.95f, 0.95f, 0.95f) } },
	{ T_SIGN_FRAIS, glm::vec3(0.08f, 0.59f, 0.75f), { P_YOGURT, P_BOTTLE, P_CHEESE },
	  { glm::vec3(0.97f, 0.97f, 0.95f), glm::vec3(1.00f, 0.88f, 0.55f), glm::vec3(0.60f, 0.80f, 1.00f), glm::vec3(0.95f, 0.50f, 0.55f) } }
};

void drawShelfProducts(ShaderProgram& shader, const glm::mat4& parent, float length, float depth, float y, CategoryId category, int seed)
{
	const Category& c = CATEGORIES[category];
	float x = -length / 2 + 0.04f;
	for (int group = 0; ; group++)
	{
		int r = seed * 101 + group * 7;
		ProductKind kind = c.kinds[(int)(random01(r) * 3.0f)];
		glm::vec3 color = c.palette[(int)(random01(r + 1) * 4.0f)];
		int count = (random01(r + 2) < 0.5f) ? 4 : 6;
		if (x + rowWidth(kind, count) > length / 2 - 0.04f)
			count = 4;
		float width = rowWidth(kind, count);
		if (x + width > length / 2 - 0.04f)
			break;

		if (random01(r + 3) > 0.08f)
			drawProducts(shader, parent, kind, count, glm::vec3(x + width / 2, y, depth - 0.03f - PRODUCTS[kind].size.z / 2), color);
		x += width + 0.05f;
	}
}

void drawShelf(ShaderProgram& shader, const glm::mat4& parent, float length, float depth, CategoryId category, int seed)
{
	for (int level = 0; level < NUM_LEVELS; level++)
	{
		float y = SHELF_LEVELS[level];
		if (level > 0)
			drawCube(shader, parent, glm::vec3(0.0f, y - 0.03f, depth / 2), glm::vec3(length, 0.03f, depth), SHELF_COLOR, T_WHITE, 32.0f);

		shader.setUniform("worldUV", 1);
		shader.setUniform("uvScale", 1.0f / 0.6f);
		drawCube(shader, parent, glm::vec3(0.0f, y - 0.06f, depth), glm::vec3(length, 0.06f, 0.012f), WHITE, T_PRICE_STRIP);
		shader.setUniform("worldUV", 0);
		drawShelfProducts(shader, parent, length, depth, y, category, seed * 10 + level);
	}
}

void drawHangingSign(ShaderProgram& shader, glm::vec3 pos, float width, TexId tex)
{
	glm::mat4 world(1.0f);
	float height = width / 4.0f;
	drawCube(shader, world, pos, glm::vec3(width + 0.06f, height + 0.06f, 0.04f), WHITE);
	drawPanel(shader, world, pos + glm::vec3(0.0f, height / 2 + 0.03f, 0.021f), 0.0f, width, height, tex);
	drawPanel(shader, world, pos + glm::vec3(0.0f, height / 2 + 0.03f, -0.021f), 180.0f, width, height, tex);
	for (int side = -1; side <= 1; side += 2)
		drawCylinder(shader, world, pos + glm::vec3(side * (width / 2 - 0.15f), height + 0.06f, 0.0f), 0.008f,
					 STORE_HEIGHT - pos.y - height, DARK);
}

void drawAisle(ShaderProgram& shader, float x, float z, float length, CategoryId category, int seed)
{
	const Category& c = CATEGORIES[category];
	glm::mat4 aisle = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z))
					* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawCube(shader, aisle, glm::vec3(0.0f), glm::vec3(length, 0.12f, 1.0f), DARK);
	drawCube(shader, aisle, glm::vec3(0.0f, 0.12f, 0.0f), glm::vec3(length, 1.86f, 0.06f), SHELF_COLOR * 0.9f);
	drawCube(shader, aisle, glm::vec3(0.0f, 1.98f, 0.0f), glm::vec3(length, 0.08f, 0.14f), c.color);
	for (int end = -1; end <= 1; end += 2)
		drawCube(shader, aisle, glm::vec3(end * length / 2, 0.0f, 0.0f), glm::vec3(0.05f, 2.06f, 1.0f), METAL, T_WHITE, 48.0f);

	for (int side = 0; side < 2; side++)
	{
		glm::mat4 shelfFrame = aisle * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f * side), glm::vec3(0.0f, 1.0f, 0.0f))
							 * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.03f));
		drawShelf(shader, shelfFrame, length - 0.06f, 0.47f, category, seed * 2 + side);
	}

	glm::mat4 front = aisle * glm::translate(glm::mat4(1.0f), glm::vec3(-length / 2 - 0.03f, 0.0f, 0.0f))
					* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	drawPanel(shader, front, glm::vec3(0.0f, 1.55f, 0.002f), 0.0f, 0.5f, 0.75f, T_POSTER, 0.25f);
	drawCube(shader, front, glm::vec3(0.0f, 0.0f, 0.35f), glm::vec3(0.9f, 0.1f, 0.6f), WOOD, T_WOODCRATE);
	ProductKind promo = c.kinds[0];
	for (int layer = 0; layer < 2; layer++)
		for (int row = 0; row < 2; row++)
			drawProducts(shader, front, promo, 4, glm::vec3(0.0f, 0.1f + layer * PRODUCTS[promo].size.y, 0.2f + row * 0.25f),
						 c.palette[(seed + layer) % 4]);

	drawHangingSign(shader, glm::vec3(x, 2.9f, z + length / 2 - 0.8f), 2.0f, c.sign);
}

void drawWallShelf(ShaderProgram& shader, int side, CategoryId category, int seed)
{
	glm::mat4 frame = glm::translate(glm::mat4(1.0f), glm::vec3(side * STORE_X, 0.0f, WALL_SHELF_Z))
					* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f * side), glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube(shader, frame, glm::vec3(0.0f, 0.0f, 0.25f), glm::vec3(WALL_SHELF_LENGTH, 0.12f, 0.5f), DARK);
	drawCube(shader, frame, glm::vec3(0.0f, 0.0f, 0.02f), glm::vec3(WALL_SHELF_LENGTH, 2.0f, 0.04f), SHELF_COLOR * 0.9f);
	drawCube(shader, frame, glm::vec3(0.0f, 1.95f, 0.08f), glm::vec3(WALL_SHELF_LENGTH, 0.1f, 0.16f), CATEGORIES[category].color);
	for (int end = -1; end <= 1; end += 2)
		drawCube(shader, frame, glm::vec3(end * WALL_SHELF_LENGTH / 2, 0.0f, 0.25f), glm::vec3(0.05f, 2.05f, 0.5f), METAL, T_WHITE, 48.0f);
	drawShelf(shader, frame * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.04f)), WALL_SHELF_LENGTH - 0.06f, 0.44f, category, seed);
	drawPanel(shader, frame, glm::vec3(0.0f, 3.05f, 0.01f), 0.0f, 2.8f, 0.7f, CATEGORIES[category].sign);
}

void drawFridges(ShaderProgram& shader)
{
	const glm::vec3 COLD_LIGHT(0.55f, 0.72f, 0.85f);
	for (int m = 0; m < 4; m++)
	{
		glm::mat4 module = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f + 4.0f * m, 0.0f, STORE_BACK));
		drawCube(shader, module, glm::vec3(0.0f, 0.0f, 0.45f), glm::vec3(4.0f, 0.12f, 0.9f), DARK);
		drawCube(shader, module, glm::vec3(0.0f, 0.12f, 0.03f), glm::vec3(3.9f, 1.98f, 0.04f), WHITE, T_WHITE, 16.0f, COLD_LIGHT);
		drawCube(shader, module, glm::vec3(0.0f, 2.1f, 0.47f), glm::vec3(4.0f, 0.4f, 0.95f), WHITE, T_WHITE, 32.0f);
		drawCube(shader, module, glm::vec3(0.0f, 2.06f, 0.9f), glm::vec3(3.9f, 0.04f, 0.04f), WHITE, T_WHITE, 16.0f, glm::vec3(0.9f));
		for (int side = -1; side <= 1; side += 2)
			drawCube(shader, module, glm::vec3(side * 1.97f, 0.0f, 0.45f), glm::vec3(0.06f, 2.5f, 0.92f), WHITE, T_WHITE, 32.0f);
		drawShelf(shader, module * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.06f)), 3.88f, 0.8f, C_FRAIS, 50 + m);
		if (m % 2 == 1)
			drawPanel(shader, module, glm::vec3(0.0f, 2.3f, 0.952f), 0.0f, 1.4f, 0.35f, T_SIGN_FRAIS);
	}
}

struct Fruit
{
	glm::vec3 color;
	float height;
	bool big;
};
const int NUM_FRUITS = 8;
const Fruit FRUITS[NUM_FRUITS] = {
	{ glm::vec3(0.80f, 0.08f, 0.08f), 0.13f, false },
	{ glm::vec3(1.00f, 0.55f, 0.08f), 0.14f, false },
	{ glm::vec3(0.45f, 0.75f, 0.30f), 0.22f, true },
	{ glm::vec3(0.55f, 0.82f, 0.20f), 0.13f, false },
	{ glm::vec3(0.90f, 0.15f, 0.08f), 0.12f, false },
	{ glm::vec3(0.50f, 0.36f, 0.20f), 0.10f, false },
	{ glm::vec3(1.00f, 0.90f, 0.25f), 0.11f, false },
	{ glm::vec3(0.82f, 0.85f, 0.45f), 0.24f, true }
};

void drawFruitCrate(ShaderProgram& shader, const glm::mat4& crate, const Fruit& fruit)
{

	drawMesh(shader, M_WOODCRATE, T_WOODCRATE, crate * glm::scale(glm::mat4(1.0f), glm::vec3(0.45f, 0.09f, 0.27f)), WHITE, 8.0f);

	glm::mat4 pile = glm::translate(crate, glm::vec3(0.0f, 0.15f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.84f, fruit.height, 0.48f));
	drawMesh(shader, fruit.big ? M_FRUITS_BIG : M_FRUITS_SMALL, T_WHITE, pile, fruit.color, 64.0f, glm::vec3(0.0f), 0.5f);
}

void drawFruitStand(ShaderProgram& shader, glm::vec3 pos)
{
	for (int t = 0; t < 2; t++)
	{
		glm::mat4 table = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.0f, 3.0f * t));
		drawCube(shader, table, glm::vec3(0.0f), glm::vec3(4.0f, 0.1f, 1.3f), DARK);
		drawCube(shader, table, glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(3.9f, 0.6f, 1.2f), WOOD, T_WOODCRATE, 8.0f);
		drawCube(shader, table, glm::vec3(0.0f, 0.7f, -0.3f), glm::vec3(3.9f, 0.2f, 0.6f), WOOD * 0.85f, T_WOODCRATE, 8.0f);
		for (int row = 0; row < 2; row++)
			for (int i = 0; i < 4; i++)
			{
				glm::mat4 crate = table * glm::translate(glm::mat4(1.0f), glm::vec3(-1.45f + 0.97f * i, 0.76f + 0.2f * (1 - row), 0.3f - 0.6f * (1 - row)))
								* glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				int n = t * 8 + row * 4 + i;
				drawFruitCrate(shader, crate, FRUITS[(n * 3) % NUM_FRUITS]);
			}
	}
	drawHangingSign(shader, pos + glm::vec3(0.0f, 2.9f, 1.5f), 2.6f, T_SIGN_FRUITS);
}

void drawCashRegister(ShaderProgram& shader, glm::vec3 pos, bool open, int seed)
{
	glm::mat4 counter = glm::translate(glm::mat4(1.0f), pos);

	drawCube(shader, counter, glm::vec3(0.0f), glm::vec3(0.9f, 0.85f, 3.0f), glm::vec3(0.94f), T_WHITE, 32.0f);
	drawCube(shader, counter, glm::vec3(0.0f), glm::vec3(0.92f, 0.1f, 3.02f), DARK);
	drawCube(shader, counter, glm::vec3(0.0f, 0.55f, 0.0f), glm::vec3(0.92f, 0.12f, 3.02f), GREEN);
	drawCube(shader, counter, glm::vec3(0.0f, 0.85f, -0.5f), glm::vec3(0.6f, 0.04f, 1.8f), glm::vec3(0.1f), T_WHITE, 8.0f);
	for (int side = -1; side <= 1; side += 2)
		drawCube(shader, counter, glm::vec3(side * 0.32f, 0.85f, -0.5f), glm::vec3(0.04f, 0.07f, 1.8f), METAL, T_WHITE, 64.0f);
	drawCube(shader, counter, glm::vec3(0.0f, 0.85f, 0.65f), glm::vec3(0.4f, 0.01f, 0.3f), glm::vec3(0.15f), T_WHITE, 128.0f);
	drawCube(shader, counter, glm::vec3(0.0f, 0.86f, 0.65f), glm::vec3(0.3f, 0.005f, 0.02f), BLACK, T_WHITE, 16.0f,
			 open ? glm::vec3(1.0f, 0.1f, 0.05f) : glm::vec3(0.0f));

	drawCylinder(shader, counter, glm::vec3(-0.3f, 0.85f, 1.05f), 0.025f, 0.35f, DARK);
	drawMesh(shader, M_BOX, T_WHITE, glm::translate(counter, glm::vec3(-0.3f, 1.18f, 1.05f))
			 * glm::rotate(glm::mat4(1.0f), glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f))
			 * glm::scale(glm::mat4(1.0f), glm::vec3(0.38f, 0.26f, 0.04f)), glm::vec3(0.1f), 64.0f,
			 open ? glm::vec3(0.25f, 0.5f, 0.7f) : glm::vec3(0.0f));
	drawCube(shader, counter, glm::vec3(0.3f, 0.85f, 1.1f), glm::vec3(0.09f, 0.14f, 0.06f), DARK, T_WHITE, 64.0f);

	drawCylinder(shader, counter, glm::vec3(0.35f, 0.85f, -1.4f), 0.02f, 1.1f, METAL);
	drawCube(shader, counter, glm::vec3(0.35f, 1.95f, -1.4f), glm::vec3(0.16f, 0.16f, 0.16f), open ? glm::vec3(0.2f, 0.9f, 0.4f) : glm::vec3(0.5f),
			 T_WHITE, 32.0f, open ? glm::vec3(0.1f, 0.7f, 0.25f) : glm::vec3(0.0f));

	if (open)
	{

		glm::mat4 robot = glm::translate(counter, glm::vec3(-0.95f, 0.0f, 0.55f))
						* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(0.265f));
		drawMesh(shader, M_ROBOT, T_ROBOT, robot, WHITE, 64.0f, glm::vec3(0.0f), 0.6f);

		for (int i = 0; i < 4; i++)
		{
			int r = seed * 50 + i;
			const Category& c = CATEGORIES[(int)(random01(r) * NUM_CATEGORIES)];
			drawProduct(shader, counter, c.kinds[i % 3], glm::vec3(0.0f, 0.89f, -1.2f + 0.38f * i), c.palette[i % 4]);
		}
	}
}

void drawCart(ShaderProgram& shader, glm::vec3 pos, float rotY)
{
	glm::mat4 cart = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::vec3 CHROME(0.75f, 0.77f, 0.80f);
	drawCube(shader, cart, glm::vec3(0.0f, 0.45f, 0.0f), glm::vec3(0.52f, 0.02f, 0.85f), CHROME, T_WHITE, 96.0f);
	for (int side = -1; side <= 1; side += 2)
	{
		drawCube(shader, cart, glm::vec3(side * 0.26f, 0.45f, 0.0f), glm::vec3(0.015f, 0.42f, 0.85f), CHROME, T_WHITE, 96.0f);
		drawCube(shader, cart, glm::vec3(0.0f, 0.45f, side * 0.425f), glm::vec3(0.52f, 0.42f, 0.015f), CHROME, T_WHITE, 96.0f);
		drawCube(shader, cart, glm::vec3(side * 0.22f, 0.07f, 0.45f), glm::vec3(0.025f, 0.88f, 0.025f), CHROME, T_WHITE, 96.0f);
		for (int end = -1; end <= 1; end += 2)
		{
			if (end < 0)
				drawCube(shader, cart, glm::vec3(side * 0.22f, 0.07f, -0.35f), glm::vec3(0.025f, 0.38f, 0.025f), CHROME, T_WHITE, 96.0f);
			drawMesh(shader, M_TUBE, T_WHITE, glm::translate(cart, glm::vec3(side * 0.22f - 0.015f, 0.04f, end * 0.37f))
					 * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
					 * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.03f, 0.08f)), glm::vec3(0.1f));
		}
	}
	drawMesh(shader, M_TUBE, T_WHITE, glm::translate(cart, glm::vec3(0.3f, 0.93f, 0.45f))
			 * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
			 * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.6f, 0.04f)), glm::vec3(0.85f, 0.15f, 0.1f), 64.0f);
}

void drawBackWall(ShaderProgram& shader)
{
	glm::mat4 world(1.0f);
	float wallH = STORE_HEIGHT + 0.3f;
	float z = STORE_BACK - WALL / 2;
	float doorX = (RES_DOOR_X0 + RES_DOOR_X1) / 2;
	float leftW = RES_DOOR_X0 + STORE_X, rightW = STORE_X - RES_DOOR_X1;
	drawCube(shader, world, glm::vec3(-STORE_X + leftW / 2, 0.0f, z), glm::vec3(leftW, wallH, WALL), WALL_COLOR);
	drawCube(shader, world, glm::vec3(RES_DOOR_X1 + rightW / 2, 0.0f, z), glm::vec3(rightW, wallH, WALL), WALL_COLOR);
	drawCube(shader, world, glm::vec3(doorX, RES_DOOR_H, z), glm::vec3(RES_DOOR_X1 - RES_DOOR_X0, wallH - RES_DOOR_H, WALL), WALL_COLOR);

	for (int side = -1; side <= 1; side += 2)
		drawCube(shader, world, glm::vec3(doorX + side * 0.65f, 0.0f, z), glm::vec3(0.12f, RES_DOOR_H + 0.07f, WALL + 0.04f), DARK, T_WHITE, 48.0f);
	drawCube(shader, world, glm::vec3(doorX, RES_DOOR_H - 0.01f, z), glm::vec3(1.42f, 0.08f, WALL + 0.04f), DARK, T_WHITE, 48.0f);
	drawCube(shader, world, glm::vec3(doorX, 0.0f, z), glm::vec3(1.2f, 0.01f, WALL), METAL, T_WHITE, 64.0f);

	drawPanel(shader, world, glm::vec3(doorX, 2.95f, STORE_BACK + 0.012f), 0.0f, 1.2f, 0.3f, T_SIGN_RESERVE, 0.2f);
	drawPanel(shader, world, glm::vec3(doorX, 2.65f, RES_FRONT - 0.012f), 180.0f, 0.8f, 0.2f, T_SIGN_EXIT, 0.9f);
}

void drawStructure(ShaderProgram& shader)
{
	glm::mat4 world(1.0f);
	glm::mat4 lying = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	shader.setUniform("worldUV", 1);
	shader.setUniform("uvScale", 1.0f / 2.4f);
	drawMesh(shader, M_QUAD, T_FLOOR, glm::translate(world, glm::vec3(0.0f, 0.0f, -2.0f)) * lying
			 * glm::scale(glm::mat4(1.0f), glm::vec3(2 * STORE_X, 24.0f, 1.0f)), WHITE, 96.0f, glm::vec3(0.0f), 0.6f);
	drawMesh(shader, M_QUAD, T_FLOOR, glm::translate(world, glm::vec3(0.0f, 0.003f, 11.8f)) * lying
			 * glm::scale(glm::mat4(1.0f), glm::vec3(32.0f, 3.0f, 1.0f)), glm::vec3(0.7f), 16.0f);

	drawCube(shader, world, glm::vec3(0.0f, STORE_HEIGHT, -2.0f), glm::vec3(2 * STORE_X, 0.3f, 24.0f), WHITE, T_CEILING, 8.0f, glm::vec3(0.35f));
	shader.setUniform("uvScale", 1.0f / 8.0f);
	drawMesh(shader, M_QUAD, T_ASPHALT, glm::translate(world, glm::vec3(0.0f, -0.03f, 0.0f)) * lying
			 * glm::scale(glm::mat4(1.0f), glm::vec3(160.0f, 160.0f, 1.0f)), glm::vec3(0.8f), 8.0f);
	shader.setUniform("worldUV", 0);

	for (int i = -5; i <= 5; i++)
		drawCube(shader, world, glm::vec3(i * 2.8f, -0.02f, 17.5f), glm::vec3(0.12f, 0.03f, 4.5f), glm::vec3(0.9f));

	float wallH = STORE_HEIGHT + 0.3f;
	drawCube(shader, world, glm::vec3(-STORE_X - WALL / 2, 0.0f, -2.0f), glm::vec3(WALL, wallH, 24.0f + 2 * WALL), WALL_COLOR);
	drawCube(shader, world, glm::vec3( STORE_X + WALL / 2, 0.0f, -2.0f), glm::vec3(WALL, wallH, 24.0f + 2 * WALL), WALL_COLOR);

	float frontH = 5.6f;
	float pieceW = STORE_X + WALL - DOOR_HALF;
	float zf = STORE_FRONT + WALL / 2;
	for (int side = -1; side <= 1; side += 2)
	{
		float xc = side * (DOOR_HALF + pieceW / 2);
		drawCube(shader, world, glm::vec3(xc, 0.0f, zf - WALL / 2), glm::vec3(pieceW, frontH, WALL), WALL_COLOR);

		drawCube(shader, world, glm::vec3(xc, 0.5f, zf + 0.01f), glm::vec3(pieceW - 1.6f, 2.2f, WALL), glm::vec3(0.25f, 0.35f, 0.42f), T_WHITE, 128.0f);
		drawCube(shader, world, glm::vec3(xc, 0.5f, zf - 0.01f), glm::vec3(pieceW - 1.6f, 2.2f, WALL), glm::vec3(0.3f), T_WHITE, 128.0f,
				 glm::vec3(0.55f, 0.68f, 0.78f));
		drawCube(shader, world, glm::vec3(side * (DOOR_HALF + 0.08f), 0.0f, zf), glm::vec3(0.16f, DOOR_HEIGHT, WALL + 0.06f), DARK, T_WHITE, 48.0f);
	}
	drawCube(shader, world, glm::vec3(0.0f, DOOR_HEIGHT, zf), glm::vec3(2 * DOOR_HALF, frontH - DOOR_HEIGHT, WALL), WALL_COLOR);
	drawCube(shader, world, glm::vec3(0.0f, DOOR_HEIGHT - 0.16f, zf), glm::vec3(2 * DOOR_HALF + 0.32f, 0.16f, WALL + 0.06f), DARK, T_WHITE, 48.0f);
	drawCube(shader, world, glm::vec3(0.0f, frontH - 0.5f, zf + 0.02f), glm::vec3(2 * (STORE_X + WALL), 0.5f, WALL), GREEN);
	drawCube(shader, world, glm::vec3(0.0f, frontH - 0.6f, zf + 0.03f), glm::vec3(2 * (STORE_X + WALL), 0.1f, WALL), ORANGE);
	drawPanel(shader, world, glm::vec3(0.0f, 3.9f, STORE_FRONT + WALL + 0.01f), 0.0f, 7.0f, 7.0f * 192.0f / 1024.0f, T_SIGN_TITLE, 0.35f);

	for (int side = -1; side <= 1; side += 2)
	{
		drawCube(shader, world, glm::vec3(side * (STORE_X - 0.01f), 0.0f, -2.0f), glm::vec3(0.04f, 0.12f, 24.0f), DARK);
		drawCube(shader, world, glm::vec3(side * (STORE_X - 0.01f), 2.45f, -2.0f), glm::vec3(0.04f, 0.2f, 24.0f), GREEN);
	}
	drawCube(shader, world, glm::vec3(0.0f, 2.45f, STORE_BACK + 0.01f), glm::vec3(2 * STORE_X, 0.2f, 0.04f), GREEN);
	drawPanel(shader, world, glm::vec3(0.0f, 3.45f, STORE_BACK + 0.02f), 0.0f, 7.5f, 7.5f * 192.0f / 1024.0f, T_SIGN_TITLE, 0.1f);

	drawCube(shader, world, glm::vec3(0.0f, 0.0f, 8.9f), glm::vec3(4.0f, 0.012f, 1.8f), glm::vec3(0.18f, 0.2f, 0.22f), T_WHITE, 4.0f);
}

void drawPillars(ShaderProgram& shader)
{
	glm::mat4 world(1.0f);
	for (int side = -1; side <= 1; side += 2)
	{
		glm::mat4 pillar = glm::translate(world, glm::vec3(side * PILLAR_X, 0.0f, PILLAR_Z));
		drawCube(shader, pillar, glm::vec3(0.0f), glm::vec3(0.5f, STORE_HEIGHT, 0.5f), WALL_COLOR);
		drawCube(shader, pillar, glm::vec3(0.0f), glm::vec3(0.52f, 0.12f, 0.52f), DARK);
		drawCube(shader, pillar, glm::vec3(0.0f, 2.45f, 0.0f), glm::vec3(0.52f, 0.2f, 0.52f), GREEN);
		drawPanel(shader, pillar, glm::vec3(0.0f, 1.5f, 0.252f), 0.0f, 0.42f, 0.63f, T_POSTER, 0.25f);
		drawPanel(shader, pillar, glm::vec3(0.0f, 1.5f, -0.252f), 180.0f, 0.42f, 0.63f, T_POSTER, 0.25f);
	}
}

void drawSupermarket(ShaderProgram& shader, bool building = true)
{
	const CategoryId aisleCategory[NUM_AISLES] = { C_EPICERIE, C_PETITDEJ, C_SNACKS, C_BOISSONS };
	for (int i = 0; i < NUM_AISLES; i++)
		drawAisle(shader, AISLE_X[i], AISLE_Z, AISLE_LENGTH, aisleCategory[i], i + 1);
	drawWallShelf(shader, -1, C_HYGIENE, 7);
	drawWallShelf(shader, 1, C_BOISSONS, 8);
	drawFridges(shader);

	drawFruitStand(shader, FRUIT_STAND_POS);

	drawCashRegister(shader, glm::vec3(CHECKOUT_X[0], 0.0f, CHECKOUT_Z), true, 1);
	drawCashRegister(shader, glm::vec3(CHECKOUT_X[1], 0.0f, CHECKOUT_Z), false, 2);
	drawHangingSign(shader, glm::vec3((CHECKOUT_X[0] + CHECKOUT_X[1]) / 2, 2.9f, CHECKOUT_Z + 1.0f), 2.2f, T_SIGN_CAISSES);

	for (int i = 0; i < 3; i++)
		drawCart(shader, glm::vec3(4.0f, 0.0f, 7.9f + 0.3f * i), 0.0f);
	drawCart(shader, glm::vec3(-4.6f, 0.0f, -3.5f), 160.0f);
	drawPillars(shader);

	if (building)
		drawStructure(shader);
}

float gDoorAngle = RES_DOOR_AJAR;

glm::mat4 doorMatrix()
{
	return glm::translate(glm::mat4(1.0f), glm::vec3(RES_DOOR_X0, 0.0f, RES_FRONT))
		 * glm::rotate(glm::mat4(1.0f), glm::radians(gDoorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
}

void drawReserveDoor(ShaderProgram& shader)
{
	glm::mat4 door = doorMatrix();
	float width = RES_DOOR_X1 - RES_DOOR_X0;
	drawCube(shader, door, glm::vec3(width / 2, 0.0f, 0.0f), glm::vec3(width - 0.02f, RES_DOOR_H - 0.02f, 0.045f),
			 glm::vec3(0.50f, 0.53f, 0.57f), T_WHITE, 48.0f);
	for (int face = -1; face <= 1; face += 2)
	{
		drawCube(shader, door, glm::vec3(width - 0.2f, 0.95f, face * 0.025f), glm::vec3(0.12f, 0.35f, 0.01f), METAL, T_WHITE, 96.0f);
		drawCube(shader, door, glm::vec3(width / 2, 0.0f, face * 0.025f), glm::vec3(width - 0.04f, 0.25f, 0.01f), METAL * 0.8f, T_WHITE, 96.0f);
	}
}

void drawReserveRoom(ShaderProgram& shader)
{
	glm::mat4 world(1.0f);
	glm::mat4 lying = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	float w = STORE_X - RES_X0, d = RES_FRONT - RES_BACK;
	glm::vec3 c((RES_X0 + STORE_X) / 2, 0.0f, (RES_FRONT + RES_BACK) / 2);

	shader.setUniform("worldUV", 1);
	shader.setUniform("uvScale", 1.0f / 4.0f);
	drawMesh(shader, M_QUAD, T_CONCRETE, glm::translate(world, c) * lying * glm::scale(glm::mat4(1.0f), glm::vec3(w, d, 1.0f)),
			 WHITE, 24.0f, glm::vec3(0.0f), 0.25f);
	shader.setUniform("worldUV", 0);

	const glm::vec3 BLOCK(0.60f, 0.61f, 0.60f);
	float h = RES_HEIGHT + 0.3f;
	drawCube(shader, world, glm::vec3(RES_X0 - WALL / 2, 0.0f, c.z), glm::vec3(WALL, h, d + 2 * WALL), BLOCK);
	drawCube(shader, world, glm::vec3(STORE_X + WALL / 2, 0.0f, c.z), glm::vec3(WALL, h, d + 2 * WALL), BLOCK);
	drawCube(shader, world, glm::vec3(c.x, 0.0f, RES_BACK - WALL / 2), glm::vec3(w, h, WALL), BLOCK);
	drawCube(shader, world, glm::vec3(c.x, RES_HEIGHT, c.z), glm::vec3(w, 0.3f, d), glm::vec3(0.4f), T_WHITE, 8.0f);
	drawCube(shader, world, glm::vec3(7.25f, 0.0f, RES_BACK + 1.3f), glm::vec3(8.1f, 0.004f, 0.08f), glm::vec3(0.95f, 0.75f, 0.1f));
	drawCube(shader, world, glm::vec3(6.5f, RES_HEIGHT - 0.06f, -17.5f), glm::vec3(1.4f, 0.06f, 0.16f), METAL, T_WHITE, 64.0f);

	drawPanel(shader, world, glm::vec3(STORE_X - 0.012f, 1.3f, -18.1f), -90.0f, 0.5f, 0.625f, T_EMPLOYEE, 0.0f);
}

void drawMetalShelf(ShaderProgram& shader)
{
	glm::mat4 frame = glm::translate(glm::mat4(1.0f), glm::vec3(RES_X0, 0.0f, SHELF_TOP_POS.z))
					* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::vec3 GREY(0.45f, 0.47f, 0.5f);
	const float LEVELS[4] = { 0.1f, 0.7f, 1.3f, SHELF_TOP_POS.y - 0.03f };
	for (int i = -1; i <= 1; i += 2)
		for (int k = 0; k < 2; k++)
			drawCube(shader, frame, glm::vec3(i * 2.08f, 0.0f, 0.04f + 0.52f * k), glm::vec3(0.04f, SHELF_TOP_POS.y, 0.04f), GREY, T_WHITE, 48.0f);
	for (int l = 0; l < 4; l++)
		drawCube(shader, frame, glm::vec3(0.0f, LEVELS[l], 0.3f), glm::vec3(4.2f, 0.03f, 0.6f), GREY, T_WHITE, 48.0f);

	struct Item { float x; int level; float w, h; bool bag; };
	const Item ITEMS[] = {
		{ -1.6f, 0, 0.32f, 0.46f, true }, { -1.2f, 0, 0.32f, 0.46f, true }, { -0.8f, 0, 0.32f, 0.46f, true },
		{ 0.3f, 0, 0.5f, 0.4f, false }, { 1.2f, 0, 0.6f, 0.45f, false },
		{ -1.4f, 1, 0.45f, 0.35f, false }, { -0.7f, 1, 0.5f, 0.42f, false }, { 0.5f, 1, 0.45f, 0.3f, false }, { 1.4f, 1, 0.5f, 0.45f, false },
		{ -1.5f, 2, 0.5f, 0.4f, false }, { 0.6f, 2, 0.32f, 0.46f, true }, { 1.0f, 2, 0.32f, 0.46f, true }, { 1.6f, 2, 0.4f, 0.3f, false },
		{ -1.7f, 3, 0.45f, 0.35f, false }
	};
	for (int i = 0; i < (int)(sizeof(ITEMS) / sizeof(ITEMS[0])); i++)
	{
		const Item& it = ITEMS[i];
		glm::vec3 pos(it.x, LEVELS[it.level] + 0.03f, it.bag ? 0.4f : 0.3f);
		glm::vec3 size(it.w, it.h, it.bag ? 0.14f : 0.48f);
		drawCube(shader, frame, pos, size, WHITE, it.bag ? T_CROQUETTES : T_CARDBOARD, 8.0f);
	}
}

void drawPallet(ShaderProgram& shader, glm::vec3 pos, MeshId load, float loadHeight)
{
	drawCube(shader, glm::mat4(1.0f), pos, glm::vec3(1.1f, 0.14f, 1.0f), WOOD, T_WOODCRATE, 8.0f);
	drawMesh(shader, load, T_CARDBOARD, glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.14f, 0.0f))
			 * glm::scale(glm::mat4(1.0f), glm::vec3(1.05f, loadHeight, 0.95f)), WHITE, 8.0f);
}

void drawPalletRack(ShaderProgram& shader)
{
	glm::mat4 world(1.0f);
	const glm::vec3 RACK_BLUE(0.12f, 0.28f, 0.60f), RACK_ORANGE(0.95f, 0.42f, 0.08f);
	const float X0 = 3.2f, BAY = 2.7f, LEVEL_H = 1.25f;
	float zFront = RES_BACK + 1.05f, zBack = RES_BACK + 0.08f, zMid = RES_BACK + 0.56f;
	for (int i = 0; i <= 3; i++)
	{
		drawCube(shader, world, glm::vec3(X0 + BAY * i, 0.0f, zFront), glm::vec3(0.08f, 3.3f, 0.08f), RACK_BLUE, T_WHITE, 32.0f);
		drawCube(shader, world, glm::vec3(X0 + BAY * i, 0.0f, zBack), glm::vec3(0.08f, 3.3f, 0.08f), RACK_BLUE, T_WHITE, 32.0f);
	}
	for (int b = 0; b < 3; b++)
	{
		float xc = X0 + BAY * (b + 0.5f);
		for (int l = 1; l <= 2; l++)
		{
			drawCube(shader, world, glm::vec3(xc, l * LEVEL_H - 0.12f, zFront), glm::vec3(BAY - 0.08f, 0.12f, 0.06f), RACK_ORANGE, T_WHITE, 32.0f);
			drawCube(shader, world, glm::vec3(xc, l * LEVEL_H - 0.12f, zBack), glm::vec3(BAY - 0.08f, 0.12f, 0.06f), RACK_ORANGE, T_WHITE, 32.0f);
		}
		for (int l = 0; l < 3; l++)
			for (int k = 0; k < 2; k++)
			{
				int n = b * 6 + l * 2 + k;
				if (random01(n + 300) < 0.2f)
					continue;
				drawPallet(shader, glm::vec3(xc + (k - 0.5f) * 1.3f, l * LEVEL_H, zMid),
						   random01(n + 400) < 0.5f ? M_CARTONS_A : M_CARTONS_B, l == 2 ? 0.95f : 0.85f);
			}
	}
}

const float GRAVITY = 9.81f;

struct FallingObject
{
	MeshId mesh;
	TexId tex;
	glm::vec3 color;
	glm::vec3 size;
	glm::vec3 shelfPos;
	glm::vec3 pushVelocity;
	float pushSpin;
	float restitution;
	float pushTime;

	int state;
	glm::vec3 pos;
	glm::vec3 vel;
	float angle, spin;
};
const int NUM_FALLING = 3;
FallingObject gFalling[NUM_FALLING] = {
	{ M_BOX,  T_FRAGILE,   WHITE,                        glm::vec3(0.30f, 0.28f, 0.34f), glm::vec3(2.42f, SHELF_TOP_POS.y, -16.8f),
	  glm::vec3(1.1f, 0.6f, 0.15f), -5.0f, 0.3f, 0.35f },
	{ M_TUBE, T_LABEL_CAN, glm::vec3(0.85f, 0.2f, 0.15f), glm::vec3(0.09f, 0.13f, 0.09f), glm::vec3(2.5f, SHELF_TOP_POS.y, -17.9f),
	  glm::vec3(1.4f, 0.9f, -0.2f), -9.0f, 0.45f, 0.8f },
	{ M_TUBE, T_LABEL_CAN, glm::vec3(0.2f, 0.5f, 0.85f),  glm::vec3(0.09f, 0.13f, 0.09f), glm::vec3(2.45f, SHELF_TOP_POS.y, -18.1f),
	  glm::vec3(1.2f, 1.1f, 0.1f), -7.0f, 0.45f, 0.95f }
};
float gEventStart = -1.0f;

float halfHeight(const FallingObject& o)
{
	return fabsf(cosf(o.angle)) * o.size.y / 2 + fabsf(sinf(o.angle)) * o.size.x / 2;
}

void resetFallingObjects()
{
	for (int i = 0; i < NUM_FALLING; i++)
	{
		FallingObject& o = gFalling[i];
		o.state = 0;
		o.pos = o.shelfPos + glm::vec3(0.0f, o.size.y / 2, 0.0f);
		o.vel = glm::vec3(0.0f);
		o.angle = o.spin = 0.0f;
	}
	gEventStart = -1.0f;
}

void updateFallingObjects(float dt)
{
	const float STEP = 1.0f / 240.0f;
	const float HALF_PI = glm::half_pi<float>();
	for (int i = 0; i < NUM_FALLING; i++)
	{
		FallingObject& o = gFalling[i];
		if (o.state == 0 && gEventStart >= 0.0f && gTime - gEventStart >= o.pushTime)
		{
			o.state = 1;
			o.vel = o.pushVelocity;
			o.spin = o.pushSpin;
		}
		if (o.state == 1)
		{
			for (float t = 0.0f; t < dt; t += STEP)
			{
				float h = glm::min(STEP, dt - t);
				o.vel.y -= GRAVITY * h;
				o.pos += o.vel * h;
				o.angle += o.spin * h;
				if (o.pos.y < halfHeight(o))
				{
					o.pos.y = halfHeight(o);
					if (o.vel.y < 0.0f)
					{
						o.vel.y = -o.vel.y * o.restitution;
						o.vel.x *= 0.6f;
						o.vel.z *= 0.6f;
						o.spin *= 0.5f;
						if (o.vel.y < 0.4f)
							o.state = 2;
					}
				}
			}
		}
		else if (o.state == 2)
		{

			float k = 1.0f - expf(-dt * 8.0f);
			o.angle += (roundf(o.angle / HALF_PI) * HALF_PI - o.angle) * k;
			o.pos += o.vel * dt;
			o.vel *= expf(-dt * 6.0f);
			o.vel.y = 0.0f;
			o.pos.y = halfHeight(o);
		}
	}
}

void drawFallingObjects(ShaderProgram& shader)
{
	for (int i = 0; i < NUM_FALLING; i++)
	{
		const FallingObject& o = gFalling[i];
		glm::mat4 model = glm::translate(glm::mat4(1.0f), o.pos)
						* glm::rotate(glm::mat4(1.0f), o.angle, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -o.size.y / 2, 0.0f))
						* glm::scale(glm::mat4(1.0f), o.size);
		drawMesh(shader, o.mesh, o.tex, model, o.color, o.mesh == M_TUBE ? 96.0f : 8.0f, glm::vec3(0.0f), 0.4f);
	}
}

void drawReserve(ShaderProgram& shader)
{
	drawMetalShelf(shader);
	drawPalletRack(shader);
	drawPallet(shader, glm::vec3(5.4f, 0.0f, -18.2f), M_CARTONS_A, 1.25f);
	for (int i = 0; i < 3; i++)
		drawMesh(shader, M_WOODCRATE, T_WOODCRATE, glm::translate(glm::mat4(1.0f), glm::vec3(11.3f, 0.6f * i, -15.3f))
				 * glm::rotate(glm::mat4(1.0f), glm::radians(8.0f * (i % 2) - 4.0f), glm::vec3(0.0f, 1.0f, 0.0f))
				 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)), WHITE, 8.0f);
	drawFallingObjects(shader);
	drawReserveDoor(shader);
	drawReserveRoom(shader);
}

enum CatState { CAT_ON_SHELF, CAT_PUSHING, CAT_JUMPING, CAT_RUNNING, CAT_WALKING };

struct Cat
{
	CatState state;
	glm::vec3 pos;
	glm::vec3 vel;
	float heading;
	float headYaw;
	float stride;
	float stateTime;
	float dazzled;
	float eyeGlow;
	int waypoint;
};
Cat gCat;

const int NUM_WAYPOINTS = 7;
const int LOOP_START = 3;
const glm::vec2 CAT_PATH[NUM_WAYPOINTS] = {
	glm::vec2(6.4f, -16.0f), glm::vec2(8.5f, -15.0f), glm::vec2(8.5f, -13.4f),
	glm::vec2(4.5f, -12.3f), glm::vec2(4.5f, 0.9f), glm::vec2(0.0f, 0.9f), glm::vec2(0.0f, -12.3f)
};
const glm::vec3 CAT_LANDING(3.5f, 0.0f, -17.2f);
const glm::vec3 GINGER(0.95f, 0.55f, 0.22f);

void resetCat()
{
	gCat.state = CAT_ON_SHELF;
	gCat.pos = SHELF_TOP_POS;
	gCat.vel = glm::vec3(0.0f);
	gCat.heading = glm::half_pi<float>();
	gCat.headYaw = 0.0f;
	gCat.stride = gCat.stateTime = gCat.dazzled = 0.0f;
	gCat.eyeGlow = 0.0f;
	gCat.waypoint = 0;
}

void startCatEvent()
{
	if (gCat.state != CAT_ON_SHELF)
		return;
	gCat.state = CAT_PUSHING;
	gCat.stateTime = 0.0f;
	gEventStart = gTime;
}

float wrapAngle(float a)
{
	return atan2f(sinf(a), cosf(a));
}

void drawCat(ShaderProgram& shader)
{
	const float LEG = 0.22f;
	glm::mat4 cat = glm::translate(glm::mat4(1.0f), gCat.pos) * glm::rotate(glm::mat4(1.0f), gCat.heading, glm::vec3(0.0f, 1.0f, 0.0f));
	bool sitting = gCat.state == CAT_ON_SHELF || gCat.state == CAT_PUSHING;
	bool moving = gCat.state == CAT_RUNNING || gCat.state == CAT_WALKING;

	glm::mat4 pose(1.0f);
	if (sitting)
		pose = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, -0.15f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f))
			 * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.2f));
	else if (moving)
		pose = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.01f * fabsf(sinf(gCat.stride)), 0.0f));
	else
		pose = glm::rotate(glm::mat4(1.0f), glm::clamp(-gCat.vel.y * 0.15f, -0.6f, 0.4f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 body = cat * pose;
	drawMesh(shader, M_CAT_BODY, T_CAT_FUR, body, GINGER, 8.0f, glm::vec3(0.0f), 0.1f);

	glm::mat4 head = body * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.33f, 0.19f))
				   * glm::rotate(glm::mat4(1.0f), glm::radians(sitting ? 35.0f : 0.0f), glm::vec3(1.0f, 0.0f, 0.0f))
				   * glm::rotate(glm::mat4(1.0f), gCat.headYaw, glm::vec3(0.0f, 1.0f, 0.0f));
	drawMesh(shader, M_CAT_HEAD, T_CAT_FUR, head, GINGER, 8.0f, glm::vec3(0.0f), 0.1f);
	drawMesh(shader, M_CAT_EYES, T_WHITE, head, glm::vec3(0.6f, 0.8f, 0.2f), 128.0f, glm::vec3(0.75f, 1.0f, 0.3f) * gCat.eyeGlow, 1.0f);

	float sway = sinf(gTime * (moving ? 6.0f : 1.5f)) * (moving ? 0.25f : 0.45f);
	drawMesh(shader, M_CAT_TAIL, T_CAT_FUR, body * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.26f, -0.21f))
			 * glm::rotate(glm::mat4(1.0f), sway, glm::vec3(0.0f, 1.0f, 0.0f)), GINGER, 8.0f, glm::vec3(0.0f), 0.1f);

	const glm::vec3 HIPS[4] = { glm::vec3(-0.055f, 0.22f, 0.13f), glm::vec3(0.055f, 0.22f, 0.13f),
								glm::vec3(-0.06f, 0.22f, -0.14f), glm::vec3(0.06f, 0.22f, -0.14f) };
	for (int i = 0; i < 4; i++)
	{
		bool front = i < 2;
		glm::mat4 leg;
		if (sitting && front)
		{

			glm::vec3 hip = glm::vec3(pose * glm::vec4(HIPS[i], 1.0f));
			float swipe = 0.0f;
			if (gCat.state == CAT_PUSHING && i == 1)
				swipe = glm::radians(-70.0f) * fabsf(sinf(gCat.stateTime * 6.0f));
			leg = cat * glm::translate(glm::mat4(1.0f), hip) * glm::rotate(glm::mat4(1.0f), swipe, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, hip.y / LEG, 1.0f));
		}
		else if (sitting)
			leg = body * glm::translate(glm::mat4(1.0f), HIPS[i]) * glm::rotate(glm::mat4(1.0f), glm::radians(80.0f), glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.6f, 1.0f));
		else
		{
			float swing = moving ? 0.5f * sinf(gCat.stride + ((i == 0 || i == 3) ? 0.0f : glm::pi<float>())) : (front ? -0.6f : 0.6f);
			leg = body * glm::translate(glm::mat4(1.0f), HIPS[i]) * glm::rotate(glm::mat4(1.0f), swing, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		drawMesh(shader, M_TUBE, T_WHITE, leg * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -LEG, 0.0f))
				 * glm::scale(glm::mat4(1.0f), glm::vec3(0.045f, LEG, 0.045f)), GINGER, 8.0f, glm::vec3(0.0f), 0.1f);
	}
}

void drawShadows(ShaderProgram& shader)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glDepthMask(GL_FALSE);

	glm::mat4 lying = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	for (int i = 0; i < gNumObstacles; i++)
	{
		const Obstacle& o = gObstacles[i];
		if (!o.shadow)
			continue;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(o.x, 0.01f, o.z)) * lying
						* glm::scale(glm::mat4(1.0f), glm::vec3(o.w * 1.3f + 0.6f, o.d * 1.3f + 0.6f, 1.0f));
		drawMesh(shader, M_QUAD, T_SHADOW, model, BLACK, 1.0f, glm::vec3(1.0f), 0.0f);
	}

	if (gCat.pos.y < 0.01f && gCat.pos.z > STORE_BACK)
		drawMesh(shader, M_QUAD, T_SHADOW, glm::translate(glm::mat4(1.0f), gCat.pos + glm::vec3(0.0f, 0.012f, 0.0f))
				 * glm::rotate(glm::mat4(1.0f), gCat.heading, glm::vec3(0.0f, 1.0f, 0.0f)) * lying
				 * glm::scale(glm::mat4(1.0f), glm::vec3(0.45f, 0.75f, 1.0f)), BLACK, 1.0f, glm::vec3(1.0f), 0.0f);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

const int NUM_POINT_LIGHTS = 6;
const glm::vec3 pointLightPos[NUM_POINT_LIGHTS] = {
	glm::vec3(-4.5f, 4.1f, -10.0f), glm::vec3(4.5f, 4.1f, -10.0f),
	glm::vec3(-4.5f, 4.1f, -3.5f),  glm::vec3(4.5f, 4.1f, -3.5f),
	glm::vec3(-4.5f, 4.1f, 4.5f),   glm::vec3(4.5f, 4.1f, 4.5f)
};
const glm::vec3 CEILING_LIGHT(1.0f, 0.97f, 0.9f);

const glm::vec3 SUN_DIR = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.6f));

const glm::vec3 DOOR_LIGHT_POS(6.4f, 2.7f, -11.6f);
const glm::vec3 DOOR_LIGHT_TARGET(10.0f, 0.6f, -17.0f);
const glm::vec3 DOOR_LIGHT_COLOR(1.0f, 0.92f, 0.78f);

const glm::vec3 TUBE_POS(6.5f, RES_HEIGHT - 0.12f, -17.5f);
const glm::vec3 TUBE_COLOR(0.75f, 0.85f, 1.0f);

const float FLASH_INNER = 11.0f, FLASH_OUTER = 21.0f;
glm::vec3 gFlashPos(0.0f), gFlashDir(0.0f, 0.0f, -1.0f);

float tubeLevel(float t)
{
	float c = fmodf(t, 6.5f);
	if (c < 0.9f)
		return random01((int)(t * 18.0f)) > 0.45f ? 1.0f : 0.05f;
	return c < 2.2f ? 1.0f : 0.0f;
}

float flashlightHit(glm::vec3 p)
{
	glm::vec3 d = p - gFlashPos;
	float dist = glm::length(d);
	if (!gFlashlightOn || dist > 12.0f)
		return 0.0f;
	return glm::smoothstep(cosf(glm::radians(FLASH_OUTER)), cosf(glm::radians(FLASH_INNER)), glm::dot(d / dist, gFlashDir));
}

void setLightUniforms(ShaderProgram& shader)
{

	shader.setUniform("sunLight.direction", SUN_DIR);
	shader.setUniform("sunLight.ambient", glm::vec3(0.25f, 0.25f, 0.27f));
	shader.setUniform("sunLight.diffuse", glm::vec3(0.42f, 0.4f, 0.37f));
	shader.setUniform("sunLight.specular", glm::vec3(0.2f));

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		std::ostringstream base;
		base << "pointLights[" << i << "].";
		shader.setUniform((base.str() + "ambient").c_str(), glm::vec3(0.0f));
		shader.setUniform((base.str() + "diffuse").c_str(), CEILING_LIGHT * 0.32f);
		shader.setUniform((base.str() + "specular").c_str(), CEILING_LIGHT * 0.5f);
		shader.setUniform((base.str() + "position").c_str(), pointLightPos[i]);
		shader.setUniform((base.str() + "constant").c_str(), 1.0f);
		shader.setUniform((base.str() + "linear").c_str(), 0.045f);
		shader.setUniform((base.str() + "exponent").c_str(), 0.0075f);
	}

	shader.setUniform("spotLight.ambient", glm::vec3(0.0f));
	shader.setUniform("spotLight.diffuse", glm::vec3(1.25f, 1.18f, 1.05f));
	shader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setUniform("spotLight.position", gFlashPos);
	shader.setUniform("spotLight.direction", gFlashDir);
	shader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(FLASH_INNER)));
	shader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(FLASH_OUTER)));
	shader.setUniform("spotLight.constant", 1.0f);
	shader.setUniform("spotLight.linear", 0.07f);
	shader.setUniform("spotLight.exponent", 0.017f);
	shader.setUniform("spotLight.on", gFlashlightOn ? 1 : 0);

	shader.setUniform("reserveZ", RES_FRONT + WALL / 2);
	shader.setUniform("reserveAmbient", glm::vec3(0.03f, 0.033f, 0.045f));

	shader.setUniform("doorLight.ambient", glm::vec3(0.0f));
	shader.setUniform("doorLight.diffuse", DOOR_LIGHT_COLOR * 1.1f);
	shader.setUniform("doorLight.specular", DOOR_LIGHT_COLOR * 0.5f);
	shader.setUniform("doorLight.position", DOOR_LIGHT_POS);
	shader.setUniform("doorLight.direction", DOOR_LIGHT_TARGET - DOOR_LIGHT_POS);
	shader.setUniform("doorLight.cosInnerCone", glm::cos(glm::radians(35.0f)));
	shader.setUniform("doorLight.cosOuterCone", glm::cos(glm::radians(45.0f)));
	shader.setUniform("doorLight.constant", 1.0f);
	shader.setUniform("doorLight.linear", 0.03f);
	shader.setUniform("doorLight.exponent", 0.0f);
	shader.setUniform("doorLight.on", 1);

	float level = tubeLevel(gTime);
	shader.setUniform("tubeLight.ambient", glm::vec3(0.0f));
	shader.setUniform("tubeLight.diffuse", TUBE_COLOR * 0.7f * level);
	shader.setUniform("tubeLight.specular", TUBE_COLOR * 0.5f * level);
	shader.setUniform("tubeLight.position", TUBE_POS);
	shader.setUniform("tubeLight.constant", 1.0f);
	shader.setUniform("tubeLight.linear", 0.22f);
	shader.setUniform("tubeLight.exponent", 0.1f);
}

struct ShadowMap
{
	GLuint fbo, depthMap;
	int size;
	int texUnit;
	glm::mat4 lightSpace;
};
ShadowMap gSunShadow, gDoorShadow, gFlashShadow;

bool createShadowMap(ShadowMap& sm, int size, int texUnit)
{
	sm.size = size;
	sm.texUnit = texUnit;
	sm.lightSpace = glm::mat4(1.0f);

	glGenTextures(1, &sm.depthMap);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, sm.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glActiveTexture(GL_TEXTURE0);

	glGenFramebuffers(1, &sm.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, sm.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm.depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return ok;
}

void beginShadowPass(ShadowMap& sm, ShaderProgram& depthShader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, sm.fbo);
	glViewport(0, 0, sm.size, sm.size);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.5f, 4.0f);
	depthShader.use();
	depthShader.setUniform("lightSpace", sm.lightSpace);
}

void endShadowPass()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_CULL_FACE);
	if (gWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	int w, h;
	glfwGetFramebufferSize(gWindow, &w, &h);
	glViewport(0, 0, w, h);
}

void drawReserveCasters(ShaderProgram& shader)
{
	drawReserve(shader);
	drawBackWall(shader);
	drawCat(shader);
}

void setShadowUniforms(ShaderProgram& shader, bool flashShadows)
{
	shader.setUniformSampler("sunShadowMap", gSunShadow.texUnit);
	shader.setUniform("sunLightSpace", gSunShadow.lightSpace);
	shader.setUniformSampler("doorShadowMap", gDoorShadow.texUnit);
	shader.setUniform("doorLightSpace", gDoorShadow.lightSpace);
	shader.setUniformSampler("flashShadowMap", gFlashShadow.texUnit);
	shader.setUniform("flashLightSpace", gFlashShadow.lightSpace);
	shader.setUniform("flashShadows", flashShadows ? 1 : 0);
}

glm::mat4 beamMatrix()
{
	const glm::vec3& L = DOOR_LIGHT_POS;
	glm::vec3 hinge(RES_DOOR_X0, 0.0f, RES_FRONT);
	glm::vec3 side(RES_DOOR_X1, 0.0f, RES_FRONT);
	glm::vec3 edge = glm::vec3(doorMatrix() * glm::vec4(RES_DOOR_X1 - RES_DOOR_X0, 0.0f, 0.0f, 1.0f));

	float aHinge = atan2f(hinge.x - L.x, L.z - hinge.z);
	float aEdge = atan2f(edge.x - L.x, L.z - edge.z);
	glm::vec3 start = (aEdge > aHinge) ? edge : hinge;

	glm::mat4 m(1.0f);
	m[0] = glm::vec4(side - start, 0.0f);
	m[1] = glm::vec4(0.0f, RES_DOOR_H, 0.0f, 0.0f);
	m[2] = glm::vec4(start - L, 0.0f);
	m[3] = glm::vec4(L, 1.0f);
	return m;
}

void drawLightBeam(ShaderProgram& beamShader, const glm::mat4& view, const glm::mat4& projection)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	beamShader.use();
	beamShader.setUniform("view", view);
	beamShader.setUniform("projection", projection);
	beamShader.setUniform("model", beamMatrix());
	beamShader.setUniform("viewPos", fpsCamera.getPosition());
	beamShader.setUniform("time", gTime);
	beamShader.setUniform("mode", 0);
	beamShader.setUniform("color", DOOR_LIGHT_COLOR * 0.22f);
	gMesh[M_BEAM].draw();
	beamShader.setUniform("mode", 1);
	beamShader.setUniform("color", DOOR_LIGHT_COLOR * 0.9f);
	gMesh[M_DUST].draw();

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void drawCeilingLights(ShaderProgram& bulbShader)
{
	const float LANE_X[5] = { -9.4f, -4.5f, 0.0f, 4.5f, 9.4f };
	bulbShader.setUniform("lightColor", CEILING_LIGHT);
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 6; j++)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(LANE_X[i], STORE_HEIGHT - 0.06f, -11.5f + 3.5f * j))
							* glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.06f, 2.4f));
			bulbShader.setUniform("model", model);
			gMesh[M_BOX].draw();
		}

	bulbShader.setUniform("lightColor", glm::mix(glm::vec3(0.12f), TUBE_COLOR, tubeLevel(gTime)));
	bulbShader.setUniform("model", glm::translate(glm::mat4(1.0f), TUBE_POS) * glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 0.05f, 0.05f)));
	gMesh[M_BOX].draw();
}

void updateCat(float dt)
{
	gCat.stateTime += dt;
	glm::vec3 cam = fpsCamera.getPosition();

	glm::vec3 toCam = cam - gCat.pos;
	float yaw = 0.0f;
	if (glm::length(toCam) < 8.0f)
		yaw = glm::clamp(wrapAngle(atan2f(toCam.x, toCam.z) - gCat.heading), -1.2f, 1.2f);
	gCat.headYaw += (yaw - gCat.headYaw) * (1.0f - expf(-dt * 4.0f));

	float hit = flashlightHit(gCat.pos + glm::vec3(0.0f, 0.4f, 0.0f));
	gCat.eyeGlow = 0.15f + 2.5f * hit;

	switch (gCat.state)
	{
	case CAT_ON_SHELF:
		gCat.dazzled = (hit > 0.5f && cam.z < RES_FRONT) ? gCat.dazzled + dt : 0.0f;
		if (gCat.dazzled > 1.0f)
			startCatEvent();
		break;

	case CAT_PUSHING:
		if (gCat.stateTime > 1.2f)
		{

			const float T = 0.7f;
			glm::vec3 d = CAT_LANDING - gCat.pos;
			gCat.vel = glm::vec3(d.x / T, (d.y + 0.5f * GRAVITY * T * T) / T, d.z / T);
			gCat.heading = atan2f(d.x, d.z);
			gCat.state = CAT_JUMPING;
			gCat.stateTime = 0.0f;
		}
		break;

	case CAT_JUMPING:
		gCat.vel.y -= GRAVITY * dt;
		gCat.pos += gCat.vel * dt;
		if (gCat.pos.y <= 0.0f && gCat.vel.y < 0.0f)
		{
			gCat.pos.y = 0.0f;
			gCat.vel = glm::vec3(0.0f);
			gCat.state = CAT_RUNNING;
			gCat.waypoint = 0;
		}
		break;

	case CAT_RUNNING:
	case CAT_WALKING:
	{
		float speed = (gCat.state == CAT_RUNNING) ? 2.6f : 0.55f;
		glm::vec2 to = CAT_PATH[gCat.waypoint] - glm::vec2(gCat.pos.x, gCat.pos.z);
		float dist = glm::length(to);
		float step = glm::min(speed * dt, dist);
		if (dist > 0.0001f)
		{
			gCat.pos += glm::vec3(to.x, 0.0f, to.y) / dist * step;
			gCat.heading += wrapAngle(atan2f(to.x, to.y) - gCat.heading) * (1.0f - expf(-dt * 8.0f));
			gCat.stride += step * 9.0f;
		}
		if (dist - step < 0.01f)
		{
			gCat.waypoint++;
			if (gCat.waypoint == LOOP_START)
				gCat.state = CAT_WALKING;
			if (gCat.waypoint >= NUM_WAYPOINTS)
				gCat.waypoint = LOOP_START;
		}
		break;
	}
	}
}

void updateAnimations(float dt)
{
	gTime += dt;
	glm::vec3 cam = fpsCamera.getPosition();

	gFlashPos = cam + fpsCamera.getRight() * 0.22f - fpsCamera.getUp() * 0.22f;
	gFlashDir = glm::normalize(glm::mix(gFlashDir, fpsCamera.getLook(), 1.0f - expf(-dt * 14.0f)));

	glm::vec2 door((RES_DOOR_X0 + RES_DOOR_X1) / 2, RES_FRONT);
	float target = RES_DOOR_AJAR;
	if (glm::distance(glm::vec2(cam.x, cam.z), door) < 2.2f)
		target = 100.0f;
	else if (glm::distance(glm::vec2(gCat.pos.x, gCat.pos.z), door) < 1.3f)
		target = 70.0f;
	gDoorAngle += (target - gDoorAngle) * (1.0f - expf(-dt * 3.5f));

	updateCat(dt);
	updateFallingObjects(dt);
}

struct Viewpoint
{
	glm::vec3 pos, target;
};
const int NUM_VIEWPOINTS = 5;
const Viewpoint VIEWPOINTS[NUM_VIEWPOINTS] = {
	{ glm::vec3(0.0f, 1.7f, 19.0f),   glm::vec3(0.0f, 1.7f, 0.0f) },
	{ glm::vec3(-1.5f, 2.6f, 9.0f),   glm::vec3(4.0f, 0.5f, -8.0f) },
	{ glm::vec3(4.6f, 1.7f, -10.6f),  glm::vec3(8.5f, 1.1f, -14.3f) },
	{ glm::vec3(4.0f, 1.7f, -19.0f),  glm::vec3(9.0f, 0.9f, -14.8f) },
	{ glm::vec3(8.0f, 1.7f, -15.6f),  glm::vec3(2.4f, 1.9f, -17.4f) }
};

void goToViewpoint(int i)
{
	fpsCamera.lookAt(VIEWPOINTS[i].pos, VIEWPOINTS[i].target);
	gFlashDir = fpsCamera.getLook();
}

void saveScreenshot(const char* fileName)
{
	int w, h;
	glfwGetFramebufferSize(gWindow, &w, &h);
	std::vector<unsigned char> pixels((size_t)w * h * 3);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
	FILE* f = fopen(fileName, "wb");
	if (!f)
		return;
	fprintf(f, "P6\n%d %d\n255\n", w, h);
	for (int y = h - 1; y >= 0; y--)
		fwrite(&pixels[(size_t)y * w * 3], 1, (size_t)w * 3, f);
	fclose(f);
}

int main(int argc, char** argv)
{
	int shotView = 0, shotFlash = 0;
	Viewpoint shotCamera = VIEWPOINTS[0];
	float shotSeconds = 0.0f;
	const char* shotFile = NULL;
	if (argc == 6 && std::string(argv[1]) == "--shot")
	{
		gShotMode = true;
		shotView = glm::clamp(atoi(argv[2]), 1, NUM_VIEWPOINTS);
		float* c = &shotCamera.pos.x;
		if (sscanf(argv[2], "%f,%f,%f,%f,%f,%f", &c[0], &c[1], &c[2], &shotCamera.target.x, &shotCamera.target.y, &shotCamera.target.z) == 6)
			shotView = 0;
		shotSeconds = (float)atof(argv[3]);
		shotFlash = atoi(argv[4]);
		shotFile = argv[5];
	}

	if (!initOpenGL())
	{

		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader, bulbShader, depthShader, beamShader;
	lightingShader.loadShaders("shaders/supermarket.vert", "shaders/supermarket.frag");
	bulbShader.loadShaders("shaders/bulb.vert", "shaders/bulb.frag");
	depthShader.loadShaders("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");
	beamShader.loadShaders("shaders/lightbeam.vert", "shaders/lightbeam.frag");

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

	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		gTexture[i].bind(0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	buildObstacles();
	resetCat();
	resetFallingObjects();

	if (!createShadowMap(gSunShadow, 2048, 1) || !createShadowMap(gDoorShadow, 1024, 2) || !createShadowMap(gFlashShadow, 1024, 3))
	{
		std::cerr << "Cannot create the shadow maps" << std::endl;
		glfwTerminate();
		return -1;
	}

	glm::vec3 storeCenter(0.0f, 0.0f, -2.0f);
	gSunShadow.lightSpace = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.5f, 40.0f)
						  * glm::lookAt(storeCenter - SUN_DIR * 20.0f, storeCenter, glm::vec3(0.0f, 0.0f, -1.0f));
	beginShadowPass(gSunShadow, depthShader);
	drawSupermarket(depthShader, false);
	endShadowPass();

	gDoorShadow.lightSpace = glm::perspective(glm::radians(90.0f), 1.0f, 0.5f, 20.0f)
						   * glm::lookAt(DOOR_LIGHT_POS, DOOR_LIGHT_TARGET, glm::vec3(0.0f, 1.0f, 0.0f));
	beginShadowPass(gDoorShadow, depthShader);
	drawReserveCasters(depthShader);
	endShadowPass();

	if (gShotMode)
	{
		if (shotView > 0)
			goToViewpoint(shotView - 1);
		else
		{
			fpsCamera.lookAt(shotCamera.pos, shotCamera.target);
			gFlashDir = fpsCamera.getLook();
		}
		gFlashlightOn = shotFlash != 0;
		for (float t = 0.0f; t < shotSeconds; t += 1.0f / 30.0f)
			update(1.0 / 30.0);
	}
	int shotFrames = 0;
	double shotStart = 0.0;

	std::cout << "Mouse + W/S/A/D/Z/X: move, wheel: zoom, F: flashlight, F1: wireframe, ESC: quit" << std::endl
			  << "G: the cat knocks the products over, R: reset the reserve, 1..5: viewpoints" << std::endl;

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = gShotMode ? 0.0 : currentTime - lastTime;

		glfwPollEvents();
		update(deltaTime);

		glm::vec3 cam = fpsCamera.getPosition();
		if (cam.z < STORE_BACK + 6.0f && cam.x > 0.0f)
		{
			beginShadowPass(gDoorShadow, depthShader);
			drawReserveCasters(depthShader);
			endShadowPass();
		}
		bool flashShadows = gFlashlightOn && cam.z < RES_FRONT + 0.5f;
		if (flashShadows)
		{
			glm::vec3 up = fabsf(gFlashDir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
			gFlashShadow.lightSpace = glm::perspective(glm::radians(2.0f * FLASH_OUTER + 6.0f), 1.0f, 0.1f, 25.0f)
									* glm::lookAt(gFlashPos, gFlashPos + gFlashDir, up);
			beginShadowPass(gFlashShadow, depthShader);
			drawReserveCasters(depthShader);
			endShadowPass();
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view(1.0), projection(1.0);

		view = fpsCamera.getViewMatrix();

		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		lightingShader.use();
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", fpsCamera.getPosition());
		lightingShader.setUniform("worldUV", 0);
		setLightUniforms(lightingShader);
		setShadowUniforms(lightingShader, flashShadows);

		drawBackWall(lightingShader);
		drawCat(lightingShader);
		drawReserve(lightingShader);
		drawSupermarket(lightingShader);
		drawShadows(lightingShader);

		bulbShader.use();
		bulbShader.setUniform("view", view);
		bulbShader.setUniform("projection", projection);
		drawCeilingLights(bulbShader);

		drawLightBeam(beamShader, view, projection);

		if (gShotMode)
		{
			glFinish();
			if (++shotFrames == 1)
				shotStart = glfwGetTime();
			else if (shotFrames == 5)
			{
				std::cout << "Frame time: " << (glfwGetTime() - shotStart) * 1000.0 / 4.0 << " ms" << std::endl;
				saveScreenshot(shotFile);
				break;
			}
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

	if (!gShotMode)
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

	int w, h;
	glfwGetFramebufferSize(gWindow, &w, &h);
	glViewport(0, 0, w, h);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

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

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		startCatEvent();
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		resetCat();
		resetFallingObjects();
	}

	if (action == GLFW_PRESS && key >= GLFW_KEY_1 && key < GLFW_KEY_1 + NUM_VIEWPOINTS)
		goToViewpoint(key - GLFW_KEY_1);
	if (action == GLFW_PRESS && key >= GLFW_KEY_KP_1 && key < GLFW_KEY_KP_1 + NUM_VIEWPOINTS)
		goToViewpoint(key - GLFW_KEY_KP_1);
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
	if (gShotMode)
	{
		updateAnimations((float)elapsedTime);
		return;
	}
	glm::vec3 oldPos = fpsCamera.getPosition();

	double mouseX, mouseY;

	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	static bool mouseReady = false;
	bool mouseInWindow = glfwGetWindowAttrib(gWindow, GLFW_FOCUSED) && glfwGetWindowAttrib(gWindow, GLFW_HOVERED);

	if (mouseReady && mouseInWindow)
		fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);
	mouseReady = mouseInWindow;

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

	glm::vec3 newPos = fpsCamera.getPosition();
	if (isBlocked(newPos.x, oldPos.z))
		newPos.x = oldPos.x;
	if (isBlocked(newPos.x, newPos.z))
		newPos.z = oldPos.z;
	newPos.x = glm::clamp(newPos.x, -30.0f, 30.0f);
	newPos.z = glm::clamp(newPos.z, -30.0f, 40.0f);
	newPos.y = glm::clamp(newPos.y, 0.5f, (newPos.z < RES_FRONT ? RES_HEIGHT : STORE_HEIGHT) - 0.4f);
	if (fabsf(newPos.z - (STORE_BACK - WALL / 2)) < 0.6f && newPos.x > RES_DOOR_X0 - 0.5f && newPos.x < RES_DOOR_X1 + 0.5f)
		newPos.y = glm::min(newPos.y, RES_DOOR_H - 0.2f);
	fpsCamera.move(newPos - fpsCamera.getPosition());

	updateAnimations((float)elapsedTime);
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
			<< "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		frameCount = 0;
	}

	frameCount++;
}
