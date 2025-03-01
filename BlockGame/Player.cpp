#include "Player.hpp"

Player::Player(float x, float y, float z, float rx, float ry, float rz, GLFWwindow* window, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, std::vector<Shader> shaders) {
	position.x = x;
	position.y = y;
	position.z = z;
	rotation.x = rx;
	rotation.y = ry;
	rotation.z = rz;
	worldUp = glm::vec3(0, 1, 0);

	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
	for (Shader sh : shaders) {
		sh.setMat4("projection", projection);
	}

	GLdouble xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	lastMouseX = xPos;
	lastMouseY = yPos;

	updateCameraVectors();
}

void Player::update(GLFWwindow* window, float deltaTime) {
	handleMovement(window, deltaTime);
	handleCamera(window);
}

void Player::updateShader(std::vector<Shader*> shaders) {
	view = glm::lookAt(position, position + front, up);
	for (int i = 0; i < shaders.size(); i++) {
		(*shaders[i]).setMat4("view", view);
	}
}

void Player::updateCameraVectors()
{
	// calculate the new Front vector
	glm::vec3 tempFront;
	tempFront.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	tempFront.y = sin(glm::radians(rotation.x));
	tempFront.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	front = glm::normalize(tempFront);
	// also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, front));
}

void Player::handleCamera(GLFWwindow* window) {
	GLdouble xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	float xOffset = xPos - lastMouseX;
	float yOffset = lastMouseY - yPos;
	lastMouseX = xPos;
	lastMouseY = yPos;
	rotation.y += xOffset * mouseSensitivity;
	rotation.x += yOffset * mouseSensitivity;
	rotation.x = std::min(maxVerticalRotation, std::max(-maxVerticalRotation, rotation.x));
	updateCameraVectors();
}

void Player::handleMovement(GLFWwindow* window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += front * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= front * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		position -= worldUp * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		position += worldUp * speed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}