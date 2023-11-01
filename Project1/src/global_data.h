#pragma once
#include"all_Include.h"

double xpos, ypos;

float mixValue = 0.2;
const int screenWidth = 1280;
const int screenHeight = 768;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;
unsigned int planeVAO;
bool show_app_style_editor = false;
bool test = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

float _metallic = 0.0f;
float _roughness = 0.0f;
glm::vec3 _albedo = glm::vec3(1.0f);
bool use_metallicMap = false;
bool use_roughnessMap = false;
bool use_normalMap = false;
bool use_aoMap = false;
bool use_skybox = false;
bool use_albedoMap = false;
bool use_plane = false;

GLenum renderMode = GL_FILL;

unsigned int imGUItexture;
unsigned int Metallic;
unsigned int Roughness;
unsigned int Normal;
unsigned int AO;
unsigned int Albedo;


enum chage_object {
	sphere,
	plane,
	cube
};
static int object_index = 0;

glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 captureViews[] =
{
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

