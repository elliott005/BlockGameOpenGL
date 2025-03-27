#include "Player.hpp"

Player::Player(float x, float y, float z, float rx, float ry, float rz, GLFWwindow* window, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, std::vector<Shader> shaders) {
	position.x = x;
	position.y = y;
	position.z = z;
	rotation.x = rx;
	rotation.y = ry;
	rotation.z = rz;
	worldUp = glm::vec3(0, 1, 0);

	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	for (Shader sh : shaders) {
		sh.setMat4("projection", projection);
	}

	GLdouble xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	lastMouseX = xPos;
	lastMouseY = yPos;

	updateCameraVectors();
}

void Player::update(GLFWwindow* window, float deltaTime, World* world) {
	handleMovement(window, deltaTime, world);
	handleCamera(window);
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Player::updateShader(std::vector<Shader*> shaders) {
	view = glm::lookAt(position, position + front, up);
	for (int i = 0; i < shaders.size(); i++) {
		// this doesn't work cause you need to use the shader before. to fix
		shaders[i]->setMat4("view", view);
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

void Player::handleMovement(GLFWwindow* window, float deltaTime, World* world) {
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		flying = false;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		flying = true;
		velocity.y = 0.0;
	}

	velocity = glm::vec3(0.0f, velocity.y, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		sprinting = true;
	}
	float moveSpeed = speed;
	if (sprinting) {
		moveSpeed = sprintSpeed;
	}
	if (flying) {
		moveSpeed = flySpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		velocity += glm::normalize(glm::vec3(front.x, 0.0f, front.z));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		velocity += -glm::normalize(glm::vec3(front.x, 0.0f, front.z));
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		velocity += -glm::normalize(glm::vec3(right.x, 0.0f, right.z));
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		velocity += glm::normalize(glm::vec3(right.x, 0.0f, right.z));
	}
	if (velocity.x == 0.0 and velocity.z == 0.0) {
		sprinting = false;
	}
	else {
		glm::vec3 tempVel = glm::normalize(glm::vec3(velocity.x, 0.0f, velocity.z)) * moveSpeed * deltaTime;
		velocity.x = tempVel.x;
		velocity.z = tempVel.z;
	}
	
	if (flying) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			position -= worldUp * verticalFlySpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			position += worldUp * verticalFlySpeed * deltaTime;
		}
	}
	else {
		velocity -= worldUp * gravity * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS and onGround) {
			velocity = worldUp * jumpHeight * deltaTime;
		}
	}
	move(world);
}

void Player::move(World* world) {
	std::vector<Chunk*> nearbyChunks;
	for (int i = 0; i < world->chunks.size(); i++) {
		if (glm::distance(glm::vec3(position.x, 0.0f, position.z), world->chunks[i].center) < world->chunks[i].chunkSize) {
			nearbyChunks.push_back(&world->chunks[i]);
		}
	}

	glm::vec3 tempPosition;
	if (velocity.x) {
		tempPosition = position + glm::vec3(velocity.x, 0, 0);
		if (canMove(nearbyChunks, tempPosition)) {
			position = tempPosition;
		}
	}
	if (velocity.y) {
		tempPosition = position + glm::vec3(0, velocity.y, 0);
		if (canMove(nearbyChunks, tempPosition)) {
			onGround = false;
			position = tempPosition;
		} else {
			onGround = true;
			velocity.y = 0.0f;
		}
	}
	if (velocity.z) {
		tempPosition = position + glm::vec3(0, 0, velocity.z);
		if (canMove(nearbyChunks, tempPosition)) {
			position = tempPosition;
		}
	}
	//position += velocity;
}

bool Player::canMove(std::vector<Chunk*> nearbyChunks, glm::vec3 pos) {
	for (Chunk* chunk : nearbyChunks) {
		glm::vec3 localPos = pos - chunk->position;
		//printf("%f, %f\n", localPos.x, localPos.z);
		for (int x = std::floor(localPos.x - width); x <= std::ceil(localPos.x + width); x++) {
			for (int y = std::floor(localPos.y - height); y <= std::ceil(localPos.y); y++) {
				for (int z = std::floor(localPos.z - width); z <= std::ceil(localPos.z + width); z++) {
					/*printf("%i, %i, %i\n", x, y, z);
					printf("Chunk: %f, %f\n", chunk->position.x, chunk->position.z);
					printf("cubes sizes: %i, %i, %i\n", chunk->cubes.size(), chunk->cubes[0].size(), chunk->cubes[0][0].size());*/
					Cube* cubeTuple = chunk->getBlockAt(x, y, z, 0);
					if (not cubeTuple->isAir) {
						return false;
					}
				}
			}
		}
	}
	return true;
}