//#include "Camera2.hpp"
//namespace Control
//{
//	MouseButtons mouseButtons;
//	glm::vec2 mousePos;
//	void Camera2::Control_camera(GLFWwindow* window, float delta_time)
//	{
//		{
//			mouseButtons.right = false;
//			mouseButtons.left = false;
//			mouseButtons.middle = false;
//
//			int right_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
//			int left_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
//			double mouse_position_double[2];
//			glfwGetCursorPos(window, &mouse_position_double[0], &mouse_position_double[1]);
//			mousePos.x = mouse_position_double[0];
//			mousePos.y = mouse_position_double[1];
//
//			float mouse_position[2] = { (float)mouse_position_double[0], (float)mouse_position_double[1] };
//			if (right_mouse_state == GLFW_PRESS) {
//				mouseButtons.right = true;
//				if (firstMouse)
//				{
//					lastX = mouse_position[0];
//					lastY = mouse_position[1];
//					firstMouse = false;
//				}
//
//				float xoffset = -(mouse_position[0] - lastX);
//				float yoffset = -(mouse_position[1] - lastY); // reversed since y-coordinates go from bottom to top
//
//				lastX = mouse_position[0];
//				lastY = mouse_position[1];
//				rotate(glm::vec3(yoffset * rotationSpeed, -xoffset * rotationSpeed, 0.0f));
//				//ProcessMouseMovement(xoffset, yoffset, true);
//			}
//
//			if (right_mouse_state == GLFW_RELEASE) {
//				firstMouse = true;
//			}
//
//			if (left_mouse_state == GLFW_PRESS) {
//				mouseButtons.left = true;
//			}
//
//			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
//				this->keys.up = true;
//			}
//			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
//				this->keys.down = true;
//			}
//			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
//				this->keys.right = true;
//			}
//			else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
//				this->keys.left = true;
//			}
//			update(delta_time);
//		}
//	}
//
//	glm::mat4 Camera2::GetViewMatrix()
//	{
//		return this->matrices.view;
//	}
//
//	glm::vec3 Camera2::GetViewPos()
//	{
//		return (this->position * -1.f);
//	}
//
//	float Camera2::GetNearPlane()
//	{
//		return this->getNearClip();
//	}
//
//	float Camera2::GetFarPlane()
//	{
//		return this->getFarClip();
//	}
//
//	glm::mat4 Camera2::GetProjectMatrix()
//	{
//		return this->matrices.perspective;
//	}
//}