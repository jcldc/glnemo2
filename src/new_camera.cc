//
// Created by kalterkrieg on 28/01/2020.
//

#include <iostream>
#include "new_camera.h"

using namespace glm;

#define PI 3.14159265359

namespace glnemo {

BaseCamera::BaseCamera() {
  m_position = {0, 0, 4};
  m_orientation = fromAxisAngle(-m_position, 0);
  m_matrix_clean = false;
}

const quat &BaseCamera::getOrientation() const {
  return m_orientation;
}

bool BaseCamera::isMatrixClean() const {
  return m_matrix_clean;
}

const mat4 &BaseCamera::getViewMatrix() {
  if (!m_matrix_clean)
    buildMatrices();
  return m_view_matrix;
}

const mat4 &BaseCamera::getOrientationMatrix() {
  if (!m_matrix_clean)
    buildMatrices();
  return m_orientation_matrix;
}

void BaseCamera::buildMatrices() {
  mat4 t(1.f);
  t = translate(t, m_position);
  mat4 r = mat4_cast(m_orientation);
  m_view_matrix = t * r;
  m_orientation_matrix = r;
  m_matrix_clean = true;
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

const vec3 &BaseCamera::getPosition() const {
  return m_position;
}

/********* ArcBall Camera ************/
ArcballCamera::ArcballCamera() {
  m_zoom = 4;
}

void ArcballCamera::rotate(float x, float y, float z) {
  quat rot_x = fromAxisAngle(m_orientation * vec3(1, 0, 0), -x);
  quat rot_y = fromAxisAngle(m_orientation * vec3(0, 1, 0), -y);
  quat rot_z = fromAxisAngle(normalize(-m_position), z);
  m_orientation = normalize(rot_x * rot_y * rot_z * m_orientation);
  m_position = m_orientation * vec3(0, 0, m_zoom);
  m_matrix_clean = false;
}

float ArcballCamera::getZoom() const {
  return m_zoom;
}

void ArcballCamera::setZoom(float zoom) {
  m_zoom = zoom;
}

float ArcballCamera::increaseZoom() {
  m_zoom *= 1.1;
  return m_zoom;
}

float ArcballCamera::decreaseZoom() {
  m_zoom *= 0.9;
  return m_zoom;
}

void ArcballCamera::reset() {
  m_orientation = quat(1.f, 0, 0, 0);
  m_zoom = 4;
  m_position = {0, 0, m_zoom};
  m_matrix_clean = false;
}

/********* Free Camera ************/
void FreeCamera::rotate(float x, float y, float z) {
  quat rot_x = fromAxisAngle(m_orientation * vec3(1, 0, 0), -x);
  quat rot_y = fromAxisAngle(m_orientation * vec3(0, 1, 0), -y);
  quat rot_z = fromAxisAngle(m_orientation * vec3(0, 0, 1), -z);
  m_orientation = normalize(rot_x * rot_y * rot_z * m_orientation);
  m_matrix_clean = false;
}

void FreeCamera::setOrientation(quat orientation) {
  m_orientation = orientation;
  m_matrix_clean = false;
}

void FreeCamera::moveForward(float dt) {
    m_position += m_orientation*vec3(0, 0, 1) / (float)(1 / (m_speed * dt / 30.0));
    m_matrix_clean = false;
}

void FreeCamera::moveRight(float dt) {
    m_position += m_orientation*vec3(1, 0, 0) / (float)(1 / (m_speed * dt / 30.0));
    m_matrix_clean = false;
}

void FreeCamera::moveLeft(float dt) {
    m_position += m_orientation*vec3(-1, 0, 0) / (float)(1 / (m_speed * dt / 30.0));
    m_matrix_clean = false;
}

void FreeCamera::moveBackward(float dt) {
    m_position += m_orientation*vec3(0, 0, -1) / (float)(1 / (m_speed * dt / 30.0));
    m_matrix_clean = false;
}

void FreeCamera::reset() {
  m_orientation = quat(1.f, 0, 0, 0);
  m_position = {0, 0, 4};
  m_matrix_clean = false;
}


/********* Camera ************/
NewCamera::NewCamera() {
  m_free_camera = new FreeCamera();
  m_arcball_camera = new ArcballCamera();
  m_current_camera = m_arcball_camera;
  m_mode = CameraMode::arcball;
}

const quat &NewCamera::getOrientation() {
  return m_current_camera->getOrientation();
}

NewCamera::~NewCamera() {
  delete m_free_camera;
  delete m_arcball_camera;
}

const mat4 &NewCamera::getViewMatrix() {
  return m_current_camera->getViewMatrix();
}

// float NewCamera::increaseSpeed(){
//   float new_speed = *1.1;
// }
// float NewCamera::decreaseSpeed(){
//   return val*0.9;

const mat4 &NewCamera::getOrientationMatrix() {
  return m_current_camera->getOrientationMatrix();
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

const CameraMode &NewCamera::getCameraMode() const {
  return m_mode;
}

// }
float NewCamera::increaseZoom(){
  return m_arcball_camera->increaseZoom();
}

float NewCamera::decreaseZoom(){
  return m_arcball_camera->decreaseZoom();
}

void NewCamera::toggleCameraMode() {
  if(m_mode == CameraMode::arcball){
    m_current_camera = m_free_camera;
    m_mode = CameraMode::free;
  } else {
    m_current_camera = m_arcball_camera;
    m_mode = CameraMode::arcball;
  }
}

void NewCamera::moveForward(float dt) {
  if(m_mode == CameraMode::free)
    m_free_camera->moveForward(dt);
}

void NewCamera::moveBackward(float dt) {
  if(m_mode == CameraMode::free)
    m_free_camera->moveBackward(dt);
}

void NewCamera::moveLeft(float dt) {
  if(m_mode == CameraMode::free)
    m_free_camera->moveLeft(dt);
}

void NewCamera::moveRight(float dt) {
  if(m_mode == CameraMode::free)
    m_free_camera->moveRight(dt);
}

const vec3 &NewCamera::getPosition() const {
  return m_current_camera->getPosition();
}

void NewCamera::setOrientation(quat orientation) {
  if(m_mode == CameraMode::free)
    m_free_camera->setOrientation(orientation);
}

void NewCamera::setOrientation(CubemapFace face) {
  if(m_mode == CameraMode::free){
    switch (face){
      case CubemapFace::top:
        m_free_camera->setOrientation(quat(vec3(PI/2, 0, 0)));
        break;
      case CubemapFace::bottom:
        m_free_camera->setOrientation(quat(vec3(-PI/2, 0, 0)));
        break;
      case CubemapFace::front:
        m_free_camera->setOrientation(quat(vec3(0, 0, 0)));
        break;
      case CubemapFace::back:
        m_free_camera->setOrientation(quat(vec3(0, PI, 0)));
        break;
      case CubemapFace::left:
        m_free_camera->setOrientation(quat(vec3(0,  PI/2, 0)));
        break;
      case CubemapFace::right:
        m_free_camera->setOrientation(quat(vec3(0, -PI/2, 0)));
        break;
    }
  }
}

}