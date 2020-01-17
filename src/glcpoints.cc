//
// Created by kalterkrieg on 25/11/2019.
//

#include <fstream>
#include <array>
#include <QtCore/QFile>
#include "glcpoints.h"
#include "globaloptions.h"

namespace glnemo {

int GLCPoint::next_id = 0;
//int GLCPointset::wwidth = 0; // useful for fixed size text tag

/******* GLCPoint ********/

GLCPoint::GLCPoint(std::array<float, 3> coords, float size, std::string text)
        : m_coords(coords), m_size(size) {
  m_id = next_id;
  next_id++;

  if (text == "")
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
void GLCPoint::setCoords(std::array<float, 3> coords) {
  m_coords = coords;
}
void GLCPoint::setSize(float size) {
  m_size = size;
}
void GLCPoint::setName(std::string name) {
  m_name = name;
}


/******* GLCPointset ********/
CPointTextRenderer* CPointset::text_renderer = nullptr;

void CPointset::sendUniforms() {
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);

  m_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
  m_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
  m_shader->sendUniformXfv("color", 3, 1, m_color.data());
}

CPointset::CPointset(CShader *shader, std::string name) :
        m_shader(shader), m_name(name), m_color{1, 1, 1} {
  m_is_shown = true;
  m_is_filled = false;
  m_is_name_visible = false;
  m_threshold = 100;
  m_fill_ratio = 0.1;
  m_name_angle = 45;
  m_name_offset = 1;
  m_name_size_factor = 1;
  m_nb_sphere_sections = 12;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenVertexArrays(1, &m_vao);

}

CPointset::CPointset(CShader *shader, const CPointset &other) {
  m_shader = shader;
  m_name = other.m_name;
  m_color = other.m_color;
  m_is_shown = other.m_is_shown;
  m_is_filled = other.m_is_filled;
  m_is_name_visible = other.m_is_name_visible;
  m_threshold = other.m_threshold;
  m_fill_ratio = other.m_fill_ratio;
  m_name_angle = other.m_name_angle;
  m_name_offset = other.m_name_offset;
  m_name_size_factor = other.m_name_size_factor;
  m_pointset_type = other.m_pointset_type;
  m_nb_sphere_sections = other.m_nb_sphere_sections;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenVertexArrays(1, &m_vao);
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

GLCPoint *CPointset::addPoint(std::array<float, 3> coords, float size, std::string text) {
  auto cpoint = new GLCPoint(coords, size, text);
  m_cpoints[cpoint->getId()] = cpoint;
  genVboData();
  return cpoint;
}

void CPointset::addPoints(std::vector<GLCPointData> cpoint_data_v) { // return vector maybe
  for (GLCPointData cpoint_data : cpoint_data_v) {
    auto cpoint = new GLCPoint(cpoint_data.coords, cpoint_data.size, cpoint_data.text);
    m_cpoints[cpoint->getId()] = cpoint;
  }
  genVboData();
}

void CPointset::setShow(bool show) {
  m_is_shown = show;
}

bool CPointset::isShown() {
  return m_is_shown;
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
  std::vector<float> data;
  //maybe use reserve to preallocate,
  // m_data.reserve(4*m_cpoints.size());

  std::vector<GLCPoint*> cpoints_v;
  for( auto it = m_cpoints.begin(); it != m_cpoints.end(); ++it ) {
      cpoints_v.push_back( it->second );
  }
  std::sort(cpoints_v.begin(), cpoints_v.end(), [](GLCPoint* a, GLCPoint* b) {
    return a->getSize() < b->getSize();
  });

  for (auto point : cpoints_v) {
    data.push_back(point->getCoords()[0]);
    data.push_back(point->getCoords()[1]);
    data.push_back(point->getCoords()[2]);
    data.push_back(point->getSize());
  }
  // SEND DATA
  glBindVertexArray(m_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
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
void CPointset::setFilled(bool filled) {
  m_is_filled = filled;
}
const bool CPointset::isFilled() const {
  return m_is_filled;
}
CPointsetTypes CPointset::getPointsetType() const {
  return m_pointset_type;
}
const int CPointset::getThreshold() const {
  return m_threshold;
}
const QColor CPointset::getQColor() const {
  return QColor(m_color[0] * 255, m_color[1] * 255, m_color[2] * 255);
}
void CPointset::setColor(const QColor &color) {
  m_color = {static_cast<float>(color.redF()), static_cast<float>(color.greenF()), static_cast<float>(color.blueF())};
}
const CPointsetTypes &CPointset::getShape() const {
  return m_pointset_type;
}
void CPointset::setColor(std::array<float, 3> color) {
  m_color = color;
}
const std::array<float, 3> &CPointset::getColor() const {
  return m_color;
}
void CPointset::setName(std::string new_name) {
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
void CPointset::setCpointText(int id, std::string text) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setName(text);
}
void CPointset::displayText() {
  text_renderer->renderText(this);
}
void CPointset::setNameVisible(bool visible) {
  m_is_name_visible = visible;
}
const bool CPointset::isNameVisible() {
  return m_is_name_visible;
}
void CPointset::setFillratio(float fill_ratio) {
  m_fill_ratio = fill_ratio < 0 ? 0 : fill_ratio > 1 ? 1 : fill_ratio; //clamp
}
const float CPointset::getFillratio() const {
  return m_fill_ratio;
}
void CPointset::setNameSizeFactor(float name_size_factor) {
  m_name_size_factor = name_size_factor;
}
const float CPointset::getNameSizeFactor() const {
  return m_name_size_factor;
}
void CPointset::setNameOffset(float name_offset) {
  m_name_offset = name_offset;
}
const float CPointset::getNameOffset() const {
  return m_name_offset;
}
void CPointset::setNameAngle(int angle) {
  m_name_angle = angle % 360;
}
const int CPointset::getNameAngle() const {
  return m_name_angle;
}
void CPointset::setNbSphereSection(int nb_sections) {
  m_nb_sphere_sections = nb_sections;
}
const int CPointset::getNbSphereSections() const {
  return m_nb_sphere_sections;
}

/******* GLCPointDisk ********/
CPointsetDisk::CPointsetDisk(CShader *shader, std::string name)
        : CPointsetRegularPolygon(shader, name) {
  m_nb_vertices = 100;
  m_pointset_type = CPointsetTypes::disk;
}
CPointsetDisk::CPointsetDisk(CShader *shader, const CPointset &other) : CPointsetRegularPolygon(shader, other) {
  m_nb_vertices = 100;
  m_pointset_type = CPointsetTypes::disk;
}

CPointsetRegularPolygon::CPointsetRegularPolygon(CShader *shader, std::string name) : CPointset(shader,
                                                                                                name) {
}
void CPointsetRegularPolygon::sendUniforms() {
   CPointset::sendUniforms();

  m_shader->sendUniformi("is_filled", m_is_filled);
  m_shader->sendUniformf("fill_ratio", m_fill_ratio);
}

void CPointsetRegularPolygon::display() {

  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_objects = m_cpoints.size() * m_threshold / 100;
  sendUniforms();
  if (m_is_filled) {
    m_shader->sendUniformi("nb_vertices", m_nb_vertices);
    glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, m_nb_vertices, nb_objects);
  } else {
    m_shader->sendUniformi("nb_vertices", m_nb_vertices * 2);
    glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, m_nb_vertices * 2 + 2, nb_objects);
  }
  glBindVertexArray(0);
  m_shader->stop();
}
CPointsetRegularPolygon::CPointsetRegularPolygon(CShader *shader, const CPointset &other) : CPointset(shader, other) {

}

/******* GLCPointSquare ********/
CPointsetSquare::CPointsetSquare(CShader *shader, std::string name)
        : CPointsetRegularPolygon(shader, name) {
  m_nb_vertices = 4;
  m_pointset_type = CPointsetTypes::square;
}
CPointsetSquare::CPointsetSquare(CShader *shader, const CPointset &other) : CPointsetRegularPolygon(shader, other) {
  m_nb_vertices = 4;
  m_pointset_type = CPointsetTypes::square;
}

/******* GLCPointTag ********/
CPointsetTag::CPointsetTag(CShader *shader, std::string name) : CPointset(shader, name) {
  m_pointset_type = CPointsetTypes::tag;
}

CPointsetTag::CPointsetTag(CShader *shader, const CPointset &other) : CPointset(shader, other) {
  m_pointset_type = CPointsetTypes::tag;
}

void CPointsetTag::display() {
  if (!ready() || !m_is_shown)
    return;

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
//  m_shader->sendUniformXfv("modelviewMatrixInverse", 16, 1, (const float *) glm::value_ptr(glm::inverse(mviewGLM)));
//  m_shader->sendUniformf("screen_scale", 2 * tagSizeOnScreen / wwidth);
}

/******* GLCPointSphere ********/
CPointsetSphere::CPointsetSphere(CShader *shader, std::string name) : CPointset(shader, name) {
  m_pointset_type = CPointsetTypes::sphere;

}

CPointsetSphere::CPointsetSphere(CShader *shader, const CPointset &other) : CPointset(shader, other) {
  m_pointset_type = CPointsetTypes::sphere;
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
  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_objects = m_cpoints.size() * m_threshold / 100;

  sendUniforms();
  m_shader->sendUniformi("subdivisions", m_nb_sphere_sections);

//    glEnable(GL_LINE_SMOOTH);
//    glEnable(GL_POLYGON_SMOOTH);
//    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
//    glLineWidth (1.0);
  int nb_vertex_per_sphere = m_nb_sphere_sections * m_nb_sphere_sections + m_nb_sphere_sections;
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, nb_vertex_per_sphere, nb_objects);
  glBindVertexArray(0);
  m_shader->stop();
}
void CPointsetSphere::sendUniforms() {
  CPointset::sendUniforms();
}

/******* GLCPointsetManager ********/


CPointsetManager::CPointsetManager() {
  // SHADER INIT
  m_nb_sets = 0;

  shapeToStr[CPointsetTypes::disk] = "disk";
  shapeToStr[CPointsetTypes::square] = "square";
  shapeToStr[CPointsetTypes::tag] = "tag";
  shapeToStr[CPointsetTypes::sphere] = "sphere";

  for (auto sts: shapeToStr) {
    strToShape[sts.second] = sts.first;
  }
}

void CPointsetManager::loadFile(std::string filepath) {
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
    CPointset *pointset;
    CShader *shader;

    std::string str_shape = (*it)["shape"];
    std::string name((*it).value("name", defaultName()));
    bool name_too_long = false;
    while (m_pointsets[name])
      if (name.size() < 50)
        name += " duplicate";
      else {
        name_too_long = true;
        break;
      }

    if (name_too_long)// TODO alert user pointset was not loaded
      continue;

    pointset = newPointset(str_shape, name);
    if (!pointset)
      continue;
    pointset->setColor((*it).value("color", std::array<float, 3>{0, 0, 0}));
    std::vector<GLCPointData> cpoint_data_v;
    auto data = (*it)["data"];
    cpoint_data_v.resize(data.size());
    for (std::size_t i = 0; i < data.size(); ++i) {
      cpoint_data_v[i] = {data[i]["coords"], data[i]["radius"],
                          data[i].value("text", std::string())}; // TODO change radius to size
    }
    pointset->addPoints(cpoint_data_v);
    m_pointsets[name] = pointset;
    m_nb_sets++;
  }
}

std::string CPointsetManager::defaultName() const {
  return "CPoint set " + std::to_string(m_nb_sets);
}

CPointsetManager::~CPointsetManager() {
  for (auto cpointset_pair: m_pointsets) {
    for (auto cpoint_pair : cpointset_pair.second->getCPoints())
      delete cpoint_pair.second;
    delete cpointset_pair.second;
  }
}

void CPointsetManager::initShaders() {

  auto text_renderer = new CPointTextRenderer();
  text_renderer->init();
  CPointset::text_renderer = text_renderer;

  m_regular_polygon_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/regular_polygon.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_regular_polygon_shader->init()) {
    delete m_regular_polygon_shader;
    std::cerr << "Failed to initialize regular_polygon shader\n";
    exit(1);
  }

  m_tag_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag_shape.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_tag_shader->init()) {
    delete m_tag_shader;
    std::cerr << "Failed to initialize tag shape shader\n";
    exit(1);
  }

  m_sphere_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/sphere.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_sphere_shader->init()) {
    delete m_sphere_shader;
    std::cerr << "Failed to initialize sphere shader\n";
    exit(1);
  }
}

void CPointsetManager::displayAll() {
  for (auto cpointset_pair: m_pointsets) {
    CPointset *cpointset = cpointset_pair.second;
    cpointset->display();
    if(cpointset->isNameVisible())
      cpointset->displayText();
  }
}

CPointset *CPointsetManager::createNewCPointset() {
  CPointsetDisk *pointset = new CPointsetDisk(m_regular_polygon_shader, defaultName());
  m_pointsets[defaultName()] = pointset;
  m_nb_sets++;
  return pointset;
}
void CPointsetManager::deleteCPointset(std::string pointset_name) {
  auto it = m_pointsets.find(pointset_name);
  m_pointsets.erase(it);
  m_nb_sets--;
}
CPointset * CPointsetManager::changePointsetType(std::string pointset_name, std::string new_type) {
  CPointset *old_pointset = m_pointsets[pointset_name];
  CPointset *new_pointset = newPointset(new_type, *old_pointset);
  if (new_pointset) {
    m_pointsets[pointset_name] = new_pointset;
    delete old_pointset;
    return new_pointset;
  }
  return nullptr;
}

void CPointsetManager::setW(int w) {
//  GLCPointset::wwidth = w;
}

void CPointsetManager::saveToFile(std::string file_path) {
  std::ofstream outfile(file_path);
  json final_obj;
  for (auto pair: *this) {
    CPointset *set = pair.second;
    json data_array = json::array();
    for (auto cpoint_pair : set->getCPoints()) {
      GLCPoint *cpoint = cpoint_pair.second;
      data_array.push_back({{"coords", cpoint->getCoords()},
                            {"radius", cpoint->getSize()}});
    }
    json current_dict = {{"name",  set->getName()},
                         {"shape", shapeToStr[set->getShape()]},
                         {"color", set->getColor()},
                         {"data",  data_array}};

    final_obj.push_back(current_dict);
  }
  outfile << final_obj;
}

CPointset *CPointsetManager::newPointset(std::string str_shape, std::string name) {
  if (str_shape == "disk") {
    return new CPointsetDisk(m_regular_polygon_shader, name);
  } else if (str_shape == "square") {
    return new CPointsetSquare(m_regular_polygon_shader, name);
  } else if (str_shape == "tag") {
    return new CPointsetTag(m_tag_shader, name);
  } else if (str_shape == "sphere") {
    return new CPointsetSphere(m_sphere_shader, name);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}

CPointset *CPointsetManager::newPointset(std::string str_shape, const CPointset& other) {
  if (str_shape == "disk") {
    return new CPointsetDisk(m_regular_polygon_shader, other);
  } else if (str_shape == "square") {
    return new CPointsetSquare(m_regular_polygon_shader, other);
  } else if (str_shape == "tag") {
    return new CPointsetTag(m_tag_shader, other);
  } else if (str_shape == "sphere") {
    return new CPointsetSphere(m_sphere_shader, other);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}
void CPointsetManager::setPointsetName(std::string old_name, std::string new_name) {
  if (old_name != new_name) {
    const iterator it = m_pointsets.find(old_name);
    if (it != m_pointsets.end()) {
      m_pointsets[new_name] = it->second;
      m_pointsets[new_name]->setName(new_name);
      m_pointsets.erase(it);
    }
  }
}
void CPointsetManager::deleteCPoint(std::string pointset_name, int cpoint_id) {
  m_pointsets.at(pointset_name)->deletePoint(cpoint_id);
}

void CPointTextRenderer::init() {

  m_text_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag_text.vert",

          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag_text.frag");
  if (!m_text_shader->init()) {
    delete m_text_shader;
    std::cerr << "Failed to initialize tag text shader\n";
    exit(1);
  }

  glGenVertexArrays(1, &m_text_vao);
  glGenBuffers(1, &m_text_vbo);
  glBindVertexArray(m_text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
  }

  QFile font_file(GlobalOptions::RESPATH + "/fonts/DejaVuSansMono.ttf");
  if (!font_file.open(QIODevice::ReadOnly)) {
    std::cout << "Could not open font file" << std::endl;
    exit(1);
  }
  QByteArray blob = font_file.readAll();
  const FT_Byte *data = reinterpret_cast<const FT_Byte * >(blob.data());
  FT_Face face;

  if (FT_New_Memory_Face(ft, data, blob.size(), 0, &face)) {
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

  for (GLubyte c = 0; c < 128; c++) {
    // Load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
      continue;
    }
    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
    );
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use
    Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
    };
    Characters.insert(std::pair<GLchar, Character>(c, character));
  }
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}

void CPointTextRenderer::renderText(CPointset *pointset) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Iterate through all cpoints
  for (auto cpoint_pair: pointset->getCPoints()) {
    GLCPoint *cpoint = cpoint_pair.second;
    float y = 0;
    float x = 0;
    float scale = .01;
    std::string text = cpoint->getName();

    m_text_shader->start();

    GLfloat proj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    GLfloat mview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mview);

    m_text_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
    m_text_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);

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

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
      Character ch = Characters[*c];

      GLfloat xpos = x + ch.Bearing.x * scale;
      GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

      GLfloat w = ch.Size.x * scale;
      GLfloat h = ch.Size.y * scale;
      // Update VBO for each character
      GLfloat vertices[6][4] = {
              {xpos,     ypos + h, 0.0, 0.0},
              {xpos,     ypos,     0.0, 1.0},
              {xpos + w, ypos,     1.0, 1.0},

              {xpos,     ypos + h, 0.0, 0.0},
              {xpos + w, ypos,     1.0, 1.0},
              {xpos + w, ypos + h, 1.0, 0.0}
      };
      // Render glyph texture over quad
      glBindTexture(GL_TEXTURE_2D, ch.TextureID);
      // Update content of VBO memory
      glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      // Render quad
      glDrawArrays(GL_TRIANGLES, 0, 6);
      // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
      x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_text_shader->stop();
  }
}
CPointTextRenderer::~CPointTextRenderer() {
  for (auto ch : Characters)
    glDeleteTextures(1, &ch.second.TextureID);

  glDeleteBuffers(1, &m_text_vbo);
  glDeleteVertexArrays(1, &m_text_vao);
}
}
