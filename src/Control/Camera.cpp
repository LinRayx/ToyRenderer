#include "Camera.h"

namespace Control
{
	Camera::Camera(int sc_width, int sc_height, float move_speed, float move_sen) 
		: sc_width(sc_width), sc_height(sc_height)
	{
		Position = glm::vec3(0, 0, 3);
		WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Zoom = 45.0f;
		Yaw = -90.0f;
		Pitch = 0.0f;
		firstMouse = true;
		lastX = sc_width / 2;
		lastY = sc_height / 2;
		MouseSensitivity = move_sen;
		MovementSpeed = move_speed;
		updateCameraVectors();
	}
	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(Position,Position + Front, Up);
	}
	glm::mat4 Camera::GetProjectMatrix()
	{
		return glm::perspective(glm::radians(Zoom), (float)sc_width / (float)sc_height, 0.1f, 100.0f);
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
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
	void Camera::Control_camera(GLFWwindow* window, float delta_time)
	{
		int right_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
		double mouse_position_double[2];
		glfwGetCursorPos(window, &mouse_position_double[0], &mouse_position_double[1]);
		float mouse_position[2] = { (float)mouse_position_double[0], (float)mouse_position_double[1] };
		if (right_mouse_state == GLFW_PRESS) {
			if (firstMouse)
			{
				lastX = mouse_position[0];
				lastY = mouse_position[1];
				firstMouse = false;
			}

			float xoffset = mouse_position[0] - lastX;
			float yoffset = mouse_position[1] - lastY; // reversed since y-coordinates go from bottom to top

			lastX = mouse_position[0];
			lastY = mouse_position[1];
			ProcessMouseMovement(xoffset, yoffset, true);
		}
		if (right_mouse_state == GLFW_RELEASE) {
			firstMouse = true;
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