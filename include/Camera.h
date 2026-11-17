#pragma once

#include <lineargl/lineargl.h>
#include <linearmath/linearmath.hpp>
#include <linearmath/gtc/matrix_transform.hpp>

#include <vector>

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  10.0f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    linearmath::vec3 Position;
    linearmath::vec3 Front;
    linearmath::vec3 Up;
    linearmath::vec3 Right;
    linearmath::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(linearmath::vec3 position = linearmath::vec3(0.0f, 5.0f, 10.0f), linearmath::vec3 up = linearmath::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    linearmath::mat4 GetViewMatrix() const;
    linearmath::mat4 GetProjectionMatrix(float width, float height) const;

    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSprinting = false);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void LookAt(linearmath::vec3 target);

private:
    void updateCameraVectors();
};



