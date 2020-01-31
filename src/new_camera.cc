//
// Created by kalterkrieg on 28/01/2020.
//

#include <iostream>
#include "new_camera.h"

using namespace glm;

#define PI 3.14159265359

namespace glnemo {

BaseCamera::BaseCamera() {
  m_rotation = quat();
  m_position = {4, 4, 4};
}

const quat &BaseCamera::getRotation() const {
  return m_rotation;
}

bool BaseCamera::isMatrixClean() const {
  return m_matrix_clean;
}

const mat4 &BaseCamera::getViewMatrix() {
  if (!m_matrix_clean)
    buildMatrix();
  return m_view_matrix;
}

quat BaseCamera::fromAxisAngle(vec3 axis, double angle) {
  quat out;
  //x, y, and z form a normalized vector which is now the axis of rotation.
  out.w = cosf(angle*PI/360);
  out.x = axis.x * sinf(angle*PI/360);
  out.y = axis.y * sinf(angle*PI/360);
  out.z = axis.z * sinf(angle*PI/360);
  return normalize(out);
}

ArcballCamera::ArcballCamera() {
  m_zoom = 4;
}

void ArcballCamera::buildMatrix() {
  m_position = m_rotation * vec3(0, 0, m_zoom);

  mat4 t(1.f);
  t = translate(t, m_position);
  mat4 r = mat4_cast(m_rotation);
  m_view_matrix = t * r;
  m_matrix_clean = true;
}

void ArcballCamera::rotate(float x, float y, float z) {
  if (z != 0) std::cout << z << "\n";
  quat rot_x = fromAxisAngle(m_rotation * vec3(1, 0, 0), -x);
  quat rot_y = fromAxisAngle(m_rotation * vec3(0, 1, 0), -y);
  quat rot_z = fromAxisAngle(normalize(-m_position), z);
  std::cout << rot_z.x << " " << rot_z.y << " " << rot_z.z << " " << rot_z.w << "\n";
  m_rotation = normalize(rot_x * rot_y * rot_z * m_rotation);
  m_matrix_clean = false;
}

float ArcballCamera::getZoom() const {
  return m_zoom;
}

void ArcballCamera::setZoom(float zoom) {
  m_zoom = zoom;
}

void ArcballCamera::reset() {
  m_rotation = quat(1.f, 0, 0, 0);
  m_zoom = 4;
  m_position = {0, 0, m_zoom};
  m_matrix_clean = false;
}

////////////////
void FreeCamera::rotate(float x, float y, float z) {
  m_matrix_clean = false;
}

void FreeCamera::buildMatrix() {

}
////////////////////


NewCamera::NewCamera() {
  m_free_camera = new FreeCamera();
  m_arcball_camera = new ArcballCamera();
  m_current_camera = m_arcball_camera;
  m_mode = CameraMode::arcball;
}

const quat &NewCamera::getRotation() {
  return m_current_camera->getRotation();
}

NewCamera::~NewCamera() {
  delete m_free_camera;
  delete m_arcball_camera;
}

const mat4 &NewCamera::getMatrix() {
  return m_current_camera->getViewMatrix();
}

void NewCamera::reset() {
  m_current_camera->reset();
}

void NewCamera::rotate(float x, float y, float z) {
  m_current_camera->rotate(x, y, z);
}

void NewCamera::setZoom(float zoom) {
  if (m_mode == CameraMode::arcball)
    m_arcball_camera->setZoom(zoom);
}

}