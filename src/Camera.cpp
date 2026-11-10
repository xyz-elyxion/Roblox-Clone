#include "Camera.h"

Camera::Camera(rblxmath::vec3 position, rblxmath::vec3 up, float yaw, float pitch) 
    : Front(rblxmath::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

rblxmath::mat4 Camera::GetViewMatrix() const {
    return rblxmath::lookAt(Position, Position + Front, Up);
}

rblxmath::mat4 Camera::GetProjectionMatrix(float width, float height) const {
    return rblxmath::perspective(rblxmath::radians(Zoom), width / height, 0.1f, 1000.0f);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSprinting) {
    float velocity = MovementSpeed * deltaTime;
    if (isSprinting) velocity *= 4.0f; // Sprint multiplier
    
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity; // Studio style: E/Q or just standard
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f; 
}

void Camera::updateCameraVectors() {
    rblxmath::vec3 front;
    front.x = cos(rblxmath::radians(Yaw)) * cos(rblxmath::radians(Pitch));
    front.y = sin(rblxmath::radians(Pitch));
    front.z = sin(rblxmath::radians(Yaw)) * cos(rblxmath::radians(Pitch));
    Front = rblxmath::normalize(front);
    Right = rblxmath::normalize(rblxmath::cross(Front, WorldUp));
    Up    = rblxmath::normalize(rblxmath::cross(Right, Front));
}

void Camera::LookAt(rblxmath::vec3 target) {
    Front = rblxmath::normalize(target - Position);
    // Extract pitch and yaw from Front vector
    Pitch = rblxmath::degrees(asin(Front.y));
    Yaw   = rblxmath::degrees(atan2(Front.z, Front.x));
    updateCameraVectors();
}



