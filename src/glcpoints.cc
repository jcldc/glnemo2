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

  if(text=="")
    m_text = "CPoint " + std::to_string(m_id);
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
const int &GLCPoint::getId() const {
  return m_id;
}
void GLCPoint::setCoords(std::array<float, 3> coords) {
  m_coords = coords;
}
void GLCPoint::setSize(float size) {
  m_size = size;
}
void GLCPoint::setText(std::string text) {
  m_text = text;
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

GLCPoint * GLCPointset::addPoint(std::array<float, 3> coords, float size, std::string text) {
  auto cpoint = new GLCPoint(coords, size, text);
  m_cpoints[cpoint->getId()] = cpoint;
  genVboData();
  return cpoint;
}

void GLCPointset::addPoints(std::vector<GLCPointData> cpoint_data_v) { // return vector maybe
  for(GLCPointData cpoint_data : cpoint_data_v){
    auto cpoint = new GLCPoint(cpoint_data.coords, cpoint_data.size, cpoint_data.text);
    m_cpoints[cpoint->getId()] = cpoint;
  }
  genVboData();
}

void GLCPointset::setShow(bool show) {
  m_is_shown = show;
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

void GLCPointset::genVboData() {
  // DATA INIT
  std::vector<float> data;
  //maybe use reserve to preallocate,
  // m_data.reserve(4*m_cpoints.size());
  for (auto point : m_cpoints) {
    data.push_back(point.second->getCoords()[0]);
    data.push_back(point.second->getCoords()[1]);
    data.push_back(point.second->getCoords()[2]);
    data.push_back(point.second->getSize());
  }
  // SEND DATA
  glBindVertexArray(m_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);

  setAttributes();
  glBindVertexArray(0);
}

const glcpointmap_t &GLCPointset::getCPoints() const {
  return m_cpoints;
}

void GLCPointset::copyCPoints(GLCPointset *other) {
  m_cpoints = other->getCPoints();
  genVboData();
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
  m_color = {static_cast<float>(color.redF()), static_cast<float>(color.greenF()), static_cast<float>(color.blueF())};
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
void GLCPointset::setName(std::string new_name) {
  m_name = new_name;
}
void GLCPointset::deletePoint(int id) {
  GLCPoint* cpoint = m_cpoints[id];
  m_cpoints.erase(id);
  delete cpoint;
  genVboData();
}
void GLCPointset::setCpointSize(int id, float size ) {
  m_cpoints.at(id)->setSize(size);
  genVboData();
}
void GLCPointset::setCpointCoords(int id, std::array<float, 3> coords) {
  m_cpoints.at(id)->setCoords(coords);
  genVboData();
}
void GLCPointset::setCpointCoordsX(int id, float x) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({x, cpoint->getCoords()[1],  cpoint->getCoords()[2]});
  genVboData();
}
void GLCPointset::setCpointCoordsY(int id, float y) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({cpoint->getCoords()[0], y, cpoint->getCoords()[2]});
  genVboData();
}
void GLCPointset::setCpointCoordsZ(int id, float z) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setCoords({cpoint->getCoords()[0],  cpoint->getCoords()[1], z});
  genVboData();
}
void GLCPointset::setCpointText(int id, std::string text) {
  GLCPoint *cpoint = m_cpoints.at(id);
  cpoint->setText(text);
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
  int nb_objects = m_cpoints.size() * m_threshold / 100;
  m_shader->sendUniformi("is_filled", m_is_filled);
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

/******* GLCPointSquare ********/
GLCPointsetSquare::GLCPointsetSquare(CShader *shader, std::string name)
        : GLCPointsetRegularPolygon(shader, name) {
  m_nb_vertices = 4;
  m_pointset_type = CPointsetTypes::square;
}

/******* GLCPointTag ********/
GLCPointsetTag::GLCPointsetTag(CShader *shape_shader, CShader *text_shader, std::string name) : GLCPointset(shape_shader, name) {
  m_pointset_type = CPointsetTypes::tag;
  m_text_shader = text_shader;

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
    exit( 1);
  }
  QByteArray blob = font_file.readAll();
  const FT_Byte* data = reinterpret_cast<const FT_Byte* >(blob.data());
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

void GLCPointsetTag::display() {
  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_objects = m_cpoints.size() * m_threshold / 100;

  sendUniforms();
  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, 3, nb_objects);

  glBindVertexArray(0);
  m_shader->stop();

  renderText();
}

void GLCPointsetTag::sendUniforms() {
  GLCPointset::sendUniforms();

//  float tagSizeOnScreen = 50; // InPixels
//
//  GLfloat mview[16];
//  glGetFloatv(GL_MODELVIEW_MATRIX, mview);
//  glm::mat4 mviewGLM = glm::make_mat4(mview);
//  m_shader->sendUniformXfv("modelviewMatrixInverse", 16, 1, (const float *) glm::value_ptr(glm::inverse(mviewGLM)));
//  m_shader->sendUniformf("screen_scale", 2 * tagSizeOnScreen / wwidth);
}

void GLCPointsetTag::renderText() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Iterate through all cpoints
  for (auto cpoint_pair: m_cpoints) {
    GLCPoint *cpoint = cpoint_pair.second;
    float y = 0;
    float x = 0;
    float scale = .01;
    std::string text = cpoint->getText();

    m_text_shader->start();

    GLfloat proj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    GLfloat mview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mview);

    m_text_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
    m_text_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);

    m_text_shader->sendUniformXfv("color", 3, 1, m_color.data());
    m_text_shader->sendUniformXfv("point_center", 3, 1, cpoint->getCoords().data());

    m_text_shader->sendUniformf("offset", cpoint->getSize());
    scale *= cpoint->getSize();
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
GLCPointsetTag::~GLCPointsetTag() {
  for(auto ch : Characters)
    glDeleteTextures(1, &ch.second.TextureID);
}
/******* GLCPointSphere ********/
int GLCPointsetSphere::subdivisions = 16;
int GLCPointsetSphere::nb_vertex_per_sphere = subdivisions*subdivisions+subdivisions;

GLCPointsetSphere::GLCPointsetSphere(CShader *m_shader, std::string name) : GLCPointset(m_shader,
                                                                                        name) {
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

void GLCPointsetSphere::display() {
  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  int nb_objects = m_cpoints.size() * m_threshold / 100;

  sendUniforms();
  m_shader->sendUniformi("subdivisions", subdivisions);

//    glEnable(GL_LINE_SMOOTH);
//    glEnable(GL_POLYGON_SMOOTH);
//    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
//    glLineWidth (1.0);

  glDrawArraysInstancedARB(GL_LINE_STRIP, 0, nb_vertex_per_sphere, nb_objects);
  glBindVertexArray(0);
  m_shader->stop();
}
void GLCPointsetSphere::sendUniforms() {
  GLCPointset::sendUniforms();
}

/******* GLCPointsetManager ********/


GLCPointsetManager::GLCPointsetManager() {
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
    bool name_too_long = false;
    while(m_pointsets[name])
      if(name.size()<50)
        name += " duplicate";
      else{
        name_too_long = true;
        break;
      }

    if(name_too_long)// TODO alert user pointset was not loaded
      continue;

    pointset = newPointset(str_shape, name);
    if(!pointset)
      continue;
    pointset->setColor((*it).value("color", std::array<float, 3> {0,0,0}));
    std::vector<GLCPointData> cpoint_data_v;
    auto data = (*it)["data"];
    cpoint_data_v.resize(data.size());
    for (std::size_t i = 0; i < data.size(); ++i) {
      cpoint_data_v[i] = {data[i]["coords"], data[i]["radius"], data[i].value("text", std::string())}; // TODO change radius to size
    }
    pointset->addPoints(cpoint_data_v);
    m_pointsets[name] = pointset;
    m_nb_sets++;
  }
}

std::string GLCPointsetManager::defaultName() const {
  return "CPoint set " + std::to_string(m_nb_sets);
}

GLCPointsetManager::~GLCPointsetManager() {
  for (auto cpointset_pair: m_pointsets) {
    for (auto cpoint_pair : cpointset_pair.second->getCPoints())
      delete cpoint_pair.second;
    delete cpointset_pair.second;
  }
}

void GLCPointsetManager::initShaders() {

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

  m_tag_text_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag_text.vert",

          GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/tag_text.frag");
  if (!m_tag_text_shader->init()) {
    delete m_tag_text_shader;
    std::cerr << "Failed to initialize tag text shader\n";
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

void GLCPointsetManager::displayAll() {
  for (auto cpointset: m_pointsets)
    cpointset.second->display();
}

GLCPointset* GLCPointsetManager::createNewCPointset() {
  GLCPointsetDisk *pointset = new GLCPointsetDisk(m_regular_polygon_shader, defaultName());
  m_pointsets[defaultName()] = pointset;
  m_nb_sets++;
  return pointset;
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
    new_pointset->setColor(old_pointset->getQColor());
    new_pointset->setShow(old_pointset->isShown());
    new_pointset->setThreshold(old_pointset->getThreshold());
    m_pointsets[pointset_name] = new_pointset;
    delete old_pointset;
  }
}

void GLCPointsetManager::setW(int w) {
//  GLCPointset::wwidth = w;
}

void GLCPointsetManager::saveToFile(std::string file_path) {
  std::ofstream outfile (file_path);
  json final_obj;
  for(auto pair: *this){
    GLCPointset *set = pair.second;
    json data_array = json::array();
    for(auto cpoint_pair : set->getCPoints()){
      GLCPoint *cpoint = cpoint_pair.second;
      data_array.push_back({{"coords", cpoint->getCoords()},
                                {"radius", cpoint->getSize()}});
    }
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
    return new GLCPointsetTag(m_tag_shader, m_tag_text_shader, name);
  } else if (str_shape == "sphere") {
    return new GLCPointsetSphere(m_sphere_shader, name);
  } else {
    std::cerr << "Unrecognized shape : " + str_shape;
    return nullptr;
  }
}
void GLCPointsetManager::setPointsetName(std::string old_name, std::string new_name) {
  if (old_name != new_name) {
    const iterator it = m_pointsets.find(old_name);
    if (it != m_pointsets.end()) {
      m_pointsets[new_name] = it->second;
      m_pointsets[new_name]->setName(new_name);
      m_pointsets.erase(it);
    }
  }
}
}
