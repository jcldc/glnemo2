//
// Created by kalterkrieg on 28/01/2020.
//

#ifndef GLNEMO2_NEW_CAMERA_H
#define GLNEMO2_NEW_CAMERA_H

#include <QObject>
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

class BaseCamera{
public:
  BaseCamera();
  ~BaseCamera() = default;
  virtual void reset() = 0;
  virtual void rotate(float, float, float) = 0;
  virtual const quat &getRotation() const;
  bool isMatrixClean() const;
  const mat4 &getViewMatrix();
protected:
  virtual void buildMatrix() = 0;
  static quat fromAxisAngle(vec3 axis, double angle);
  quat m_rotation;
  vec3 m_position;
  mat4 m_view_matrix;
  bool m_matrix_clean;
};

class ArcballCamera: public BaseCamera {
public:
  ArcballCamera();
  void rotate(float, float, float) override;
  float getZoom() const;
  void setZoom(float zoom);
  void reset() override ;

private:
  void buildMatrix() override;
  float m_zoom;
};

class FreeCamera : public BaseCamera {
public:
  FreeCamera() = default;
  void rotate(float, float, float) override;
  void reset() override {};

private:
  void buildMatrix() override;
};

class NewCamera {
public:
  NewCamera();
  ~NewCamera();
  void reset();
  void rotate(float, float, float);
  const quat &getRotation();
  const mat4 &getMatrix();
  void setZoom(float zoom);
private:
  FreeCamera *m_free_camera;
  ArcballCamera *m_arcball_camera;
  CameraMode m_mode;
  BaseCamera *m_current_camera;
};
}

#endif //GLNEMO2_NEW_CAMERA_H
