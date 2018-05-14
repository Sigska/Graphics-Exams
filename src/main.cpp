
//#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm/glm.hpp"

#include <vector>
#include <fstream>
//#include "shaderload.h"
#include <time.h>
#include <cstdlib>

#include <iostream>
#include <vector>

#include "stb_image.h"

#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "camera.h"
#include "map.h"
#include "const.h"
#include "player.h"
//#include <GL\freeglut.h>
#include "user.h"
#include <Windows.h>

#include "Shader.h"
//texture loadi8ng
#include "Mesh.hpp"
#include "Model.h"
#include "Plane.h"

#include </nyCmakeLab2/Build/ExternalLibraries/include/assimp/Importer.hpp>
#include </nyCmakeLab2/Build/ExternalLibraries/include/assimp/scene.h>
#include </nyCmakeLab2/Build/ExternalLibraries/include/assimp/postprocess.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//void characterCallback(GLFWwindow* window, unsigned int keyCode);
void processInput(GLFWwindow *window);

// swap season
glm::vec2 seasonSwitch(int season);


// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// plane view and position, just like above's camera
glm::vec3 startPos(1.0f, 3.0f, 2.0f);
glm::vec3 planePos = startPos;
glm::vec3 planeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 planeUp = glm::vec3(0.0f, 1.0f, 0.0f);

// for camera movement. Not in use because we follow the plane instead
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int season;		// Toggle between 4 seasons

				// The plane controller attitude
Plane planeControl(glm::vec3(2.0f, 2.0f, 2.0f));

// cam orbit
float orbit = 0.0f;



int main()
{
	// inits for glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//startup season
	season = 4;
	
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Lovely Exam", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCharCallback(window, characterCallback);


	// glew stuff
	glewExperimental = GL_TRUE;
	glewInit();

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// three shaders used: for plane, terrain, and lightsource respectively
	Shader planeShader("vertex.vert", "fragment.frag");
	Shader modelShader("model.vert", "model.frag");
	Shader lampShader("lamp.vert", "lamp.frag");




	// Load the plane texture since it didn't load into the model somehow.
	unsigned int planeTexture;
	glGenTextures(1, &planeTexture);
	glBindTexture(GL_TEXTURE_2D, planeTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("../Assets/plane.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);



	// The physical models: Terrain, lightsource, and plane respectively
	Model terrain("../Assets/heightmap/mountains.obj");
	Model lightSource("../Assets/light/light.obj");
	Model planeModel("../Assets/model/ask21mi.blend");
	

	// set plane texture
	planeShader.use();
	planeShader.setInt("texture1", 0);



	// lightsource properties
	float lampSpeed = 0.001f;
	glm::vec3 lampPos(1.0f, -10.0f, 2.0f);


	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// deltatime
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// reads userinputs
		processInput(window);

		// clear screen
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// properties to follow the plane
		float radius = 2.0f;
		float camX = sin(orbit) * radius;
		float camZ = cos(orbit) * radius;
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view;
		view = glm::lookAt(planePos - glm::vec3(camX, 0.0f, camZ), planePos + planeFront, planeUp);


		glm::mat4 model;


		// Update and draw the lightSource
		float sunRadius = 50.0f;
		float lampX = sin(currentFrame / 10.0f) * sunRadius;
		float lampZ = cos(currentFrame / 10.0f) * sunRadius;
		lampPos.y = lampX;
		lampPos.z = lampZ;
		lampShader.use();
	
	
		lampShader.setMat4("model", model);
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		lightSource.Draw(lampShader);


		// update and draw the plane
		planeShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		planePos += planeFront * planeControl.speed * deltaTime;
		model = glm::mat4();
		model = glm::translate(model, planePos); // translate it down so it's at the center of the scene
		
		
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// correct the plane
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(planeControl.pitch), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(planeControl.bank), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
		planeShader.setMat4("projection", projection);
		planeShader.setMat4("view", view);
		planeShader.setMat4("model", model);

		planeModel.Draw(planeShader);


		// swap between seasons
		glm ::vec2 seasonVec = seasonSwitch(season);


		// update season and draw terrain
		modelShader.use();
		modelShader.setVec2("season", seasonVec);


		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		modelShader.setMat4("model", model);
		modelShader.setVec3("lampPos", lampPos);
		terrain.Draw(modelShader);

		while (deltaTime > 10) {
			season += 1;
			if (season > 4)
				season = 1;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();

	return 0;
}

/**
* @param	a GLFWindow reference to the window
* @brief	Listen to which keys are pressed, and updates
*			camera position, glider attitude and camera orbit
*/
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;



	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		planeControl.changeBank(0.1f);
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		planeControl.changePitch(-0.1);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		planeControl.changeBank(-0.1);
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		planeControl.changePitch(0.1);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		planeControl.changeSpeed(0.001f);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		planeControl.changeSpeed(-0.001f);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		season = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		season = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		season = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		season = 4;


	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		orbit += 0.001f;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		orbit -= 0.001f;



	glm::vec3 front;
	front.z = cos(glm::radians(planeControl.bank)) * cos(glm::radians(planeControl.pitch));
	front.y = sin(glm::radians(planeControl.pitch));
	front.x = sin(glm::radians(planeControl.bank)) * cos(glm::radians(planeControl.pitch));
	planeFront = glm::normalize(front);

}

/**
*	@Param	A reference to GLFWindow and two integers for width and height
*	@brief	Resizes the window relatively to the new window size
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/**
*	@Param	A reference to GLFWwindow and two double mouse coordinates
*	@Brief	Chekcs if we moved the mouse a first time, and finds mouse position.
Also makes sure we don't go off limits.
*/
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

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;


	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


glm::vec2 seasonSwitch(int season)
{

	glm::vec2 seasonVec;

	switch (season)
	{
	case 1: seasonVec.x = -0.5f; seasonVec.y = 0.5f; break;
	case 2: seasonVec.x = -0.5f; seasonVec.y = 0.9f; break;
	case 3: seasonVec.x = -0.55f; seasonVec.y = 0.7f; break;
	case 4: seasonVec.x = -0.6f; seasonVec.y = -0.6f; break;
	}
	return seasonVec;
}