//
// Created by kalterkrieg on 25/11/2019.
//

#include <fstream>
#include <array>
#include "glcpoints.h"
#include "globaloptions.h"

namespace glnemo {

int GLCPointset::wwidth = 0;

/******* GLCPoint ********/

GLCPoint::GLCPoint(std::array<float, 3> coords, float size, std::string text)
        : m_coords(coords), m_size(size), m_text(text) {
}

const std::array<float, 3> &GLCPoint::getCoords() const {
  return m_coords;
}

const float &GLCPoint::getSize() const {
  return m_size;
}
const std::string &GLCPoint::getText() const {
  return m_text;
}


/******* GLCPointset ********/

void GLCPointset::sendUniforms() {
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);

  m_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
  m_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
  m_shader->sendUniformXfv("color", 3, 1, m_color.data());
}

GLCPointset::GLCPointset(CShader *shader, std::string name) :
        m_shader(shader), m_name(name), m_color{1,1,1} {
  m_is_shown = true;
  m_is_filled = false;
  m_threshold = 100;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenVertexArrays(1, &m_vao);

}

GLCPointset::~GLCPointset() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

bool GLCPointset::ready() {
  return m_shader != nullptr;
}

const std::string &GLCPointset::getName() const {
  return m_name;
}

void GLCPointset::addPoint(std::array<float, 3> coords, float size, std::string text) {
  GLCPoint *cpoint = new GLCPoint(coords, size, text);
  m_cpoints.insert(cpoint);
  initVboData();
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
    std::cerr << "Error occured when getting \"point_center\" attribute\n";
    exit(1);
  }
  if (radius_disk_attrib == -1) {
    std::cerr << "Error occured when getting \"radius\" attribute\n";
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
  for (GLCPoint *point : m_cpoints) {
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
  initVboData();
}

void GLCPointset::setFilled(bool filled) {
  m_is_filled = filled;
}

const bool GLCPointset::isFilled() const {
  return m_is_filled;
}
CPointsetTypes GLCPointset::getPointsetType() const {
  return m_pointset_type;
}
const int GLCPointset::getThreshold() const {
  return m_threshold;
}
const QColor GLCPointset::getQColor() const {
  return QColor(m_color[0]*255, m_color[1]*255, m_color[2]*255);
}
void GLCPointset::setColor(const QColor &color) {
  m_color = {color.redF(), color.greenF(), color.blueF()};
}
const CPointsetTypes &GLCPointset::getShape() const {
  return m_pointset_type;
}
void GLCPointset::setColor(std::array<float, 3> color) {
  m_color = color;
}
const std::array<float, 3> &GLCPointset::getColor() const {
  return m_color;
}

/******* GLCPointDisk ********/
GLCPointsetDisk::GLCPointsetDisk(CShader *shader, std::string name)
        : GLCPointsetRegularPolygon(shader, name) {
  m_nb_vertices = 100;
  m_pointset_type = CPointsetTypes::disk;
}

GLCPointsetRegularPolygon::GLCPointsetRegularPolygon(CShader *m_shader, std::string name) : GLCPointset(m_shader,
                                                                                                        name) {
}

void GLCPointsetRegularPolygon::display() {

  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_instances = m_cpoints.size() * m_threshold / 100;
  m_shader->sendUniformi("is_filled", m_is_filled);
  sendUniforms();
  if (m_is_filled) {
    m_shader->sendUniformi("nb_vertices", m_nb_vertices);
    glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, m_nb_vertices, nb_instances);
  } else {
    m_shader->sendUniformi("nb_vertices", m_nb_vertices * 2);
    glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices * 2 + 2, nb_instances);
  }
  glBindVertexArray(0);
  m_shader->stop();
}

/******* GLCPointSquare ********/
GLCPointsetSquare::GLCPointsetSquare(CShader *shader, std::string name)
        : GLCPointsetRegularPolygon(shader, name) {
  m_nb_vertices = 4;
  m_pointset_type = CPointsetTypes::square;
}

/******* GLCPointTag ********/
GLCPointsetTag::GLCPointsetTag(CShader *m_shader, std::string name) : GLCPointset(m_shader, name) {
  m_pointset_type = CPointsetTypes::tag;
}

void GLCPointsetTag::display() {
  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_instances = m_cpoints.size() * m_threshold / 100;

  sendUniforms();
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, 3, nb_instances);

  glBindVertexArray(0);
  m_shader->stop();
}

void GLCPointsetTag::sendUniforms() {
  GLCPointset::sendUniforms();

  float tagSizeOnScreen = 50; // InPixels

  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);
  glm::mat4 mviewGLM = glm::make_mat4(mview);
  m_shader->sendUniformXfv("modelviewMatrixInverse", 16, 1, (const float *) glm::value_ptr(glm::inverse(mviewGLM)));
  m_shader->sendUniformf("screen_scale", 2 * tagSizeOnScreen / wwidth);
}


/******* GLCPointsetManager ********/


GLCPointsetManager::GLCPointsetManager() {
  // SHADER INIT
  m_nb_sets = 0;

  shapeToStr[CPointsetTypes::disk] = "disk";
  shapeToStr[CPointsetTypes::square] = "square";
  shapeToStr[CPointsetTypes::tag] = "tag";

  for(auto sts: shapeToStr){
    strToShape[sts.second] = sts.first;
  }
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

    pointset = newPointset(str_shape, name);
    if(!pointset)
      continue;

    for (json::iterator data = (*it)["data"].begin(); data != (*it)["data"].end(); ++data) {
      pointset->addPoint((*data)["coords"], (*data)["radius"], std::string()); //TODO change "radius" to "size"
    }
    m_pointsets[name] = pointset;
    m_nb_sets++;
  }
}

std::string GLCPointsetManager::defaultName() const {
  return "CPoint set " + std::to_string(m_nb_sets);
}

GLCPointsetManager::~GLCPointsetManager() {
  for (auto cpointset: m_pointsets) {
    for (GLCPoint *cpoint : cpointset.second->getCPoints())
      delete cpoint;
    delete cpointset.second;
  }
}

void GLCPointsetManager::initShaders() {

  m_regular_polygon_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/regular_polygon.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_regular_polygon_shader->init()) {
    delete m_regular_polygon_shader;
    std::cerr << "Failed to initialize regular polygon shader\n";
    exit(1);
  }

  m_tag_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_tag_shader->init()) {
    delete m_tag_shader;
    std::cerr << "Failed to initialize tag shader\n";
    exit(1);
  }
}

void GLCPointsetManager::displayAll() {
  for (auto cpointset: m_pointsets)
    cpointset.second->display();
}

void GLCPointsetManager::createNewCPointset() {
  m_pointsets[defaultName()] = new GLCPointsetDisk(m_regular_polygon_shader, defaultName());
  m_nb_sets++;
}

void GLCPointsetManager::deleteCPointset(std::string pointset_name) {
  delete m_pointsets[pointset_name];
  m_pointsets.erase(pointset_name);
  m_nb_sets--;
}
void GLCPointsetManager::changePointsetType(std::string pointset_name, std::string new_type) {
  GLCPointset *old_pointset = m_pointsets[pointset_name];
  GLCPointset *new_pointset = newPointset(new_type, pointset_name);
  if (new_pointset) {
    new_pointset->copyCPoints(old_pointset);
    m_pointsets[pointset_name] = new_pointset;
    delete old_pointset;
  }
}
void GLCPointsetManager::setW(int w) {
  GLCPointset::wwidth = w;
}

void GLCPointsetManager::saveToFile(std::string file_path) {
  std::ofstream outfile (file_path);
  json final_obj;
  for(auto pair: *this){
    GLCPointset *set = pair.second;
    json data_array = json::array();
    for(GLCPoint *cpoint : set->getCPoints())
      data_array.push_back({{"coords", cpoint->getCoords()},
                                {"radius", cpoint->getSize()}});
    json current_dict = {{"name", set->getName()},
                         {"shape", shapeToStr[set->getShape()]},
                         {"color", set->getColor()},
                         {"data", data_array}};

    final_obj.push_back(current_dict);
  }
  outfile << final_obj;
}

GLCPointset *GLCPointsetManager::newPointset(std::string str_shape, std::string name) {
  if (str_shape == "disk") {
    return new GLCPointsetDisk(m_regular_polygon_shader, name);
  } else if (str_shape == "square") {
    return new GLCPointsetSquare(m_regular_polygon_shader, name);
  } else if (str_shape == "tag") {
    return new GLCPointsetTag(m_tag_shader, name);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}
}
