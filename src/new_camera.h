//
// Created by kalterkrieg on 28/01/2020.
//

#ifndef GLNEMO2_NEW_CAMERA_H
#define GLNEMO2_NEW_CAMERA_H

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include "globject.h"
#include "globaloptions.h"

using namespace glm;

namespace glnemo {

enum CameraMode {
  free,
  arcball
};

enum CubemapFace {
  right,
  first = right,
  left,
  top,
  bottom,
  front,
  back,
  last
};

class BaseCamera {
public:
  BaseCamera();
  ~BaseCamera() = default;
  virtual void reset() = 0;
  virtual void rotate(float, float, float) = 0;
  virtual const quat &getOrientation() const;
  bool isMatrixClean() const;
  const mat4 &getViewMatrix();
  const mat4 &getOrientationMatrix();
  const vec3 &getPosition() const;
  static quat fromAxisAngle(vec3 axis, double angle);
protected:
  virtual void buildMatrices();
  quat m_orientation;
  vec3 m_position;
  mat4 m_view_matrix;
  mat4 m_orientation_matrix;
  bool m_matrix_clean;
};

class ArcballCamera : public BaseCamera {
public:
  ArcballCamera();
  void rotate(float, float, float) override;
  float getZoom() const;
  void setZoom(float zoom);
  float increaseZoom();
  float decreaseZoom();
  void reset() override;

private:
  float m_zoom;
};

class FreeCamera : public BaseCamera {
public:
  FreeCamera() = default;
  void rotate(float, float, float) override;
  void setOrientation(quat);
  void reset() override;
  void moveForward(float dt);
  void moveBackward(float dt);
  void moveLeft(float dt);
  void moveRight(float dt);

private:
  float m_speed;
};

class NewCamera {
public:
  NewCamera();
  ~NewCamera();
  void reset();
  void rotate(float, float, float);
  const quat &getOrientation();
  const mat4 &getViewMatrix();
  const mat4 &getOrientationMatrix();
  const vec3 &getPosition() const;
  void setOrientation(quat);
  void setOrientation(CubemapFace face);
  void setZoom(float zoom);
  void toggleCameraMode();
  void setCameraMode(CameraMode);
  const CameraMode &getCameraMode() const;
  // float increaseSpeed();
  // float decreaseSpeed();
  float increaseZoom();
  float decreaseZoom();
  void moveForward(float dt);
  void moveBackward(float dt);
  void moveLeft(float dt);
  void moveRight(float dt);
private:
  FreeCamera *m_free_camera;
  ArcballCamera *m_arcball_camera;
  CameraMode m_mode;
  BaseCamera *m_current_camera;
};
}

#endif //GLNEMO2_NEW_CAMERA_H
