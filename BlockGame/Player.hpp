#pragma once

#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderClass.hpp"
#include <GLFW/glfw3.h>

#include "World.hpp"

class Player{
public:
	Player(float x, float y, float z, float rx, float ry, float rz, GLFWwindow* window, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, std::vector<Shader> shaders);

	void update(GLFWwindow* window, float deltaTime, World* world);
	void updateShader(std::vector<Shader*> shaders);

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 velocity{0.f, 0.f, 0.f};
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;
private:
	void updateCameraVectors();
	void handleCamera(GLFWwindow* window);
	void handleMovement(GLFWwindow* window, float deltaTime, World* world);
	void move(World* world);
	bool canMove(std::vector<Chunk*> nearbyChunks, glm::vec3 pos);

	glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 projection = glm::mat4(1.0f);

	float lastMouseX = 0.0f;
	float lastMouseY = 0.0f;
	float mouseSensitivity = 0.1;
	float maxVerticalRotation = 70.0;

	float speed = 7.0;
	float sprintSpeed = 12.0;
	float jumpHeight = 20.0;
	float gravity = 0.7;
	float verticalFlySpeed = 20.0;
	float flySpeed = 30.0;
	
	bool onGround = false;
	bool sprinting = false;

	float height = 1.8;
	float width = 0.4;

	bool flying = false;

	//std::vector<float> keysPressed;
};