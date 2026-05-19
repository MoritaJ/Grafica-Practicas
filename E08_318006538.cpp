/*
 * Práctica 8 (Iluminación)
 */

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"   // Shader con iluminación (clase Shader)
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

 // Iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// ── Ventana / escena ──────────────────────────────────────────────────────────
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;
Skybox skybox;

// ── Texturas ──────────────────────────────────────────────────────────────────
Texture pisoTexture;
Texture dadoTexture;
Texture dado8Texture;
Texture cauchoTexture;
Texture metalTexture;
Texture ojosTexture;
Texture AgaveTexture;
Texture lamparaTexture;


// ── Modelos Auto ────────────────────────────────────────────
Model Carroceria_M;
Model Cofre_M;
Model Parabrisas_M;
Model Faros_M;
Model LlantaDD_M, LlantaDT_M, LlantaID_M, LlantaIT_M;
Model RinDD_M, RinDT_M, RinID_M, RinIT_M;

// ── Modelos ─────────────────────────────────────────────
Model Kitt_M;
Model Llanta_M;
Model Blackhawk_M;
Model Lamp_M;

// ── Materiales ────────────────────────────────────────────────────────────────
Material Material_brillante;
Material Material_opaco;

// ── Tiempo ────────────────────────────────────────────────────────────────────
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// ── Control de luces ───────────────────────────────────────────────────────
int faroColorIndex = 0;
glm::vec3 faroColores[] = {
    glm::vec3(1.0f, 0.0f, 0.0f),  // rojo
    glm::vec3(1.0f, 1.0f, 0.0f),  // amarillo
    glm::vec3(0.0f, 1.0f, 0.0f),  // verde
    glm::vec3(0.0f, 1.0f, 1.0f),  // cian
    glm::vec3(0.0f, 0.0f, 1.0f),  // azul
    glm::vec3(1.0f, 0.0f, 1.0f),  // magenta
};

// ── Control de Lamp ─────────────────────────────────────────────────────────
bool lampOn = true;

// ── Estado del coche ─────────────────────────────────────────────────────
glm::vec3 carPosition = glm::vec3(4.0f, -2.0f, 0.0f);
const float CAR_SPEED = 3.0f;
float wheelAngle = 0.0f;
const float WHEEL_SPEED = 90.0f;
float hoodAngle = 0.0f;
const float HOOD_SPEED = 45.0f;
const float HOOD_MAX = 45.0f;

// ── Estado del helicóptero ────────────────────────────────────────────────────
glm::vec3 heliPosition = glm::vec3(0.0f, 5.0f, 6.0f);
const float HELI_SPEED = 3.0f;

// ── Iluminación  ──────────────────────────────────────────────────────────
DirectionalLight mainLight;
PointLight  pointLights[MAX_POINT_LIGHTS];
SpotLight   spotLights[MAX_SPOT_LIGHTS];

// ── Shaders ───────────────────────────────────────────────────────────────────
// Usamos shader_light que soporta iluminación completa
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// ─────────────────────────────────────────────────────────────────────────────
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount,
    GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0],
            vertices[in1 + 1] - vertices[in0 + 1],
            vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0],
            vertices[in2 + 1] - vertices[in0 + 1],
            vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }
    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

// ── Geometría base: pirámide, piso, vegetación  →  meshList[0..3] ─────────────
void CreateObjects()
{
    unsigned int indices[] = { 0,3,1, 1,3,2, 2,3,0, 0,1,2 };
    GLfloat vertices[] = {
        -1.0f,-1.0f,-0.6f,  0.0f,0.0f,  0.0f,0.0f,0.0f,
         0.0f,-1.0f, 1.0f,  0.5f,0.0f,  0.0f,0.0f,0.0f,
         1.0f,-1.0f,-0.6f,  1.0f,0.0f,  0.0f,0.0f,0.0f,
         0.0f, 1.0f, 0.0f,  0.5f,1.0f,  0.0f,0.0f,0.0f
    };

    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        -10.0f,0.0f,-10.0f,  0.0f, 0.0f,  0.0f,-1.0f,0.0f,
         10.0f,0.0f,-10.0f,  10.0f,0.0f,  0.0f,-1.0f,0.0f,
        -10.0f,0.0f, 10.0f,  0.0f,10.0f,  0.0f,-1.0f,0.0f,
         10.0f,0.0f, 10.0f,  10.0f,10.0f, 0.0f,-1.0f,0.0f
    };

    unsigned int vegIndices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7 };
    GLfloat vegVertices[] = {
        -0.5f,-0.5f,0.0f,  0.0f,0.0f,  0.0f,0.0f,0.0f,
         0.5f,-0.5f,0.0f,  1.0f,0.0f,  0.0f,0.0f,0.0f,
         0.5f, 0.5f,0.0f,  1.0f,1.0f,  0.0f,0.0f,0.0f,
        -0.5f, 0.5f,0.0f,  0.0f,1.0f,  0.0f,0.0f,0.0f,
         0.0f,-0.5f,-0.5f, 0.0f,0.0f,  0.0f,0.0f,0.0f,
         0.0f,-0.5f, 0.5f, 1.0f,0.0f,  0.0f,0.0f,0.0f,
         0.0f, 0.5f, 0.5f, 1.0f,1.0f,  0.0f,0.0f,0.0f,
         0.0f, 0.5f,-0.5f, 0.0f,1.0f,  0.0f,0.0f,0.0f,
    };

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);

    Mesh* obj1 = new Mesh(); obj1->CreateMesh(vertices, indices, 32, 12); meshList.push_back(obj1); // [0]
    Mesh* obj2 = new Mesh(); obj2->CreateMesh(vertices, indices, 32, 12); meshList.push_back(obj2); // [1]
    Mesh* obj3 = new Mesh(); obj3->CreateMesh(floorVertices, floorIndices, 32, 6); meshList.push_back(obj3); // [2]
    Mesh* obj4 = new Mesh(); obj4->CreateMesh(vegVertices, vegIndices, 64, 12); meshList.push_back(obj4); // [3]
}

// ── Dado de 6 caras ──────────────────────────────────────────
void CrearDado()
{
    unsigned int cubo_indices[] = {
        0,1,2, 2,3,0,       // front
        8,9,10, 10,11,8,    // back
        12,13,14, 14,15,12, // left
        16,17,18, 18,19,16, // bottom
        20,21,22, 22,23,20, // top
        4,5,6, 6,7,4        // right
    };
    GLfloat cubo_vertices[] = {
        // front
        -0.5f,-0.5f, 0.5f,  0.3346f,0.0020f,  0.0f,0.0f,-1.0f,
         0.5f,-0.5f, 0.5f,  0.6653f,0.0020f,  0.0f,0.0f,-1.0f,
         0.5f, 0.5f, 0.5f,  0.6653f,0.2490f,  0.0f,0.0f,-1.0f,
        -0.5f, 0.5f, 0.5f,  0.3346f,0.2490f,  0.0f,0.0f,-1.0f,
        // right
         0.5f,-0.5f, 0.5f,  0.3423f,0.7520f,  -1.0f,0.0f,0.0f,
         0.5f,-0.5f,-0.5f,  0.6615f,0.7520f,  -1.0f,0.0f,0.0f,
         0.5f, 0.5f,-0.5f,  0.6615f,0.9912f,  -1.0f,0.0f,0.0f,
         0.5f, 0.5f, 0.5f,  0.3423f,0.9912f,  -1.0f,0.0f,0.0f,
         // back
         -0.5f,-0.5f,-0.5f,  0.6641f,0.2793f,  0.0f,0.0f,1.0f,
          0.5f,-0.5f,-0.5f,  0.3385f,0.2793f,  0.0f,0.0f,1.0f,
          0.5f, 0.5f,-0.5f,  0.3385f,0.4990f,  0.0f,0.0f,1.0f,
         -0.5f, 0.5f,-0.5f,  0.6641f,0.4990f,  0.0f,0.0f,1.0f,
         // left
         -0.5f,-0.5f,-0.5f,  0.3346f,0.5029f,  1.0f,0.0f,0.0f,
         -0.5f,-0.5f, 0.5f,  0.6627f,0.5029f,  1.0f,0.0f,0.0f,
         -0.5f, 0.5f, 0.5f,  0.6627f,0.7520f,  1.0f,0.0f,0.0f,
         -0.5f, 0.5f,-0.5f,  0.3346f,0.7520f,  1.0f,0.0f,0.0f,
         // bottom
         -0.5f,-0.5f, 0.5f,  0.3346f,0.5010f,  0.0f,1.0f,0.0f,
          0.5f,-0.5f, 0.5f,  0.0103f,0.5010f,  0.0f,1.0f,0.0f,
          0.5f,-0.5f,-0.5f,  0.0103f,0.7480f,  0.0f,1.0f,0.0f,
         -0.5f,-0.5f,-0.5f,  0.3346f,0.7480f, 0.0f,1.0f,0.0f,
         // top
         -0.5f,0.5f, 0.5f,   0.6691f,0.5029f,  0.0f,-1.0f,0.0f,
          0.5f,0.5f, 0.5f,   0.9910f,0.5029f,  0.0f,-1.0f,0.0f,
          0.5f,0.5f,-0.5f,   0.9910f,0.7520f,  0.0f,-1.0f,0.0f,
         -0.5f,0.5f,-0.5f,   0.6691f,0.7520f,  0.0f,-1.0f,0.0f,
    };
    Mesh* dado = new Mesh();
    dado->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
    meshList.push_back(dado); // [4]
}

void CrearDado8()
{
    GLfloat oct_vertices[] = {
        // Cara 1 — número "8"
         0.0f, 1.0f, 0.0f,  0.7540f,0.0000f,  0.5774f,-0.5774f,-0.5774f,
        -1.0f, 0.0f, 0.0f,  0.5000f,0.2483f,  0.5774f,-0.5774f,-0.5774f,
         0.0f, 0.0f, 1.0f,  1.0000f,0.2483f,  0.5774f,-0.5774f,-0.5774f,
        // Cara 2 — número "6"
        0.0f,-1.0f, 0.0f,  1.0000f,0.2483f,   0.5774f, 0.5774f,-0.5774f,
        0.0f, 0.0f, 1.0f,  0.7600f,0.4965f,   0.5774f, 0.5774f,-0.5774f,
        -1.0f, 0.0f, 0.0f,  0.5000f,0.2483f,  0.5774f, 0.5774f,-0.5774f,
        // Cara 3 — número "7"
        0.0f, 1.0f, 0.0f,  1.0000f,0.7448f,  -0.5774f,-0.5774f,-0.5774f,
        0.0f, 0.0f, 1.0f,  0.5000f,0.7448f,  -0.5774f,-0.5774f,-0.5774f,
        1.0f, 0.0f, 0.0f,  0.7600f,0.4965f,  -0.5774f,-0.5774f,-0.5774f,
        // Cara 4 — número "5"
        0.0f,-1.0f, 0.0f,  0.7520f,0.9931f,  -0.5774f, 0.5774f,-0.5774f,
        1.0f, 0.0f, 0.0f,  0.5000f,0.7448f,  -0.5774f, 0.5774f,-0.5774f,
        0.0f, 0.0f, 1.0f,  1.0000f,0.7448f,  -0.5774f, 0.5774f,-0.5774f,
        // Cara 5 — número "2"
        0.0f, 1.0f, 0.0f,  0.0000f,0.2483f,  -0.5774f,-0.5774f, 0.5774f,
        1.0f, 0.0f, 0.0f,  0.5000f,0.2483f,  -0.5774f,-0.5774f, 0.5774f,
        0.0f, 0.0f,-1.0f,  0.2400f,0.4965f,  -0.5774f,-0.5774f, 0.5774f,
        // Cara 6 — número "4"
        0.0f,-1.0f, 0.0f,  0.5000f,0.2483f,  -0.5774f, 0.5774f, 0.5774f,
        0.0f, 0.0f,-1.0f,  0.7600f,0.4965f,  -0.5774f, 0.5774f, 0.5774f,
        1.0f, 0.0f, 0.0f,  0.2460f,0.4965f,  -0.5774f, 0.5774f, 0.5774f,
        // Cara 7 — número "1"
        0.0f, 1.0f, 0.0f,  0.5000f,0.7448f,   0.5774f,-0.5774f, 0.5774f,
        0.0f, 0.0f,-1.0f,  0.2400f,0.4965f,   0.5774f,-0.5774f, 0.5774f,
        -1.0f, 0.0f, 0.0f,  0.7540f,0.4965f,  0.5774f,-0.5774f, 0.5774f,
        // Cara 8 — número "3"
        0.0f,-1.0f, 0.0f,  0.0000f,0.7448f,   0.5774f, 0.5774f, 0.5774f,
        -1.0f, 0.0f, 0.0f, 0.2400f,0.4965f,   0.5774f, 0.5774f, 0.5774f,
        0.0f, 0.0f,-1.0f,  0.5000f,0.7448f,   0.5774f, 0.5774f, 0.5774f,
    };
    unsigned int oct_indices[] = {
        0,1,2,  3,4,5,  6,7,8,  9,10,11,
        12,13,14,  15,16,17,  18,19,20,  21,22,23
    };
    Mesh* dado8 = new Mesh();
    dado8->CreateMesh(oct_vertices, oct_indices, 192, 24);
    meshList.push_back(dado8); // [5]
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

// ── Input del coche ──────────────────────────────────────────────────────
void ProcessCarInput(bool* keys, GLfloat dt)
{
    if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_R]) {
        carPosition.z -= CAR_SPEED * dt;
        wheelAngle += WHEEL_SPEED * dt;
    }
    if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_F]) {
        carPosition.z += CAR_SPEED * dt;
        wheelAngle -= WHEEL_SPEED * dt;
    }
    if (keys[GLFW_KEY_LEFT])  carPosition.x -= CAR_SPEED * dt;
    if (keys[GLFW_KEY_RIGHT]) carPosition.x += CAR_SPEED * dt;

    wheelAngle = fmod(wheelAngle, 2.5f);

    if (keys[GLFW_KEY_O]) hoodAngle = glm::min(hoodAngle + HOOD_SPEED * dt, HOOD_MAX);
    if (keys[GLFW_KEY_C]) hoodAngle = glm::max(hoodAngle - HOOD_SPEED * dt, 0.0f);

    //Faro (luz)
    static bool cPressed = false;
    if (keys[GLFW_KEY_C] && !cPressed) {
        faroColorIndex = (faroColorIndex + 1) % 6;
        cPressed = true;
    }
    if (!keys[GLFW_KEY_C]) cPressed = false;
}

//── Input del Blackhawk ──────────────────────────────────────────────────────
void ProcessHeliInput(bool* keys, GLfloat dt)
{
    if (keys[GLFW_KEY_I]) heliPosition.z -= HELI_SPEED * dt;
    if (keys[GLFW_KEY_K]) heliPosition.z += HELI_SPEED * dt;
}

// ── Input de la lámpara ──────────────────────────────────────────────────────
void ProcessLampInput(bool* keys, GLfloat dt)
{
    static bool lPressed = false;
    if (keys[GLFW_KEY_L] && !lPressed) {
        lampOn = !lampOn;
        lPressed = true;
    }
    if (!keys[GLFW_KEY_L]) lPressed = false;
}

// ══════════════════════════════════════════════════════════════════════════════
int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();  // meshList [0..3]
    CrearDado();      // meshList [4]
    CrearDado8();     // meshList [5]
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 1.0f);

    // ── Texturas ──────────────────────────────────────────────────────────────
    pisoTexture = Texture("Textures/piso.tga");              pisoTexture.LoadTextureA();
    dadoTexture = Texture("Textures/Dado_emociones.png");    dadoTexture.LoadTextureA();
    dado8Texture = Texture("Textures/8Dado.png");             dado8Texture.LoadTextureA();
    cauchoTexture = Texture("Textures/Caucho.png");           cauchoTexture.LoadTextureA();
    metalTexture = Texture("Textures/metal.png");             metalTexture.LoadTextureA();
    ojosTexture = Texture("Textures/ojitos.png");             ojosTexture.LoadTextureA();
    AgaveTexture = Texture("Textures/Agave.tga");             AgaveTexture.LoadTextureA();
    lamparaTexture = Texture("Textures/lam.png");             lamparaTexture.LoadTexture();

    // ── Modelos Auto  ───────────────────────────────────────
    Carroceria_M = Model(); Carroceria_M.LoadModel("Models/Carroceria.obj");
    Cofre_M = Model(); Cofre_M.LoadModel("Models/Cofre.obj");
    Parabrisas_M = Model(); Parabrisas_M.LoadModel("Models/Parabrisas2.obj");
    Faros_M = Model(); Faros_M.LoadModel("Models/Faros.obj");

    LlantaDD_M = Model(); LlantaDD_M.LoadModel("Models/LlantaDD.obj");
    LlantaDT_M = Model(); LlantaDT_M.LoadModel("Models/LlantaDT.obj");
    LlantaID_M = Model(); LlantaID_M.LoadModel("Models/LlantaID.obj");
    LlantaIT_M = Model(); LlantaIT_M.LoadModel("Models/LlantaIT.obj");

    RinDD_M = Model(); RinDD_M.LoadModel("Models/RinDD.obj");
    RinDT_M = Model(); RinDT_M.LoadModel("Models/RinDT.obj");
    RinID_M = Model(); RinID_M.LoadModel("Models/RinID.obj");
    RinIT_M = Model(); RinIT_M.LoadModel("Models/RinIT.obj");

    // ── Modelos  ─────────────────────────────────────────
    Kitt_M = Model(); Kitt_M.LoadModel("Models/kitt_optimizado.obj");
    Llanta_M = Model(); Llanta_M.LoadModel("Models/llanta_optimizada.obj");
    Blackhawk_M = Model(); Blackhawk_M.LoadModel("Models/uh60.obj");

    // ── Modelos  ─────────────────────────────────────────
	Model Lamp_M; Lamp_M.LoadModel("Models/Lamp.obj");


    // ── Skybox ────────────────────────────────────────────────────────────────
    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
    skybox = Skybox(skyboxFaces);

    // ── Materiales ────────────────────────────────────────────────────────────
    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // ── Iluminación ──────────────────────────────────────────────────────
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f);

    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
        0.0f, 1.0f,
        -6.0f, 1.5f, 1.5f,
        0.3f, 0.2f, 0.1f);
    pointLightCount++;

    unsigned int spotLightCount = 0;
    // Linterna (ligada a la cámara)
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f);
    spotLightCount++;
    // Luz fija verde
    spotLights[1] = SpotLight(0.0f, 1.0f, 0.0f,
        1.0f, 2.0f,
        5.0f, 10.0f, 0.0f,
        0.0f, -5.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;
    // Faro frontal del coche
    spotLights[2] = SpotLight(0.0f, 0.0f, 1.0f,   // color azul
        0.0f, 2.0f,                               // ambient, diffuse
        0.0f, 0.0f, 0.0f,                         // posición (se actualiza cada frame)
        0.0f, 0.0f, -1.0f,                        // dirección hacia adelante
        1.0f, 0.0f, 0.0f,                         // atenuación
        25.0f);                                   // ángulo del cono
    spotLightCount++;
    // Foco helicóptero (amarillo, apunta al piso)
    spotLights[3] = SpotLight(1.0f, 1.0f, 0.0f,   // amarillo
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,                         // apunta hacia abajo
        1.0f, 0.0f, 0.0f,
        20.0f);
    spotLightCount++;
    pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,   // blanco
        0.0f, 1.0f,
        0.0f, -4.0f, -10.0f,                        // posición: ajusta según donde pongas la lámpara
        0.3f, 0.2f, 0.1f);
    pointLightCount++;

    // ── Uniforms ──────────────────────────────────────────────────────────────
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0,
        uniformEyePosition = 0, uniformColor = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0;

    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 1000.0f);

    // ══════════════════════════════════════════════════════════════════════════
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        ProcessCarInput(mainWindow.getsKeys(), deltaTime);
        ProcessHeliInput(mainWindow.getsKeys(), deltaTime);
        ProcessLampInput(mainWindow.getsKeys(), deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Actualizar linterna a la posición de la cámara
        glm::vec3 lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        if (lampOn) {
            pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, -4.0f, -10.0f,
                0.5f, 0.3f, 0.1f);   // ← más concentrada
        }
        else {
            pointLights[1] = PointLight(0.0f, 0.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, -4.0f, -10.0f,
                0.5f, 0.3f, 0.1f);   // ← todo en 0, sin luz
        }
        unsigned int activeLights = lampOn ? pointLightCount : pointLightCount - 1;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        // Faro frontal — se mueve con el coche
        glm::vec3 faroPos = carPosition + glm::vec3(2.2f, 5.5f, 2.0f);
        glm::vec3 faroDir = glm::vec3(0.0f, -0.2f, -1.0f); 
        spotLights[2].SetFlash(faroPos, faroDir);

        spotLights[2] = SpotLight(
            faroColores[faroColorIndex].r,
            faroColores[faroColorIndex].g,
            faroColores[faroColorIndex].b,
            0.0f, 2.0f,
            faroPos.x, faroPos.y, faroPos.z,
            faroDir.x, faroDir.y, faroDir.z,
            1.0f, 0.0f, 0.0f,
            25.0f);

        // Foco helicóptero — se mueve con él
        glm::vec3 heliLightPos = heliPosition + glm::vec3(0.0f, -0.5f, 0.0f);
        glm::vec3 heliLightDir = glm::vec3(0.0f, -1.0f, 0.0f); // hacia el piso
        spotLights[3].SetFlash(heliLightPos, heliLightDir);

        // Enviar luces al shader
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0f), modelaux(1.0f);
        glm::vec3 color(1.0f);

        // ── PISO ──────────────────────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // ── DADO 6 CARAS ──────────────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 4.5f, -2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        dadoTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[4]->RenderMesh();

      
        // ── DADO 8 CARAS ──────────────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f, 4.5f, -2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        dado8Texture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        // ── COCHE ARTICULADO ─────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);   // azul
        model = glm::mat4(1.0f);
        model = glm::translate(model, carPosition);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        metalTexture.UseTexture();
        Carroceria_M.RenderModel();

        glm::mat4 carBase = model;   // padre jerárquico

        // Parabrisas
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        modelaux = carBase;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        ojosTexture.UseTexture();        
        Parabrisas_M.RenderModel();     // ← usa RenderModel normal
       
        // Faros
        modelaux = carBase;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Faros_M.RenderModel();

        // Cofre (articulado con tecla O/C)
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(0.0f, 0.5f, 1.8f));
        modelaux = glm::rotate(modelaux, hoodAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        modelaux = glm::translate(modelaux, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        metalTexture.UseTexture();
        Cofre_M.RenderModel();

        // Llantas
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, 1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        cauchoTexture.UseTexture();
        LlantaDD_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, -1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        cauchoTexture.UseTexture();
        LlantaDT_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, 1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        cauchoTexture.UseTexture();
        LlantaID_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, -1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        cauchoTexture.UseTexture();
        LlantaIT_M.RenderModel();

        // Rines
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, 1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		metalTexture.UseTexture();
        RinDD_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, -1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		metalTexture.UseTexture();
        RinDT_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, 1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		metalTexture.UseTexture();
        RinID_M.RenderModel();

        modelaux = carBase;
        modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, -1.4f));
        modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		metalTexture.UseTexture();
        RinIT_M.RenderModel();

        // ── BLACKHAWK ─────────────────────────────────────────────────────────────
        model = glm::mat4(1.0f);
        model = glm::translate(model, heliPosition);  // ← usa la variable
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 0 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Blackhawk_M.RenderModel();

        // ── LÁMPARA ───────────────────────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.3f, -10.0f)); // misma posición que la luz
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Lamp_M.RenderModel();

       /*  ── KITT + LLANTAS ───────────────────────────────────────────────
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f + mainWindow.getmuevex(), 0.5f, -3.0f));
        glm::mat4 kittBase = model;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Kitt_M.RenderModel();

         Llantas de Kitt
        color = glm::vec3(0.5f, 0.5f, 0.5f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        modelaux = kittBase;
        modelaux = glm::translate(modelaux, glm::vec3(7.0f, -0.5f, 8.0f));
        modelaux = glm::rotate(modelaux, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = glm::scale(modelaux, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        Llanta_M.RenderModel();

        modelaux = kittBase;
        modelaux = glm::translate(modelaux, glm::vec3(15.5f, -0.5f, 8.0f));
        modelaux = glm::rotate(modelaux, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = glm::scale(modelaux, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        Llanta_M.RenderModel();

        modelaux = kittBase;
        modelaux = glm::translate(modelaux, glm::vec3(7.0f, -0.5f, 1.5f));
        modelaux = glm::rotate(modelaux, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = glm::scale(modelaux, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        Llanta_M.RenderModel();

        modelaux = kittBase;
        modelaux = glm::translate(modelaux, glm::vec3(15.5f, -0.5f, 1.5f));
        modelaux = glm::rotate(modelaux, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = glm::scale(modelaux, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
        Llanta_M.RenderModel();*/

       /*  ── AGAVE con transparencia ─────────────────────────────────────
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        AgaveTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[3]->RenderMesh();
        glDisable(GL_BLEND);*/

      

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}