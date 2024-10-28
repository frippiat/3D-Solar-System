#include "Camera.hpp"
// Getters and setters for camera

float Camera::getFov() const {
    return m_fov;
}

void Camera::setFoV(const float f) {
    m_fov = f;
}

float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

void Camera::setAspectRatio(const float a) {
    m_aspectRatio = a;
}

float Camera::getNear() const {
    return m_near;
}

void Camera::setNear(const float n) {
    m_near = n;
}

float Camera::getFar() const {
    return m_far;
}

void Camera::setFar(const float n) {
    m_far = n;
}

void Camera::setPosition(const glm::vec3 &p) {
    m_pos = p;
}

glm::vec3 Camera::getPosition() const {
    return m_pos;
}

// Compute the view matrix using glm::lookAt
glm::mat4 Camera::computeViewMatrix() const {
    return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

// Compute the projection matrix using glm::perspective
glm::mat4 Camera::computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
}

// Define the global camera instance
Camera g_camera;
