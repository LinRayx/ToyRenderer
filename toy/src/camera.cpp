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