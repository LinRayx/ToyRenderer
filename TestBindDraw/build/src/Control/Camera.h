#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Control
{
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class Camera
	{
    public:
        Camera(int sc_width, int sc_height, float move_speed = 0.5, float move_sen = 0.5);
        glm::mat4 GetViewMatrix();
        glm::mat4 GetProjectMatrix(int screen_w, int screen_h);
        void ProcessKeyboard(Camera_Movement direction, float deltaTime);
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
        void ProcessMouseScroll(float yoffset);
        void Control_camera(GLFWwindow* window, float delta_time);
    private:
        void updateCameraVectors();
       
	private:
        // camera Attributes
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;
        // euler Angles
        float Yaw;
        float Pitch;
        // camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;

        bool firstMouse;
        float lastX;;
        float lastY;
	};
}

#endif // !CAMERA_H
