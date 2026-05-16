/*
Práctica 5: Optimización y Carga de Modelos
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
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;
//float angulocola = 0.0f;
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;
Model Goddard_M;

//Auto
Model Carroceria_M;
Model LlantaDereFrente_M;
Model LlantaDereTracera_M;
Model LlantaIzqFrente_M;
Model LlantaIzqTracera_M;
Model Cofre_M;

Skybox skybox;

//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//Estado del coche 
// Posición del coche en el mundo
glm::vec3 carPosition = glm::vec3(4.0f, -2.0f, 0.0f);
const float CAR_SPEED = 3.0f;   // unidades/segundo

float wheelAngle = 0.0f;
const float WHEEL_SPEED = 90.0f;

float hoodAngle = 0.0f;
const float HOOD_SPEED = 45.0f;  // grados/segundo de apertura del cofre
const float HOOD_MAX = 45.0f;  // apertura máxima del cofre


// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";




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

	
	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

// Input del coche
void ProcessCarInput(bool* keys, GLfloat dt)
{
	// ↑ o R → avanzar en Z
	if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_R]) {
		carPosition.z -= CAR_SPEED * dt;
		wheelAngle += WHEEL_SPEED * dt;
	}

	// ↓ o F → retroceder en Z
	if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_F]) {
		carPosition.z += CAR_SPEED * dt;
		wheelAngle -= WHEEL_SPEED * dt;
	}

	// ← → desplazamiento lateral en X
	if (keys[GLFW_KEY_LEFT])
		carPosition.x -= CAR_SPEED * dt;
	if (keys[GLFW_KEY_RIGHT])
		carPosition.x += CAR_SPEED * dt;

	// Normalizar ángulo
	wheelAngle = fmod(wheelAngle, 2.5f);

	// O = abrir cofre | C = cerrar cofre
	if (keys[GLFW_KEY_O])
		hoodAngle = glm::min(hoodAngle + HOOD_SPEED * dt, HOOD_MAX);
	if (keys[GLFW_KEY_C])
		hoodAngle = glm::max(hoodAngle - HOOD_SPEED * dt, 0.0f);
}

int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 1.0f);

	Goddard_M = Model();
	Goddard_M.LoadModel("Models/goddard_base.obj");

	//Cargar modelos del coche 
	Carroceria_M = Model();
	Carroceria_M.LoadModel("Models/Carroceria.obj");

	LlantaDereFrente_M = Model();
	LlantaDereFrente_M.LoadModel("Models/LlantaDereFrente.obj");

	LlantaDereTracera_M = Model();
	LlantaDereTracera_M.LoadModel("Models/LlantaDereTracera.obj");

	LlantaIzqFrente_M = Model();
	LlantaIzqFrente_M.LoadModel("Models/LlantaIzqFrente.obj");

	LlantaIzqTracera_M = Model();
	LlantaIzqTracera_M.LoadModel("Models/LlantaIzqTracera.obj");

	Cofre_M = Model();
	Cofre_M.LoadModel("Models/Cofre.obj");


	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Coche
		ProcessCarInput(mainWindow.getsKeys(), deltaTime);

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Se dibuja el Skybox
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		
		// INICIA DIBUJO DEL PISO
		color = glm::vec3(0.5f, 0.5f, 0.5f); //piso de color gris
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMesh();

		//------------*INICIA DIBUJO DE NUESTROS DEMÁS OBJETOS-------------------*
		//Goddard
		color = glm::vec3(0.0f, 0.0f, 0.0f); //modelo de goddard de color negro
		
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, -1.5f));
		//modelaux = model;
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Goddard_M.RenderModel();//modificar por el modelo sin las 4 patas y sin cola
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		//En sesión se separara una parte del modelo de Goddard y se unirá por jeraquía al cuerpo
		modelaux = model;
		//Cola


		//Siguientes modelos
		/* Ejercicio:
		1.- Separar las 4 patas de Goddard del modelo del cuerpo, unir por medio de jerarquía cada pata al cuerpo de Goddard
		2.- Hacer que al presionar una tecla cada pata pueda rotar un máximo de 45° "hacia adelante y hacia atrás"
		*/

		//pata delantera derecha
		modelaux = model;

		// pata delantera izquierda
		modelaux = model;
		//pata trasera derecha
		modelaux = model;

		//pata trasera izquierda
		modelaux = model;

		
	 //  COCHE
	 //  Todas las partes se posicionan relativas a carPosition (jerarquía).
	 //
	 //  Ajusta los offsets de las llantas y el cofre según las dimensiones
	 //  reales de tus .obj.  Los valores a continuación son un buen punto
	 //  de partida típico para un coche a escala ~1:1.
	

	 // CARROCERÍA (raíz de la jerarquía del coche) 
		color = glm::vec3(0.8f, 0.1f, 0.1f);   // rojo
		model = glm::mat4(1.0f);
		model = glm::translate(model, carPosition);
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Carroceria_M.RenderModel();

		// Guardamos la matriz de la carrocería para heredar a hijos
		glm::mat4 carBase = model;

		color = glm::vec3(0.15f, 0.15f, 0.15f);  // color llantas

		// LLANTA DELANTERA DERECHA 
		modelaux = carBase;
		modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, 1.4f));
		modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		LlantaDereFrente_M.RenderModel();

		// LLANTA TRASERA DERECHA 
		modelaux = carBase;
		modelaux = glm::translate(modelaux, glm::vec3(1.3f, -0.55f, -1.4f));
		modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		LlantaDereTracera_M.RenderModel();

		// LLANTA DELANTERA IZQUIERDA 
		modelaux = carBase;
		modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, 1.4f));
		modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		LlantaIzqFrente_M.RenderModel();
		
		//LLANTA TRASERA IZQUIERDA
		modelaux = carBase;
		modelaux = glm::translate(modelaux, glm::vec3(-1.3f, -0.55f, -1.4f));
		modelaux = glm::rotate(modelaux, glm::radians(wheelAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		LlantaIzqTracera_M.RenderModel();


		//COFRE (hijo de la carrocería) 
		color = glm::vec3(0.8f, 0.0f, 0.0f); 
		modelaux = carBase;
		modelaux = glm::translate(modelaux, glm::vec3(0.0f, 0.5f, 1.8f));
		// Rotar para abrir (abre hacia arriba/bajo)
		modelaux = glm::rotate(modelaux, hoodAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		// Regresar al centro geométrico del cofre
		modelaux = glm::translate(modelaux, glm::vec3(0.0f, 0.0f, -0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelaux));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Cofre_M.RenderModel();


		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
