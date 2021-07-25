#include "camera.h"

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetViewMatrix(first_person_camera_t* first_person_camera)
{
    return glm::lookAt(first_person_camera->Position, first_person_camera->Position + first_person_camera->Front, first_person_camera->Up);
}

glm::mat4 GetProjectMatrix(first_person_camera_t* first_person_camera, int screen_w, int screen_h) {
    return glm::perspective(glm::radians(first_person_camera->Zoom), (float)screen_w / (float)screen_h, 0.1f, 100.0f);
}

void updateCameraVectors(first_person_camera_t* first_person_camera)
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(first_person_camera->Yaw)) * cos(glm::radians(first_person_camera->Pitch));
    front.y = sin(glm::radians(first_person_camera->Pitch));
    front.z = sin(glm::radians(first_person_camera->Yaw)) * cos(glm::radians(first_person_camera->Pitch));
    first_person_camera->Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    first_person_camera->Right = glm::normalize(glm::cross(first_person_camera->Front, first_person_camera->WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    first_person_camera->Up = glm::normalize(glm::cross(first_person_camera->Right, first_person_camera->Front));
}

    

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void ProcessKeyboard(Camera_Movement direction, float deltaTime, first_person_camera_t* first_person_camera)
{
    float velocity = first_person_camera->MovementSpeed * deltaTime;
    if (direction == FORWARD)
        first_person_camera->Position += first_person_camera->Front * velocity;
    if (direction == BACKWARD)
        first_person_camera->Position -= first_person_camera->Front * velocity;
    if (direction == LEFT)
        first_person_camera->Position -= first_person_camera->Right * velocity;
    if (direction == RIGHT)
        first_person_camera->Position += first_person_camera->Right * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void ProcessMouseMovement(float xoffset, float yoffset,  first_person_camera_t* first_person_camera, bool constrainPitch)
{
    xoffset *= first_person_camera->MouseSensitivity;
    yoffset *= first_person_camera->MouseSensitivity;

    first_person_camera->Yaw += xoffset;
    first_person_camera->Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (first_person_camera->Pitch > 89.0f)
            first_person_camera->Pitch = 89.0f;
        if (first_person_camera->Pitch < -89.0f)
            first_person_camera->Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors(first_person_camera);
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ProcessMouseScroll(float yoffset, first_person_camera_t* first_person_camera)
{
    first_person_camera->Zoom -= (float)yoffset;
    if (first_person_camera->Zoom < 1.0f)
        first_person_camera->Zoom = 1.0f;
    if (first_person_camera->Zoom > 45.0f)
        first_person_camera->Zoom = 45.0f;
}

void control_camera(first_person_camera_t* camera, GLFWwindow* window, float delta_time) {
    int right_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
    double mouse_position_double[2];
    glfwGetCursorPos(window, &mouse_position_double[0], &mouse_position_double[1]);
    float mouse_position[2] = { (float)mouse_position_double[0], (float)mouse_position_double[1] };
    if (right_mouse_state == GLFW_PRESS) {
        if (camera->firstMouse)
        {
            camera->lastX = mouse_position[0];
            camera->lastY = mouse_position[1];
            camera->firstMouse = false;
        }

        float xoffset = mouse_position[0] - camera->lastX;
        float yoffset = mouse_position[1] - camera->lastY ; // reversed since y-coordinates go from bottom to top

        camera->lastX = mouse_position[0];
        camera->lastY = mouse_position[1];
        ProcessMouseMovement(xoffset, yoffset, camera, true);
    }
    if (right_mouse_state == GLFW_RELEASE) {
        camera->firstMouse = true;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        ProcessKeyboard(FORWARD, delta_time, camera);
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        ProcessKeyboard(BACKWARD, delta_time, camera);
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        ProcessKeyboard(RIGHT, delta_time, camera);
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        ProcessKeyboard(LEFT, delta_time, camera);
    }
}