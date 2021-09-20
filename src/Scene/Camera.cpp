#include "Camera.h"
#include "Utils/GloableClass.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

namespace Control
{

	MouseButtons mouseButtons;
	glm::vec2 mousePos;

	Camera::Camera(int sc_width, int sc_height, float move_speed, float move_sen) 
		: sc_width(sc_width), sc_height(sc_height)
	{
		Position = glm::vec3(0, 5, 5);
		WorldUp = glm::vec3(0.0f, -1.0f, 0.0f);
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Zoom = 45.f;
		Yaw = -90.0f;
		Pitch = 0.0f;
		firstMouse = true;
		lastX = sc_width / 2;
		lastY = sc_height / 2;
		MouseSensitivity = move_sen;
		MovementSpeed = move_speed;
		nearPlane = 0.1f;
		farPlane = 256.f;
		updateCameraVectors();
	}
	glm::mat4 Camera::GetViewMatrix()
	{
		auto view = glm::lookAt(Position, Position + Front, Up);
		//if (Gloable::FilpY)
		//	view[1][1] *= -1;
		return view;
	}
	glm::mat4 Camera::GetProjectMatrix()
	{
		auto proj = glm::perspective(glm::radians(45.f), (float)sc_width / (float)sc_height, nearPlane, farPlane);
		//if (Gloable::FilpY)
		//	proj[1][1] *= -1;
		return proj;
	}
	glm::mat4 Camera::GetCustomProjectMatrix(float aspect, float near, float far, float radians)
	{
		return glm::perspective(glm::radians(radians), aspect, near, far);
	}
	glm::vec3 Camera::GetViewPos()
	{
		return Position;
	}
	glm::vec3 Camera::GetWorldUp()
	{
		return WorldUp;
	}
	float Camera::GetNearPlane()
	{
		return nearPlane;
	}
	float Camera::GetFarPlane()
	{
		return farPlane;
	}
	float Camera::GetYaw()
	{
		return Yaw;
	}
	float Camera::GetPitch()
	{
		return Pitch;
	}
	void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}
	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}
	void Camera::ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}
	void Camera::updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross( Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
	void Camera::Control_camera(GLFWwindow* window, float delta_time)
	{
		mouseButtons.right = false;
		mouseButtons.left = false;
		mouseButtons.middle = false;
		int right_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
		int left_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
		double mouse_position_double[2];
		glfwGetCursorPos(window, &mouse_position_double[0], &mouse_position_double[1]);
		mousePos.x = static_cast<float>(mouse_position_double[0]);
		mousePos.y = static_cast<float>(mouse_position_double[1]);

		float mouse_position[2] = { (float)mouse_position_double[0], (float)mouse_position_double[1] };
		if (right_mouse_state == GLFW_PRESS) {
			mouseButtons.right = true;
			if (firstMouse)
			{
				lastX = mouse_position[0];
				lastY = mouse_position[1];
				firstMouse = false;
			}

			float xoffset = -(mouse_position[0] - lastX);
			float yoffset = (lastY - mouse_position[1]); // reversed since y-coordinates go from bottom to top

			lastX = mouse_position[0];
			lastY = mouse_position[1];
			ProcessMouseMovement(xoffset, yoffset, true);
		}

		if (right_mouse_state == GLFW_RELEASE) {
			firstMouse = true;
		}

		if (left_mouse_state == GLFW_PRESS) {
			mouseButtons.left = true;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			ProcessKeyboard(FORWARD, delta_time);
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			ProcessKeyboard(BACKWARD, delta_time);
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			ProcessKeyboard(RIGHT, delta_time);
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			ProcessKeyboard(LEFT, delta_time);
		}
	}
}