/*
Animación:
- Simple o básica:Por banderas y condicionales (más de 1 transformación geométrica se ve modificada)
-Compleja: Por medio de funciones y algoritmos.
-Textura Animada
*/

//para cargar imagen
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
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

//variables para animación
float posicionX;
float velocidadAuto;
float ruedaAngulo;
float ruedaVelocidad;
float anguloCarroceria;
float velocidadGiro;
int faseAuto;
float carrilZ;
float progresoVuelta;
float steeringAngle;

//Nave
float desplazamientoNave;
float alturaFlotacion;
float orientacionNave;
float inclinacionAla;
float rapidezNave;
float rapidezFlotacion;
float rapidezOrientacion;
float rapidezAla;
int faseNave;
int sentidoFlotacion;
int sentidoAla;
//bool avanza;
// 
//variables para animación de la flecha y los números
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
//Cambiar el tiempo de los numeros 
float angulovaria = 0.0f;
float toffsetnumerocambiauTimer = 0.0f;       // temporizador para cambiar número
float toffsetnumerocambiauInterval = 8.0f;   // intervalo en segundos (ajustar para más/menos velocidad)
//Aeolipile
//Variables para el fuego animado y el giro del Aeolipile.
// El fuego se activa con la tecla F, vive un tiempo definido y luego se extingue.
// El Aeolipile gira mientras haya fuego activo.
bool  fuegoActivo      = false;   // bandera: true mientras el fuego está encendido
float fuegoTimer       = 10.0f;    // acumula el tiempo transcurrido desde que se encendió
float fuegoDuracion    = 550.0f;    // segundos que dura el fuego antes de extinguirse
float fuegoAlpha       = 0.0f;    // opacidad actual del fuego (0=apagado, 1=pleno)
float rotAeolipile     = 0.0f;    // ángulo de giro acumulado de la esfera del Aeolipile
float velAeolipile     = 5.0f;   // grados por segundo cuando el fuego está activo
//Humo
float humoOffset1 = 0.0f;   // altura acumulada instancia 1
float humoOffset2 = 0.0f;   // altura acumulada instancia 2
float humoAlpha1 = 0.0f;   // opacidad instancia 1
float humoAlpha2 = 0.0f;   // opacidad instancia 2
float humoVelocidad = 0.8f;   // qué tan rápido sube
float humoReset = 3.0f;   // altura a la que se reinicia
// Variables catapulta
float rotBrazo = 1.0f;  // ángulo actual del brazo
float velBrazo = 100.0f;  // velocidad de giro del brazo
int   faseCatapulta = 0;       // 0=espera, 1=lanzando, 2=bajando, 3=recargando
float timerEspera = 0.0f;    // espera antes de disparar
float esperaInterval = 2.0f;   // segundos de espera

// Variables balón en vuelo
bool  balonEnVuelo = false;
float balonX = 0.0f;    // posición mundo X
float balonY = 0.0f;    // posición mundo Y
float balonZ = 0.0f;    // posición mundo Z
float balonVelX = 4.0f;    // velocidad horizontal
float balonVelY = 0.0f;    // velocidad vertical (se calcula al lanzar)
float balonVelYInicial = 12.0f;   // impulso vertical al lanzar
float gravedad = -10.0f;  // gravedad
int   rebotes = 0;       // contador de rebotes
int   maxRebotes = 3;       // cuántas veces rebota
float energiaRebote = 0.6f;    // % de velocidad que conserva al rebotar (0-1)
float pisoBayon = 1.0f;   // altura del suelo donde rebota

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;



Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Aeolipile_base_M;
Model Aeolipile_M;
Model Fuego_M;
Model Humo_M;
Model Balon_M;
Model Catapulta_Base;
Model Catapulta_Brazo;



Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";



//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

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


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
 		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};


	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	Mesh* obj5 = new Mesh();
	obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	Mesh* obj6 = new Mesh();
	obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6); // todos los números

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un número

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}




int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Aeolipile_base_M = Model();
	Aeolipile_base_M.LoadModel("Models/Aeolipile_base.obj");
	Aeolipile_M = Model();
	Aeolipile_M.LoadModel("Models/Aeolipile.obj");
	Fuego_M = Model();
	Fuego_M.LoadModel("Models/fuego.obj");
	Humo_M = Model();
	Humo_M.LoadModel("Models/smoke.obj");
	Balon_M = Model();
	Balon_M.LoadModel("Models/ball.obj");
	Catapulta_Base = Model();
	Catapulta_Base.LoadModel("Models/basecapaulta.obj");
	Catapulta_Brazo = Model();
	Catapulta_Brazo.LoadModel("Models/catapulta-1.obj");




	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.5f, 0.5f,
		0.0f, -1.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;



	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	posicionX = 0.0f;
	velocidadAuto = 0.1f;
	ruedaAngulo = 0.0f;
	ruedaVelocidad = 10.0f;
	anguloCarroceria = 0.0f;
	velocidadGiro = 1.15f;
	faseAuto = 0;
	carrilZ = -2.0f;
	progresoVuelta = 0.0f;
	steeringAngle = 0.0f;

	desplazamientoNave = 0.0f;
	alturaFlotacion = 0.0f;
	orientacionNave = 0.0f;
	inclinacionAla = 0.0f;
	rapidezNave = 0.06f;
	rapidezFlotacion = 0.05f;
	rapidezOrientacion = 1.0f;
	rapidezAla = 2.5f;
	faseNave = 0;
	sentidoFlotacion = 1;
	sentidoAla = 1;

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;

		// Animación 
		float limiteRecorrido = -195.0f;
		float radioCurva = 25.0f;
		float carrilInicial = -2.0f;
		float carrilVuelta = carrilInicial + (radioCurva * 2.0f);
		float anguloVolante = 28.0f;

		if (faseAuto == 0)
		{
			anguloCarroceria = 0.0f;
			carrilZ = carrilInicial;
			steeringAngle = 0.0f;

			if (posicionX > limiteRecorrido)
			{
				posicionX -= velocidadAuto * deltaTime;
				ruedaAngulo += ruedaVelocidad * deltaTime;
			}
			else
			{
				posicionX = limiteRecorrido;
				progresoVuelta = 0.0f;
				faseAuto = 1;
			}
		}
		else if (faseAuto == 1)
		{
			progresoVuelta += velocidadGiro * deltaTime;
			if (progresoVuelta > 180.0f)
				progresoVuelta = 180.0f;

			float rad = progresoVuelta * toRadians;
			anguloCarroceria = progresoVuelta;
			posicionX = limiteRecorrido - (radioCurva * sin(rad));
			carrilZ = carrilInicial + (radioCurva * (1.0f - cos(rad)));
			steeringAngle = anguloVolante;
			ruedaAngulo += ruedaVelocidad * deltaTime;

			if (progresoVuelta >= 180.0f)
			{
				posicionX = limiteRecorrido;
				carrilZ = carrilVuelta;
				anguloCarroceria = 180.0f;
				steeringAngle = 0.0f;
				faseAuto = 2;
			}
		}
		else if (faseAuto == 2)
		{
			anguloCarroceria = 180.0f;
			carrilZ = carrilVuelta;
			steeringAngle = 0.0f;

			if (posicionX < 0.0f)
			{
				posicionX += velocidadAuto * deltaTime;
				ruedaAngulo += ruedaVelocidad * deltaTime;
			}
			else
			{
				posicionX = 0.0f;
				progresoVuelta = 0.0f;
				faseAuto = 3;
			}
		}
		else if (faseAuto == 3)
		{
			progresoVuelta += velocidadGiro * deltaTime;
			if (progresoVuelta > 180.0f)
				progresoVuelta = 180.0f;

			float rad = progresoVuelta * toRadians;
			anguloCarroceria = 180.0f + progresoVuelta;
			posicionX = radioCurva * sin(rad);
			carrilZ = carrilVuelta - (radioCurva * (1.0f - cos(rad)));
			steeringAngle = anguloVolante;
			ruedaAngulo += ruedaVelocidad * deltaTime;

			if (progresoVuelta >= 180.0f)
			{
				posicionX = 0.0f;
				carrilZ = carrilInicial;
				anguloCarroceria = 0.0f;
				steeringAngle = 0.0f;
				faseAuto = 0;
			}
		}


		float limiteDesplazamiento = 20.0f;
		float techoFlotacion = 2.0f;
		float pisoFlotacion = -2.0f;
		float techoAla = 35.0f;
		float pisoAla = -35.0f;

		alturaFlotacion += rapidezFlotacion * deltaTime * sentidoFlotacion;
		if (alturaFlotacion >= techoFlotacion)
		{
			alturaFlotacion = techoFlotacion;
			sentidoFlotacion = -1;
		}
		else if (alturaFlotacion <= pisoFlotacion)
		{
			alturaFlotacion = pisoFlotacion;
			sentidoFlotacion = 1;
		}

		inclinacionAla += rapidezAla * deltaTime * sentidoAla;
		if (inclinacionAla >= techoAla)
		{
			inclinacionAla = techoAla;
			sentidoAla = -1;
		}
		else if (inclinacionAla <= pisoAla)
		{
			inclinacionAla = pisoAla;
			sentidoAla = 1;
		}

		if (faseNave == 0)
		{
			orientacionNave = 0.0f;
			if (desplazamientoNave < limiteDesplazamiento)
				desplazamientoNave += rapidezNave * deltaTime;
			else
			{
				desplazamientoNave = limiteDesplazamiento;
				faseNave = 1;
			}
		}
		else if (faseNave == 1)
		{
			orientacionNave += rapidezOrientacion * deltaTime;
			if (orientacionNave >= 180.0f)
			{
				orientacionNave = 180.0f;
				faseNave = 2;
			}
		}
		else if (faseNave == 2)
		{
			orientacionNave = 180.0f;
			if (desplazamientoNave > 0.0f)
				desplazamientoNave -= rapidezNave * deltaTime;
			else
			{
				desplazamientoNave = 0.0f;
				faseNave = 3;
			}
		}
		else if (faseNave == 3)
		{
			orientacionNave -= rapidezOrientacion * deltaTime;
			if (orientacionNave <= 0.0f)
			{
				orientacionNave = 0.0f;
				faseNave = 0;
			}
		}


		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Tecla F enciende el fuego y reinicia su duración
		if (mainWindow.getsKeys()[GLFW_KEY_F])
		{
			fuegoActivo = true;
			fuegoTimer = 0.0f;
		}

		//Avanzar el fuego cada frame
		if (fuegoActivo)
		{
			fuegoTimer += deltaTime;

			// Encendido suave: sube de 0 a 1 en los primeros 0.3 segundos
			if (fuegoTimer < 0.3f)
				fuegoAlpha = fuegoTimer / 0.3f;
			// Llama estable
			else if (fuegoTimer < fuegoDuracion - 2.0f)
				fuegoAlpha = 1.0f;
			// Extinción suave: baja de 1 a 0 en los últimos 2 segundos
			else if (fuegoTimer < fuegoDuracion)
				fuegoAlpha = 1.0f - ((fuegoTimer - (fuegoDuracion - 2.0f)) / 2.0f);
			// Fuego apagado completamente
			else
			{
				fuegoActivo = false;
				fuegoAlpha = 0.0f;
			}

			// GiraAeolipide
			float velGiroActual = fuegoActivo ? velAeolipile * fuegoAlpha : 5.0f;
			rotAeolipile += velGiroActual * deltaTime;
			if (rotAeolipile >= 360.0f)
				rotAeolipile -= 360.0f;

			// El Aeolipile gira solo mientras hay fuego
			// fuegoAlpha como multiplicador hace que arranque y frene suavemente
			if (fuegoActivo)
			{
				// Instancia 1: sube continuamente y se reinicia
				humoOffset1 += humoVelocidad * fuegoAlpha * deltaTime;
				if (humoOffset1 >= humoReset)
					humoOffset1 = 0.0f;

				// Instancia 2: desfasada a la mitad del ciclo para que no suban juntas
				humoOffset2 += humoVelocidad * fuegoAlpha * deltaTime;
				if (humoOffset2 >= humoReset)
					humoOffset2 = 0.0f;

				// Opacidad: aparece al salir y se desvanece al llegar arriba
				// Instancia 1
				if (humoOffset1 < humoReset * 0.3f)
					humoAlpha1 = humoOffset1 / (humoReset * 0.3f);       // fade in
				else if (humoOffset1 < humoReset * 0.7f)
					humoAlpha1 = 1.0f;                                    // pleno
				else
					humoAlpha1 = 1.0f - ((humoOffset1 - humoReset * 0.7f) / (humoReset * 0.3f)); // fade out

				// Instancia 2
				if (humoOffset2 < humoReset * 0.3f)
					humoAlpha2 = humoOffset2 / (humoReset * 0.3f);
				else if (humoOffset2 < humoReset * 0.7f)
					humoAlpha2 = 1.0f;
				else
					humoAlpha2 = 1.0f - ((humoOffset2 - humoReset * 0.7f) / (humoReset * 0.3f));

				// Multiplica por fuegoAlpha para que el humo también aparezca/desaparezca
				// suavemente junto con el fuego
				humoAlpha1 *= fuegoAlpha;
				humoAlpha2 *= fuegoAlpha;
			}
			else
			{
				// Fuego apagado: reiniciar humo
				humoOffset1 = 0.0f;
				humoOffset2 = humoReset * 0.5f; // desfase inicial para el próximo encendido
				humoAlpha1 = 0.0f;
				humoAlpha2 = 0.0f;
			}
		}

		// CATAPULTA: solo activa cuando el fuego está encendido
		if (fuegoActivo)
		{
			if (faseCatapulta == 0) 
			{
				timerEspera += deltaTime;
				rotBrazo = 1.0f;
				if (timerEspera >= esperaInterval)
				{
					timerEspera = 0.0f;
					faseCatapulta = 1; // ← AQUÍ estaba el bug, decía 0
				}
			}
			else if (faseCatapulta == 1) 
			{
				rotBrazo += velBrazo * deltaTime;
				if (rotBrazo >= 70.0f)
				{
					rotBrazo = 70.0f;
					balonEnVuelo = true;
					rebotes = 0;
					balonX = 15.0f; // ← igual que el translate X de la catapulta
					balonY = 2.5f;
					balonZ = 0.0f;
					balonVelY = balonVelYInicial;
					faseCatapulta = 2;
				}
			}
			else if (faseCatapulta == 2) // brazo baja lento
			{
				rotBrazo -= (velBrazo * 0.1f) * deltaTime;
				if (rotBrazo <= 1.0f)
				{
					rotBrazo = 1.0f;
					faseCatapulta = 3;
				}
			}
			else if (faseCatapulta == 3) // recarga y repite
			{
				timerEspera += deltaTime;
				if (timerEspera >= esperaInterval)
				{
					timerEspera = 0.0f;
					faseCatapulta = 0;
				}
			}
		}
		else
		{
			faseCatapulta = 0;
			rotBrazo = 1.0f;
			timerEspera = 0.0f;
		}

		// BALÓN física parabólica con rebotes 
		if (balonEnVuelo)
		{
			// Aplicar gravedad
			balonVelY += gravedad * deltaTime;

			// Mover balón
			balonZ += balonVelX * deltaTime;
			balonY += balonVelY * deltaTime;

			// Rebotar en el piso
			if (balonY <= pisoBayon)
			{
				balonY = pisoBayon;
				balonVelY = -balonVelY * energiaRebote; // rebota con menos energía
				rebotes++;

				// Cada rebote también pierde un poco de velocidad horizontal
				balonVelX *= 0.85f;

				if (rebotes >= maxRebotes)
				{
					// Terminó de rebotar: balón se detiene y desaparece
					balonEnVuelo = false;
					balonVelX = 4.0f; // restaurar para el próximo disparo
				}
			}
		}

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation(); // para la textura con movimiento

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		//Reinicializando variables cada ciclo de reloj
		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		//Pista
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.1f, 2.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Pista_M.RenderModel();

		// COCHE
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(posicionX - 50.0f, 0.5f, carrilZ));
		model = glm::rotate(model, anguloCarroceria * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel();

		// Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 8.0f));
		model = glm::rotate(model, steeringAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, ruedaAngulo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, ruedaAngulo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 1.5f));
		model = glm::rotate(model, steeringAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -ruedaAngulo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -ruedaAngulo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// NAVE
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(desplazamientoNave, 25.0f + alturaFlotacion, 1.5f));
		model = glm::rotate(model, (orientacionNave + 180.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();

		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.4f));
		model = glm::rotate(model, inclinacionAla * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala_M.RenderModel();

		//AEOLIPILE
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_base_M.RenderModel();

		
		model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
		model = glm::rotate(model, rotAeolipile * toRadians, glm::vec3(0.0f, 0.0f, 1.0f)); 
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_M.RenderModel();

		// HUMO: parte desde modelaux, hereda el giro del Aeolipile
		if (fuegoActivo || humoAlpha1 > 0.0f || humoAlpha2 > 0.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Instancia 1 - tubo 1
			if (humoAlpha1 > 0.0f)
			{
				float escalaHumo1 = 0.001f + humoOffset1 * 0.005f;

				color = glm::vec3(0.6f, 0.6f, 0.6f) * humoAlpha1;
				glUniform3fv(uniformColor, 1, glm::value_ptr(color));

				model = modelaux;
				model = glm::translate(model,glm::vec3(-1.0f, -0.8f, 0.0f));
				model = glm::rotate(model,-90.0f * toRadians, glm::vec3(0.0f, humoOffset1, 0.0f));
				model = glm::scale(model, glm::vec3(escalaHumo1, escalaHumo1 * 0.8f, escalaHumo1));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				plainTexture.UseTexture();
				Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
				Humo_M.RenderModel();
			}

			// Instancia 2 - tubo 2 (lado opuesto)
			if (humoAlpha2 > 0.0f)
			{
				float escalaHumo2 = 0.001f + humoOffset2 * 0.005f;

				color = glm::vec3(0.5f, 0.5f, 0.5f) * humoAlpha2;
				glUniform3fv(uniformColor, 1, glm::value_ptr(color));

				model = modelaux;
				model = glm::translate(model, glm::vec3(0.5f, 0.5f, -0.0f));
				model = glm::rotate(model,+90.0f * toRadians, glm::vec3(0.0f, humoOffset2, 0.0f));
				model = glm::scale(model, glm::vec3(escalaHumo2, escalaHumo2 * 0.8f, escalaHumo2));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				plainTexture.UseTexture();
				Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
				Humo_M.RenderModel();
			}

			glDisable(GL_BLEND);
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		}

		// CATAPULTA 
		// Posición base de toda la catapulta en la escena
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
		modelaux = model; // guardamos la base para que brazo y balón hereden posición

		// BASE
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(15.0f, -1.3f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.55f, 0.27f, 0.07f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		plainTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Catapulta_Base.RenderModel();

		// BRAZO
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f)); // pivote en espacio del modelo
		model = glm::rotate(model, rotBrazo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 modelBrazo = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.55f, 0.27f, 0.07f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		plainTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Catapulta_Brazo.RenderModel();

		// BALÓN en cuchara
		if (!balonEnVuelo)
		{
			model = modelBrazo;
			model = glm::translate(model, glm::vec3(-18.0f, 65.0f, -52.0f));
			// escala absoluta para el balón
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 2.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			plainTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			Balon_M.RenderModel();
		}
		else
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(balonX, balonY, balonZ));
			model = glm::rotate(model, angulovaria * 200.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			plainTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			Balon_M.RenderModel();
		}

		// Restablecer color
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		// Dibujar el fuego con Fuego_M, solo si está activo
		if (fuegoAlpha > 0.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Parpadeo usando angulovaria
			float parpadeo = 0.55f + 0.15f * sinf(angulovaria * 8.0f);
			color = glm::vec3(1.0f, 0.4f * parpadeo, 0.0f); // naranja-rojo
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));

			// La escala crece con fuegoAlpha: la llama "crece" al encenderse
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.5f ,1.5f));
			model = glm::rotate(model, angulovaria * 1.0f * toRadians, glm::vec3(0.0f, 10.0f, 0.0f)); // rotación leve de la llama
			model = glm::scale(model, glm::vec3(fuegoAlpha * 0.8f, fuegoAlpha * 1.0f, fuegoAlpha * 0.8f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			Fuego_M.RenderModel(); 

			glDisable(GL_BLEND);

			// Restablecer color blanco
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		}

		// Restablecer color blanco para el resto de la escena
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		

		//Agave ¿qué sucede si lo renderizan antes del coche y de la pista?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -2.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();

		//textura con movimiento
		//Importantes porque la variable uniform no podemos modificarla directamente
		toffsetflechau += 0.001;
		toffsetflechav = 0.000;
		//para que no se desborde la variable
		if (toffsetflechau > 1.0)
			toffsetflechau = 0.0;
		//if (toffsetv > 1.0)
		//	toffsetv = 0;
		//printf("\ntfosset %f \n", toffsetu);
		//pasar a la variable uniform el valor actualizado
		toffset = glm::vec2(toffsetflechau, toffsetflechav);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();

		//plano con todos los números
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMesh();

		//número 1
		//toffsetnumerou = 0.0;
		//toffsetnumerov = 0.0;
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		//glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		for (int i = 1; i < 4; i++)
		{
			//números 2-4
			toffsetnumerou += 0.25;
			toffsetnumerov = 0.0;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0), 2.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();

		}

		for (int j = 1; j < 5; j++)
		{
			//números 5-8
			toffsetnumerou += 0.25;
			toffsetnumerov = -0.33;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0), 5.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();
		}


		//número cambiante: cambiar cada toffsetnumerocambiauInterval segundos 
		toffsetnumerocambiauTimer += deltaTime;
		if (toffsetnumerocambiauTimer >= toffsetnumerocambiauInterval) {
			toffsetnumerocambiauTimer = 0.0f;
			toffsetnumerocambiau += 0.25f; // avanza una celda en el atlas (0.0,0.25,0.5,0.75)
			if (toffsetnumerocambiau > 0.75f) // vuelve al primer número después del último
				toffsetnumerocambiau = 0.0f;
		}
		toffsetnumerov = 0.0f;
		toffset = glm::vec2(toffsetnumerocambiau, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		//cambiar automáticamente entre textura número 1 y número 2
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Numero1Texture.UseTexture();
		//if
		//Numero1Texture.UseTexture();
		//Numero2Texture.UseTexture();

		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMesh();


		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}