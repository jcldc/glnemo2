//
// Created by kalterkrieg on 25/11/2019.
//

#include <fstream>
#include <array>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QImage>
#include "glcpoints.h"
#include "globaloptions.h"

namespace glnemo {

std::map<CPointsetShapes, std::string> CPointset::shapeToStr;
std::map<std::string, CPointsetShapes> CPointset::strToShape;

CShader *CPointsetRegularPolygon::shader = nullptr;
CShader *CPointsetTag::shader = nullptr;
CShader *CPointsetSphere::shader = nullptr;

int GLCPoint::next_id = 0;
int CPointset::wheight = 0;
int CPointset::wwidth = 0;

/******* GLCPoint ********/

GLCPoint::GLCPoint(std::array<float, 3> coords, float size, const std::string& text)
        : m_coords(coords), m_size(size) {
  m_id = next_id;
  next_id++;
  m_is_selected = false;

  if (text.empty())
    m_name = "CPoint " + std::to_string(m_id);
  else
    m_name = text;
}

const std::array<float, 3> &GLCPoint::getCoords() const {
  return m_coords;
}

const float &GLCPoint::getSize() const {
  return m_size;
}
const std::string &GLCPoint::getName() const {
  return m_name;
}
const int &GLCPoint::getId() const {
  return m_id;
}
bool GLCPoint::isSelected() const {
  return m_is_selected;
}
void GLCPoint::setCoords(std::array<float, 3> coords) {
  m_coords = coords;
}
void GLCPoint::setSize(float size) {
  m_size = size;
}
void GLCPoint::setName(const std::string &name) {
  m_name = name;
}
void GLCPoint::select() {
  m_is_selected = true;
}
void GLCPoint::unselect() {
  m_is_selected = false;
}


/******* GLCPointset ********/
CPointTextRenderer *CPointset::text_renderer = nullptr;

void CPointset::sendUniforms() {
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);

  m_shader->sendUniformXfv("proj_matrix", 16, 1, &proj[0]);
  m_shader->sendUniformXfv("model_view_matrix", 16, 1, &mview[0]);
  m_shader->sendUniformXfv("color", 3, 1, m_color.data());
  m_shader->sendUniformXiv("screen_dims", 2, 1, std::array<int, 2>({wwidth, wheight}).data());
}

CPointset::CPointset(CShader *shader, const std::string &name) :
        m_shader(shader), m_name(name) {
  m_color = {0.2, 0.5, 0.8};
  m_is_visible = true;
  m_is_name_visible = false;
  m_threshold = 100;
  m_fill_ratio = 0.1;
  m_name_angle = 45;
  m_name_offset = 1;
  m_name_size_factor = 1;
  m_nb_sphere_sections = 12;
  m_nb_selected = 0;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenBuffersARB(1, &m_selected_vbo);
  glGenVertexArrays(1, &m_vao);
  glGenVertexArrays(1, &m_selected_vao);

}

CPointset::CPointset(CShader *shader, const CPointset &other) {
  m_shader = shader;
  m_name = other.m_name;
  m_color = other.m_color;
  m_is_visible = other.m_is_visible;
  m_is_name_visible = other.m_is_name_visible;
  m_threshold = other.m_threshold;
  m_fill_ratio = other.m_fill_ratio;
  m_name_angle = other.m_name_angle;
  m_name_offset = other.m_name_offset;
  m_name_size_factor = other.m_name_size_factor;
  m_shape = other.m_shape;
  m_nb_sphere_sections = other.m_nb_sphere_sections;
  m_nb_selected = other.m_nb_selected;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenBuffersARB(1, &m_selected_vbo);
  glGenVertexArrays(1, &m_vao);
  glGenVertexArrays(1, &m_selected_vao);
  copyCPoints(other);
}

CPointset::~CPointset() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

bool CPointset::ready() {
  return m_shader != nullptr;
}

const std::string &CPointset::getName() const {
  return m_name;
}

GLCPoint *CPointset::addPoint(std::array<float, 3> coords, float size, const std::string &text) {
  auto cpoint = new GLCPoint(coords, size, text);
  m_cpoints[cpoint->getId()] = cpoint;
  genVboData();
  return cpoint;
}

void CPointset::addPoints(const std::vector<GLCPointData>& cpoint_data_v) { // return vector maybe
  for (const GLCPointData& cpoint_data : cpoint_data_v) {
    auto cpoint = new GLCPoint(cpoint_data.coords, cpoint_data.size, cpoint_data.text);
    m_cpoints[cpoint->getId()] = cpoint;
  }
  genVboData();
}

void CPointset::setVisible(bool visible) {
  m_is_visible = visible;
}

bool CPointset::isVisible() {
  return m_is_visible;
}

void CPointset::setThreshold(int threshold) {
  m_threshold = threshold < 0 ? 0 : threshold > 100 ? 100 : threshold; //clamp
}

void CPointset::setAttributes() {
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

void CPointset::genVboData() {
  // DATA INIT
  std::vector<float> data, selected_data;
  //maybe use reserve to preallocate,
  // m_data.reserve(4*m_cpoints.size());

  std::vector<GLCPoint *> cpoints_v;
  for (auto & m_cpoint : m_cpoints) {
    cpoints_v.push_back(m_cpoint.second);
  }
  std::sort(cpoints_v.begin(), cpoints_v.end(), [](GLCPoint *a, GLCPoint *b) {
    return a->getSize() < b->getSize();
  });

  for (auto point : cpoints_v) {
    data.push_back(point->getCoords()[0]);
    data.push_back(point->getCoords()[1]);
    data.push_back(point->getCoords()[2]);
    data.push_back(point->getSize());
    if (point->isSelected()) {
      selected_data.push_back(point->getCoords()[0]);
      selected_data.push_back(point->getCoords()[1]);
      selected_data.push_back(point->getCoords()[2]);
      selected_data.push_back(point->getSize());
    }
  }
  // SEND DATA
  glBindVertexArray(m_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
  setAttributes(); // needed only once at init ?
  glBindVertexArray(m_selected_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_selected_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * selected_data.size(), selected_data.data(), GL_STATIC_DRAW);
  setAttributes();
  glBindVertexArray(0);
}

const glcpointmap_t &CPointset::getCPoints() const {
  return m_cpoints;
}

void CPointset::copyCPoints(const CPointset &other) {
  m_cpoints = other.getCPoints();
  genVboData();
}
CPointsetShapes CPointset::getPointsetShape() const {
  return m_shape;
}
int CPointset::getThreshold() const {
  return m_threshold;
}
QColor CPointset::getQColor() const {
  return QColor(m_color[0] * 255, m_color[1] * 255, m_color[2] * 255);
}
void CPointset::setColor(const QColor &color) {
  m_color = {static_cast<float>(color.redF()), static_cast<float>(color.greenF()), static_cast<float>(color.blueF())};
}
const CPointsetShapes &CPointset::getShape() const {
  return m_shape;
}
void CPointset::setColor(std::array<float, 3> color) {
  m_color = color;
}
const std::array<float, 3> &CPointset::getColor() const {
  return m_color;
}
void CPointset::setName(const std::string &new_name) {
  m_name = new_name;
}
void CPointset::deletePoint(int id) {
  GLCPoint *cpoint = m_cpoints[id];
  m_cpoints.erase(id);
  delete cpoint;
  genVboData();
}
void CPointset::setCpointSize(int id, float size) {
  m_cpoints.at(id)->setSize(size);
  genVboData();
}
void CPointset::setCpointCoords(int id, std::array<float, 3> coords) {
  m_cpoints.at(id)->setCoords(coords);
  genVboData();
}
void CPointset::setCpointCoordsX(int id, float x) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({x, cpoint->getCoords()[1], cpoint->getCoords()[2]});
  genVboData();
}
void CPointset::setCpointCoordsY(int id, float y) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({cpoint->getCoords()[0], y, cpoint->getCoords()[2]});
  genVboData();
}
void CPointset::setCpointCoordsZ(int id, float z) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({cpoint->getCoords()[0], cpoint->getCoords()[1], z});
  genVboData();
}
void CPointset::setCpointText(int id, const std::string &text) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setName(text);
}
void CPointset::displayText() {
  text_renderer->renderText(this);
}
void CPointset::setNameVisible(bool visible) {
  m_is_name_visible = visible;
}
bool CPointset::isNameVisible() {
  return m_is_name_visible;
}
void CPointset::setFillratio(float fill_ratio) {
  m_fill_ratio = fill_ratio < 0 ? 0 : fill_ratio > 1 ? 1 : fill_ratio; //clamp
}
float CPointset::getFillratio() const {
  return m_fill_ratio;
}
void CPointset::setNameSizeFactor(float name_size_factor) {
  m_name_size_factor = name_size_factor;
}
float CPointset::getNameSizeFactor() const {
  return m_name_size_factor;
}
void CPointset::setNameOffset(float name_offset) {
  m_name_offset = name_offset;
}
float CPointset::getNameOffset() const {
  return m_name_offset;
}
void CPointset::setNameAngle(int angle) {
  m_name_angle = angle % 360;
}
int CPointset::getNameAngle() const {
  return m_name_angle;
}
void CPointset::setNbSphereSections(int nb_sections) {
  m_nb_sphere_sections = nb_sections;
}
int CPointset::getNbSphereSections() const {
  return m_nb_sphere_sections;
}
json CPointset::toJson() {
  CPointsetSphere default_set("default");

  json cpoint_data = json::array();
  for (auto cpoint_pair : m_cpoints) {
    GLCPoint *cpoint = cpoint_pair.second;
    cpoint_data.push_back({{"coords", cpoint->getCoords()},
                           {"size",   cpoint->getSize()},
                           {"name",   cpoint->getName()}});
  }
  json cpointset_json = {{"name",       m_name},
                         {"shape",      shapeToStr[m_shape]},
                         {"data",       cpoint_data}}; //show name

  // serialize only if different than default value

  if (m_is_visible != default_set.isVisible())
    cpointset_json["is_visible"] = m_is_visible;
  if (m_color != default_set.getColor())
    cpointset_json["color"] = m_color;
  if (m_fill_ratio != default_set.getFillratio())
    cpointset_json["fill_ratio"] = m_fill_ratio;
  if (m_name_size_factor != default_set.getNameSizeFactor())
    cpointset_json["name_size_factor"] = m_name_size_factor;
  if (m_name_offset != default_set.getNameOffset())
    cpointset_json["name_offset"] = m_name_offset;
  if (m_nb_sphere_sections != default_set.getNbSphereSections())
    cpointset_json["nb_sphere_sections"] = m_nb_sphere_sections;
  if (m_name_angle != default_set.getNameAngle())
    cpointset_json["name_angle"] = m_name_angle;
  if (m_is_name_visible != default_set.isNameVisible())
    cpointset_json["is_name_visible"] = m_is_name_visible;

  return cpointset_json;
}
void CPointset::fromJson(json j) {

  // set optional fields
  m_color = j.value("color", m_color);
  m_is_visible = j.value("is_visible", m_is_visible);
  m_nb_sphere_sections = j.value("nb_sphere_sections", m_nb_sphere_sections);
  m_name_offset = j.value("name_offset", m_name_offset);
  m_fill_ratio = j.value("fill_ratio", m_fill_ratio);
  m_name_size_factor = j.value("name_size_factor", m_name_size_factor);
  m_name_angle = j.value("name_angle", m_name_angle);
  m_is_name_visible = j.value("is_name_visible", m_is_name_visible);

  std::vector<GLCPointData> cpoint_data_v;
  auto data = j["data"];
  cpoint_data_v.resize(data.size());
  for (std::size_t i = 0; i < data.size(); ++i) {
    cpoint_data_v[i] = {data[i]["coords"], data[i]["size"],
                        data[i].value("name", std::string())};
  }
  addPoints(cpoint_data_v);
}

void CPointset::select() {
  for (auto cpoint_pair: m_cpoints)
    cpoint_pair.second->select();
  m_nb_selected = m_cpoints.size();
  genVboData();
}

void CPointset::unselect() {
  for (auto cpoint_pair: m_cpoints)
    cpoint_pair.second->unselect();
  m_nb_selected = 0;
  genVboData();
}

void CPointset::selectCPoint(int id) {
  m_cpoints.at(id)->select();
  m_nb_selected++;
  genVboData();
}

void CPointset::unselectCPoint(int id) {
  m_cpoints.at(id)->unselect();
  m_nb_selected--;
  genVboData();
}

/******* GLCPointDisk ********/
CPointsetDisk::CPointsetDisk(const std::string &name)
        : CPointsetRegularPolygon(name) {
  m_nb_vertices = 100;
  m_shape = CPointsetShapes::disk;
}
CPointsetDisk::CPointsetDisk(const CPointset &other) : CPointsetRegularPolygon(other) {
  m_nb_vertices = 100;
  m_shape = CPointsetShapes::disk;
}

CPointsetRegularPolygon::CPointsetRegularPolygon(const std::string &name) : CPointset(shader, name) {
}

CPointsetRegularPolygon::CPointsetRegularPolygon(const CPointset &other) : CPointset(shader, other) {
}

void CPointsetRegularPolygon::sendUniforms() {
  CPointset::sendUniforms();

  m_shader->sendUniformf("fill_ratio", m_fill_ratio);
}
void CPointsetRegularPolygon::display() {
  int nb_objects = m_cpoints.size() * m_threshold / 100;
  m_shader->start();
  glBindVertexArray(m_vao);
  sendUniforms();
  m_shader->sendUniformi("second_pass", false);
  m_shader->sendUniformi("nb_vertices", m_nb_vertices * 2);
  glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices * 2 + 2, nb_objects);
  glBindVertexArray(0);
  m_shader->stop();


  m_shader->start();
  glBindVertexArray(m_selected_vao);
  m_shader->sendUniformi("second_pass", true);
  sendUniforms();
  m_shader->sendUniformi("nb_vertices", m_nb_vertices * 2);
  glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices * 2 + 2, nb_objects);
  glBindVertexArray(0);
  m_shader->stop();

}

/******* GLCPointSquare ********/
CPointsetSquare::CPointsetSquare(const std::string &name)
        : CPointsetRegularPolygon(name) {
  m_nb_vertices = 4;
  m_shape = CPointsetShapes::square;
}
CPointsetSquare::CPointsetSquare(const CPointset &other) : CPointsetRegularPolygon(other) {
  m_nb_vertices = 4;
  m_shape = CPointsetShapes::square;
}

/******* GLCPointTag ********/
CPointsetTag::CPointsetTag(const std::string &name) : CPointset(shader, name) {
  m_shape = CPointsetShapes::tag;
}

CPointsetTag::CPointsetTag(const CPointset &other) : CPointset(shader, other) {
  m_shape = CPointsetShapes::tag;
}

void CPointsetTag::display() {
  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_objects = m_cpoints.size() * m_threshold / 100;

  sendUniforms();
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, 3, nb_objects);

  glBindVertexArray(0);
  m_shader->stop();
}
void CPointsetTag::sendUniforms() {
  CPointset::sendUniforms();

//  float tagSizeOnScreen = 50; // InPixels
//
//  GLfloat mview[16];
//  glGetFloatv(GL_MODELVIEW_MATRIX, mview);
//  glm::mat4 mviewGLM = glm::make_mat4(mview);
//  m_shader->sendUniformXfv("model_view_matrixInverse", 16, 1, (const float *) glm::value_ptr(glm::inverse(mviewGLM)));
//  m_shader->sendUniformf("screen_scale", 2 * tagSizeOnScreen / wwidth);
}

/******* GLCPointSphere ********/
CPointsetSphere::CPointsetSphere(const std::string &name) : CPointset(shader, name) {
  m_shape = CPointsetShapes::sphere;
}

CPointsetSphere::CPointsetSphere(const CPointset &other) : CPointset(shader, other) {
  m_shape = CPointsetShapes::sphere;
}

//void GLCPointsetSphere::setAttributes() {
//  GLuint point_center_disk_attrib = glGetAttribLocation(m_shader->getProgramId(), "point_center");
//  GLuint radius_disk_attrib = glGetAttribLocation(m_shader->getProgramId(), "radius");
//  if (point_center_disk_attrib == -1) {
//    std::cerr << "Error occured when getting \"point_center\" attribute\n";
//    exit(1);
//  }
//  if (radius_disk_attrib == -1) {
//    std::cerr << "Error occured when getting \"radius\" attribute\n";
//    exit(1);
//  }
//  glEnableVertexAttribArrayARB(point_center_disk_attrib);
//  glVertexAttribPointerARB(point_center_disk_attrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
//  glVertexBindingDivisor(point_center_disk_attrib, nb_vertex_per_sphere);
//
//  glEnableVertexAttribArrayARB(radius_disk_attrib);
//  glVertexAttribPointerARB(radius_disk_attrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
//                           (void *) (3 * sizeof(float)));
//  glVertexBindingDivisor(radius_disk_attrib, nb_vertex_per_sphere);
//}

void CPointsetSphere::display() {

  int nb_objects = m_cpoints.size() * m_threshold / 100;
  int nb_vertex_per_sphere = m_nb_sphere_sections * m_nb_sphere_sections + m_nb_sphere_sections;

  m_shader->start();
  glBindVertexArray(m_vao);
  sendUniforms();
  m_shader->sendUniformi("second_pass", false);
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, nb_vertex_per_sphere, nb_objects);

  glBindVertexArray(m_selected_vao);
  glEnable(GL_STENCIL_TEST);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_FALSE);
  glStencilFunc(GL_NEVER, 1, 0xFF);
  glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
  glStencilMask(0xFF);
  glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF
  m_shader->sendUniformi("second_pass", false);
  glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, nb_vertex_per_sphere, m_nb_selected);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(0x00);
  glStencilFunc(GL_EQUAL, 0, 0xFF);
  m_shader->sendUniformi("second_pass", true);
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, nb_vertex_per_sphere, m_nb_selected);
  glDisable(GL_STENCIL_TEST);
  glBindVertexArray(0);

  m_shader->stop();


}
void CPointsetSphere::sendUniforms() {
  CPointset::sendUniforms();
  m_shader->sendUniformi("subdivisions", m_nb_sphere_sections);
}

/******* GLCPointsetManager ********/


CPointsetManager::CPointsetManager() {
  // SHADER INIT
  m_nb_sets = 0;

  CPointset::shapeToStr[CPointsetShapes::disk] = "disk";
  CPointset::shapeToStr[CPointsetShapes::square] = "square";
  CPointset::shapeToStr[CPointsetShapes::tag] = "tag";
  CPointset::shapeToStr[CPointsetShapes::sphere] = "sphere";

  for (const auto& sts: CPointset::shapeToStr) {
    CPointset::strToShape[sts.second] = sts.first;
  }
}

int CPointsetManager::loadFile(const std::string &filepath) {
  std::cerr << "Loading json file\n";
  std::ifstream file(filepath);
  json json_data;
  try {
    file >> json_data;

    for (auto & it : json_data) {
      CPointset *pointset;
      CShader *shader;
      std::string str_shape = it.value("shape", defaultShape());
      std::string name(it.value("name", defaultName()));
      bool name_too_long = false;
      while (m_pointsets[name])
        if (name.size() < 50)
          name += " duplicate";
        else {
          name_too_long = true;
          break;
        }

      if (name_too_long)
        continue;

      pointset = newPointset(str_shape, name);
      if (!pointset)
        continue;

      pointset->fromJson(it);
      m_pointsets[name] = pointset;
      m_nb_sets++;
    }
  } catch (json::exception&) {
    return -1;
  }
  return 0;
}

std::string CPointsetManager::defaultName() const {
  return "CPoint set " + std::to_string(m_nb_sets);
}

CPointsetManager::~CPointsetManager() {
  for (const auto& cpointset_pair: m_pointsets) {
    for (auto cpoint_pair : cpointset_pair.second->getCPoints())
      delete cpoint_pair.second;
    delete cpointset_pair.second;
  }
  delete CPointset::text_renderer;
}

void CPointsetManager::initShaders(bool glsl_130) {
  std::string glsl_version;
  if(glsl_130)
    glsl_version = "130";
  else
    glsl_version = "120";

  std::string shader_dir = "/shaders/cpoints_" + glsl_version;

  CPointset::text_renderer = new CPointTextRenderer();
  CPointset::text_renderer->init(shader_dir);

  CPointsetRegularPolygon::shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/regular_polygon.vert",
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/characteristic.frag");
  if (!CPointsetRegularPolygon::shader->init()) {
    delete CPointsetRegularPolygon::shader;
    std::cerr << "Failed to initialize regular_polygon shader\n";
    exit(1);
  }

  CPointsetTag::shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/tag_shape.vert",
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/characteristic.frag");
  if (!CPointsetTag::shader->init()) {
    delete CPointsetTag::shader;
    std::cerr << "Failed to initialize tag shape shader\n";
    exit(1);
  }

  CPointsetSphere::shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/sphere.vert",
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/characteristic.frag");
  if (!CPointsetSphere::shader->init()) {
    delete CPointsetSphere::shader;
    std::cerr << "Failed to initialize sphere shader\n";
    exit(1);
  }
}

void CPointsetManager::displayAll() {
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  for (const auto& cpointset_pair: m_pointsets) {
    CPointset *cpointset = cpointset_pair.second;
    if (cpointset->ready() && cpointset->isVisible()) {
      cpointset->display();
    }
  }
  glDisable(GL_DEPTH_TEST);
  for (const auto& cpointset_pair: m_pointsets) {
    CPointset *cpointset = cpointset_pair.second;
    if (cpointset->isNameVisible())
      cpointset->displayText();
  }
}

CPointset *CPointsetManager::createNewCPointset() {
  auto *pointset = new CPointsetDisk(defaultName());
  m_pointsets[defaultName()] = pointset;
  m_nb_sets++;
  return pointset;
}
void CPointsetManager::deleteCPointset(const std::string &pointset_name) {
  auto it = m_pointsets.find(pointset_name);
  m_pointsets.erase(it);
  m_nb_sets--;
}
CPointset *CPointsetManager::changePointsetShape(CPointset *pointset, const std::string &new_shape) {
  CPointset *new_pointset = newPointset(new_shape, *pointset);
  if (new_pointset) {
    m_pointsets[pointset->getName()] = new_pointset;
    delete pointset;
    return new_pointset;
  }
  return nullptr;
}

void CPointsetManager::setScreenDim(int wwidth, int wheight) {
  CPointset::wwidth = wwidth;
  CPointset::wheight = wheight;
}

void CPointsetManager::saveToFile(const std::string &file_path) {
  std::ofstream outfile(file_path);
  json cpointset_json;
  for (const auto& pair: m_pointsets) {
    CPointset *set = pair.second;
    cpointset_json.push_back(set->toJson());
  }
  outfile << cpointset_json;
}

CPointset *CPointsetManager::newPointset(const std::string &str_shape, const std::string &name) {
  if (str_shape == "disk") {
    return new CPointsetDisk(name);
  } else if (str_shape == "square") {
    return new CPointsetSquare(name);
  } else if (str_shape == "tag") {
    return new CPointsetTag(name);
  } else if (str_shape == "sphere") {
    return new CPointsetSphere(name);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}

CPointset *CPointsetManager::newPointset(const std::string &str_shape, const CPointset &other) {
  if (str_shape == "disk") {
    return new CPointsetDisk(other);
  } else if (str_shape == "square") {
    return new CPointsetSquare(other);
  } else if (str_shape == "tag") {
    return new CPointsetTag(other);
  } else if (str_shape == "sphere") {
    return new CPointsetSphere(other);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}
void CPointsetManager::setPointsetName(const std::string &old_name, const std::string &new_name) {
  if (old_name != new_name) {
    const iterator it = m_pointsets.find(old_name);
    if (it != m_pointsets.end()) {
      m_pointsets[new_name] = it->second;
      m_pointsets[new_name]->setName(new_name);
      m_pointsets.erase(it);
    }
  }
}

void CPointsetManager::deleteCPoint(const std::string &pointset_name, int cpoint_id) {
  m_pointsets.at(pointset_name)->deletePoint(cpoint_id);
}

void CPointsetManager::unselectAll() {
  for (auto cpointset_pair: m_pointsets)
    cpointset_pair.second->unselect();
}
std::string CPointsetManager::defaultShape() {
  return "disk";
}

void CPointTextRenderer::init(const std::string &shader_dir) {

  // initialize shader
  m_text_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + shader_dir + "/text.vert",

          GlobalOptions::RESPATH.toStdString() + shader_dir + "/text.frag");
  if (!m_text_shader->init()) {
    delete m_text_shader;
    std::cerr << "Failed to initialize tag text shader\n";
    exit(1);
  }

  // generate buffers
  glGenVertexArrays(1, &m_text_vao);
  glGenBuffers(1, &m_text_vbo);
  glBindVertexArray(m_text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glPixelStorei(GL_UNPACK_ALIGNMENT,
                1); // Disable byte-alignment restriction, maybe not needed since we have power of two texture

  // Load json description file
  std::cout << "Loading font description json file\n";
  QFile font_description_file(GlobalOptions::RESPATH + "/fonts/dejavu_sans_mono_typeface.json");
  if (!font_description_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cout << "Could not open font description file" << std::endl;
    exit(1);
  }
  QTextStream font_description_stream(&font_description_file);
  json json_font;
  try {
    json_font = json::parse(font_description_stream.readAll().toStdString());
  } catch (json::exception&) {
    std::cerr << "Failed to parse file " + font_description_file.fileName().toStdString() << "\n";
    exit(1);
  }

  int texture_width = json_font["config"]["textureWidth"];
  int texture_height = json_font["config"]["textureHeight"];

  // Load texture
  QImage texture_img(GlobalOptions::RESPATH + "/fonts/dejavu_sans_mono_typeface.png");

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGBA,
          texture_width,
          texture_height,
          0,
          GL_RGBA,
          GL_UNSIGNED_BYTE,
          texture_img.bits()
  );
  // Set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Generate characters from json file
  for (json::iterator symbol_it = json_font["symbols"].begin(); symbol_it != json_font["symbols"].end(); ++symbol_it) {
    json::value_type &symbol = *symbol_it;

    float norm_width = symbol["width"].get<float>() / texture_width;
    float norm_height = symbol["height"].get<float>() / texture_height;
    float norm_x = symbol["x"].get<float>() / texture_width;
    float norm_y = symbol["y"].get<float>() / texture_height;

    Character character = {
            std::array<std::array<float, 2>, 4>({
                    std::array<float, 2>({norm_x, norm_y}),
                    std::array<float, 2>({norm_x, norm_y + norm_height}),
                    std::array<float, 2>({norm_x + norm_width, norm_y + norm_height}),
                    std::array<float, 2>({norm_x + norm_width, norm_y})}
            ),
            glm::ivec2(symbol["width"], symbol["height"]),
            glm::ivec2(0, symbol["yoffset"]),
            symbol["xadvance"]
    };
    characters.insert(std::pair<char, Character>(symbol["id"].get<char>(), character));
  }

}

void CPointTextRenderer::renderText(CPointset *pointset) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Iterate through all cpoints
  for (auto cpoint_pair: pointset->getCPoints()) {
    GLCPoint *cpoint = cpoint_pair.second;
    float xpos = 0, ypos = 0;
    float scale = .01;
    std::string text = cpoint->getName();

    m_text_shader->start();

    GLfloat proj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    GLfloat mview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mview);

    m_text_shader->sendUniformXfv("proj_matrix", 16, 1, &proj[0]);
    m_text_shader->sendUniformXfv("model_view_matrix", 16, 1, &mview[0]);

    m_text_shader->sendUniformXfv("color", 3, 1, pointset->getColor().data());
    m_text_shader->sendUniformXfv("point_center", 3, 1, cpoint->getCoords().data());

    float offset = cpoint->getSize() * pointset->getNameOffset();
    m_text_shader->sendUniformf("offset", offset);
    int angle = pointset->getNameAngle();
    m_text_shader->sendUniformi("angle", angle);
    scale *= cpoint->getSize() * pointset->getNameSizeFactor();
//
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindVertexArray(m_text_vao);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Iterate through all characters
    std::string::const_iterator c;

    Character x = characters['x'];

    float baseline = -(x.Size.y + x.Bearing.y) * scale;

    for (c = text.begin(); c != text.end(); c++) {
      Character ch = characters[*c];

      ypos = -(ch.Size.y + ch.Bearing.y) * scale - baseline;

      GLfloat w = ch.Size.x * scale;
      GLfloat h = ch.Size.y * scale;

      // Update VBO for each character
      GLfloat vertices[6][4] = {
              {xpos,     ypos + h, ch.TexCoords[0][0], ch.TexCoords[0][1]}, //lower left
              {xpos,     ypos,     ch.TexCoords[1][0], ch.TexCoords[1][1]}, //top left
              {xpos + w, ypos,     ch.TexCoords[2][0], ch.TexCoords[2][1]}, //top right

              {xpos,     ypos + h, ch.TexCoords[0][0], ch.TexCoords[0][1]}, //lower left
              {xpos + w, ypos,     ch.TexCoords[2][0], ch.TexCoords[2][1]}, //top right
              {xpos + w, ypos + h, ch.TexCoords[3][0], ch.TexCoords[3][1]}  //bottom right
      };
      // Render glyph texture over quad
      // Update content of VBO memory
      glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      // Render quad
      glDrawArrays(GL_TRIANGLES, 0, 6);
      // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
      xpos += ch.Advance * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_text_shader->stop();
  }
  glDisable(GL_BLEND);
}
CPointTextRenderer::~CPointTextRenderer() {
  glDeleteTextures(1, &m_texture);

  glDeleteBuffers(1, &m_text_vbo);
  glDeleteVertexArrays(1, &m_text_vao);
}
}
