#pragma once

#include <rblxgl/rblxgl.h>
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/gtc/matrix_transform.hpp>

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
    rblxmath::vec3 Position;
    rblxmath::vec3 Front;
    rblxmath::vec3 Up;
    rblxmath::vec3 Right;
    rblxmath::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(rblxmath::vec3 position = rblxmath::vec3(0.0f, 5.0f, 10.0f), rblxmath::vec3 up = rblxmath::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    rblxmath::mat4 GetViewMatrix() const;
    rblxmath::mat4 GetProjectionMatrix(float width, float height) const;

    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSprinting = false);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void LookAt(rblxmath::vec3 target);

private:
    void updateCameraVectors();
};



