#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext.hpp>


class Camera {
public:
  // Getters and setters for camera parameters
  float getFov() const;
  void setFoV(const float f);
  float getAspectRatio() const;
  void setAspectRatio(const float a);
  float getNear() const;
  void setNear(const float n);
  float getFar() const;
  void setFar(const float n);
  void setPosition(const glm::vec3 &p);
  glm::vec3 getPosition() const;

  // View matrix computation
  glm::mat4 computeViewMatrix() const;

  // Projection matrix computation
  glm::mat4 computeProjectionMatrix() const;

private:
  // Camera properties
  glm::vec3 m_pos = glm::vec3(0, 0, 0);
  float m_fov = 45.f;        // Field of view, in degrees
  float m_aspectRatio = 1.f; // Ratio between the width and height of the image
  float m_near = 0.1f;       // Near clipping plane distance
  float m_far = 10.f;        // Far clipping plane distance
};

// Global camera instance
extern Camera g_camera;

#endif // CAMERA_HPP
