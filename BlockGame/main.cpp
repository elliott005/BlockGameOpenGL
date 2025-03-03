#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <filesystem> 

#include <Windows.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderClass.hpp"
#include "Application.hpp"
#include "Player.hpp"
#include "Cube.hpp"
#include "World.hpp"

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

int main(){
	Application game;
	bool success = game.createWindow(SCR_WIDTH, SCR_HEIGHT, "Block Game!");
	if (!success) {
		return -1;
	}

	glfwSetInputMode(game.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader program
    // ------------------------------------
	Shader defaultShader("defaultShader.vs", "defaultShader.fs");
	defaultShader.use();

	Player player(10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, game.window, SCR_WIDTH, SCR_HEIGHT, { defaultShader });

	double startTime = glfwGetTime();
	World world;
	printf("startup time: %f\n", glfwGetTime() - startTime);

	// load and create a texture 
	// -------------------------
	unsigned int tileSetTex;
	// texture 1
	// ---------
	glGenTextures(1, &tileSetTex);
	glBindTexture(GL_TEXTURE_2D, tileSetTex);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load("assets/sprite_059.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	defaultShader.use();
	defaultShader.setInt("texture1", 0);
	/*defaultShader.setFloat("numTilesX", 32);
	defaultShader.setFloat("numTilesY", 32);*/

	float deltaTime;
	float lastFrame = 0.0;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(game.window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		printf("FPS: %f\n", 1 / deltaTime);


		if (glfwGetKey(game.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(game.window, true);
		}

		player.update(game.window, deltaTime, &world);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tileSetTex);

		defaultShader.use();

		player.updateShader({ &defaultShader });

		world.draw(&defaultShader, player.position, player.front);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(game.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}