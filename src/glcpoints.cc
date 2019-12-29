//
// Created by kalterkrieg on 25/11/2019.
//

#include <fstream>
#include "glcpoints.h"
#include "globaloptions.h"

namespace glnemo {

/******* GLCPoint ********/

GLCPoint::GLCPoint(std::array<float, 3> coords, float size)
        : m_coords(coords), m_size(size) {
}

const std::array<float, 3> &GLCPoint::getCoords() const {
  return m_coords;
}

const float &GLCPoint::getSize() const {
  return m_size;
}


/******* GLCPointset ********/

void GLCPointset::sendUniforms(int nb_vertices) {
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);

  m_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
  m_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
  m_shader->sendUniformi("nb_vertices", nb_vertices);
  m_shader->sendUniformi("is_filled", m_is_filled);
}

GLCPointset::GLCPointset(CShader *shader, std::string name) :
        m_shader(shader), m_name(name) {
  m_is_shown = true;
  m_is_filled = false;
  m_color = QColor("white");
  m_threshold = 100;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenVertexArrays(1, &m_vao);

}

GLCPointset::~GLCPointset() {
  // for (auto charac_point : m_cpoints) {
  // delete charac_point;
  // }
  // m_cpoints.clear();
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

bool GLCPointset::ready() {
  return m_shader != nullptr;
}

const std::string &GLCPointset::getName() const {
  return m_name;
}

void GLCPointset::addPoint(GLCPoint *point) {
  m_cpoints.insert(point);
}

void GLCPointset::hide() {
  m_is_shown = false;
}

void GLCPointset::show() {
  m_is_shown = true;
}
bool GLCPointset::isShown() {
  return m_is_shown;
}
void GLCPointset::setThreshold(int threshold) {
  m_threshold = threshold < 0 ? 0 : threshold > 100 ? 100 : threshold; //clamp
}

void GLCPointset::setAttributes() {
  GLuint point_center_disk_attrib = glGetAttribLocation(m_shader->getProgramId(), "point_center");
  GLuint radius_disk_attrib = glGetAttribLocation(m_shader->getProgramId(), "radius");
  if (point_center_disk_attrib == -1) {
    std::cerr << "Error occured when getting \"point_center\" attribute (disk shader)\n";
    exit(1);
  }
  if (radius_disk_attrib == -1) {
    std::cerr << "Error occured when getting \"radius\" attribute (disk shader)\n";
    exit(1);
  }
  glEnableVertexAttribArrayARB(point_center_disk_attrib);
  glVertexAttribPointerARB(point_center_disk_attrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
  glVertexBindingDivisor(point_center_disk_attrib, 1);

  glEnableVertexAttribArrayARB(radius_disk_attrib);
  glVertexAttribPointerARB(radius_disk_attrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                           (void *) (3 * sizeof(float)));
  glVertexBindingDivisor(radius_disk_attrib, 1);
}

void GLCPointset::initVboData() {
  m_data.clear();
  // DATA INIT

  //maybe use reserve to preallocate,
  // m_data.reserve(4*m_cpoints.size());
  for (auto point : m_cpoints) {
    m_data.push_back(point->getCoords()[0]);
    m_data.push_back(point->getCoords()[1]);
    m_data.push_back(point->getCoords()[2]);
    m_data.push_back(point->getSize());
  }
  // SEND DATA
  glBindVertexArray(m_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * m_data.size(), m_data.data(), GL_STATIC_DRAW);

  setAttributes();
  glBindVertexArray(0);
}

const glcpointset_t &GLCPointset::getCPoints() const {
  return m_cpoints;
}

void GLCPointset::copyCPoints(GLCPointset *other) {
  m_cpoints = other->getCPoints();
}

void GLCPointset::setFilled(bool filled) {
  m_is_filled = filled;
}

const bool GLCPointset::isFilled() const {
  return m_is_filled;
}

/******* GLCPointDisk ********/
GLCPointsetDisk::GLCPointsetDisk(CShader *shader, std::string name)
        : GLCPointset(shader, name) {
}

void GLCPointsetDisk::display() {

  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_instances = m_cpoints.size() * m_threshold / 100;
//  sendUniforms(m_nb_vertices);
  if (m_is_filled) {
    sendUniforms(m_nb_vertices);
    glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, m_nb_vertices, nb_instances);
  } else {
    sendUniforms(m_nb_vertices*2);
    glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices*2 +2, nb_instances);
  }
  glBindVertexArray(0);
  m_shader->stop();
}

/******* GLCPointSquare ********/
GLCPointsetSquare::GLCPointsetSquare(CShader *shader, std::string name)
        : GLCPointset(shader, name) {
}

void GLCPointsetSquare::display() {

  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_instances = m_cpoints.size() * m_threshold / 100;
  if (m_is_filled) {
    sendUniforms(m_nb_vertices);
    glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, m_nb_vertices, nb_instances);
  } else {
    sendUniforms(m_nb_vertices*2);
    glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices*2 +2, nb_instances);
  }
  glBindVertexArray(0);
  m_shader->stop();
}

/******* GLCPointsetManager ********/


GLCPointsetManager::GLCPointsetManager() {
  // SHADER INIT
  m_nb_sets = 0;
}

void GLCPointsetManager::loadFile(std::string filepath) {
  std::cerr << "Loading json file\n";
  std::ifstream file(filepath);
  json json_data;
  try {
    file >> json_data;
  } catch (json::exception) {
    std::cerr << "Failed to parse file " + filepath << "\n";
    return;
  }

  for (json::iterator it = json_data.begin(); it != json_data.end(); ++it) {
    GLCPointset *pointset;
    CShader *shader;

    std::string str_shape = (*it)["shape"];
    std::string name((*it).value("name", defaultName()));
    if (str_shape == "disk") {
      shader = m_disk_shader;
      pointset = new GLCPointsetDisk(shader, name);
    } else if (str_shape == "square") {
      shader = m_square_shader;
      pointset = new GLCPointsetSquare(shader, name);
    } else {
      std::cerr << "Unrecognized shape : " + str_shape;
      continue;
    }

    for (json::iterator data = (*it)["data"].begin(); data != (*it)["data"].end(); ++data) {
      auto cpoint = new GLCPoint((*data)["coords"], (*data)["radius"]);
      pointset->addPoint(cpoint);
    }
    pointset->initVboData();
    m_pointsets[name] = pointset;
    m_nb_sets++;
  }
}

std::string GLCPointsetManager::defaultName() const {
  return "CPoint set " + std::to_string(m_nb_sets);
}

GLCPointsetManager::~GLCPointsetManager() {
  for (auto cpointset: m_pointsets) {
    for (auto cpoint : cpointset.second->getCPoints())
      delete cpoint;
    delete cpointset.second;
  }
}

void GLCPointsetManager::initShaders() {

  m_disk_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/square.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_disk_shader->init()) {
    delete m_disk_shader;
    std::cerr << "Failed to initialize disk shader\n";
    exit(1);
  }
  m_square_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/square.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_square_shader->init()) {
    delete m_square_shader;
    std::cerr << "Failed to initialize square shader\n";
    exit(1);
  }
}

void GLCPointsetManager::displayAll() {
  for (auto cpointset: m_pointsets)
    cpointset.second->display();
}

void GLCPointsetManager::createNewCPointset() {
  m_pointsets[defaultName()] = new GLCPointsetDisk(m_disk_shader, defaultName());
  m_nb_sets++;
}

void GLCPointsetManager::deleteCPointset(std::string pointset_name) {
  delete m_pointsets[pointset_name];
  m_pointsets.erase(pointset_name);
  m_nb_sets--;
}

void GLCPointsetManager::changePointsetType(std::string pointset_name, std::string new_type) {
  auto *old_pointset = m_pointsets[pointset_name];
  GLCPointset *new_pointset;
  if (new_type == "disk")
    new_pointset = new GLCPointsetDisk(m_disk_shader, pointset_name);
  else if (new_type == "square")
    new_pointset = new GLCPointsetSquare(m_square_shader, pointset_name);
  // else if(new_type == )
  if (new_pointset) {
    new_pointset->copyCPoints(old_pointset);
    new_pointset->initVboData();
    m_pointsets[pointset_name] = new_pointset;
    delete old_pointset;
  }
}

}