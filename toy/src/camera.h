#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



/*! Holds state for a first person camera that characterizes the world to
	projection space transform completely, except for the aspect ratio. It also
	provides enough information to update the camera interactively. It does not
	store any transforms or other redundant information. Such information has
	to be computed as needed.*/

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


struct first_person_camera_t {
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
};

glm::mat4 GetViewMatrix(first_person_camera_t* first_person_camera);
glm::mat4 GetProjectMatrix(first_person_camera_t* first_person_camera, int screen_w, int screen_h);
void updateCameraVectors(first_person_camera_t* first_person_camera);
#endif