//
// Created by kalterkrieg on 25/11/2019.
//

#include <fstream>
#include <QFileInfo>
#include "glcpoints.h"
#include "globaloptions.h"

namespace glnemo {

GLCPoint::GLCPoint(std::array<float, 3> coords, float size)
    : m_coords(coords), m_size(size) {
}

const std::array<float, 3> &GLCPoint::getCoords() const {
  return m_coords;
}

const float &GLCPoint::getSize() const {
  return m_size;
}

void GLCPoint::display(CShader *shader) {
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  GLfloat mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);

  shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
  shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
  shader->sendUniformi("nb_vertices", 100);
}

/******* GLCPointAnnulus ********/

GLCPointAnnulus::GLCPointAnnulus(std::array<float, 3> coords, float size, float fill_ratio) :
    GLCPoint(coords, size),
    m_fill_ratio(fill_ratio) {
}

std::vector<float> GLCPointAnnulus::getPackedData() {
  std::vector<float> packed_data;
  packed_data.push_back(m_coords[0]);
  packed_data.push_back(m_coords[1]);
  packed_data.push_back(m_coords[2]);
  packed_data.push_back(m_size);
  packed_data.push_back(m_fill_ratio);

  return packed_data;
}

void GLCPointAnnulus::setAttributes(CShader *shader) {

  GLuint point_center_annulus_attrib = glGetAttribLocation(shader->getProgramId(), "point_center");
  GLuint radius_annulus_attrib = glGetAttribLocation(shader->getProgramId(), "radius");
  GLuint fill_ratio_annulus_attrib = glGetAttribLocation(shader->getProgramId(), "fill_ratio");
  if (point_center_annulus_attrib == -1) {
    std::cerr << "Error occured when getting \"point_center\" attribute (annulus shader)\n";
    exit(1);
  }
  if (radius_annulus_attrib == -1) {
    std::cerr << "Error occured when getting \"radius\" attribute (annulus shader)\n";
    exit(1);
  }
  if (fill_ratio_annulus_attrib == -1) {
    std::cerr << "Error occured when getting \"fill_ratio\" attribute (annulus shader)\n";
    exit(1);
  }
  glEnableVertexAttribArrayARB(point_center_annulus_attrib);
  glVertexAttribPointerARB(point_center_annulus_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
  glVertexBindingDivisor(point_center_annulus_attrib, 1);

  glEnableVertexAttribArrayARB(radius_annulus_attrib);
  glVertexAttribPointerARB(radius_annulus_attrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                           (void *) (3 * sizeof(float)));
  glVertexBindingDivisor(radius_annulus_attrib, 1);

  glEnableVertexAttribArrayARB(fill_ratio_annulus_attrib);
  glVertexAttribPointerARB(fill_ratio_annulus_attrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                           (void *) (4 * sizeof(float)));
  glVertexBindingDivisor(fill_ratio_annulus_attrib, 1);

}

GLCPointAnnulus::~GLCPointAnnulus() {

}

void GLCPointAnnulus::displayAll(CShader *shader, glcpointset_t &cpoints, int threshold) {
  GLCPoint::display(shader);
  int nb_instances = cpoints.size()*threshold/100;
  glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, 100, nb_instances);

}

/******* GLCPointDisk ********/

GLCPointDisk::GLCPointDisk(std::array<float, 3> coords, float size) :
    GLCPoint(coords, size) {
}

std::vector<float> GLCPointDisk::getPackedData() {
  std::vector<float> packed_data;
  packed_data.push_back(m_coords[0]);
  packed_data.push_back(m_coords[1]);
  packed_data.push_back(m_coords[2]);
  packed_data.push_back(m_size);

  return packed_data;
}

void GLCPointDisk::setAttributes(CShader *shader) {

  GLuint point_center_disk_attrib = glGetAttribLocation(shader->getProgramId(), "point_center");
  GLuint radius_disk_attrib = glGetAttribLocation(shader->getProgramId(), "radius");
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

GLCPointDisk::~GLCPointDisk() {

}

void GLCPointDisk::displayAll(CShader *shader, glcpointset_t &cpoints, int threshold) {
  GLCPoint::display(shader);
  int nb_instances = cpoints.size()*threshold/100;
  glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, 100, nb_instances);
}

/******* GLCPointSet ********/

GLCPointSet::GLCPointSet(Shape shape, CShader *shader, void(*setAttributesFn)(CShader *),
                         void(*displayAllFn)(CShader *, glcpointset_t &, int), std::string name) :
    m_shape(shape), m_shader(shader), m_setAttributesFn(setAttributesFn), m_displayAllFn(displayAllFn), m_name(name) {
  m_is_shown = true;
  m_color = QColor("white");
  m_threshold = 100;
  // SHADER INIT
  glGenBuffersARB(1, &m_vbo);
  glGenVertexArrays(1, &m_vao);

}

GLCPointSet::~GLCPointSet() {
  for (auto charac_point : m_cpoints) {
    delete charac_point; //FIXME
  }
  m_cpoints.clear();
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void GLCPointSet::initVboData() {
  m_data.clear();
  // DATA INIT

  //maybe use reserve to preallocate,
  for (auto point : m_cpoints) {
    auto data = point->getPackedData();
    m_data.insert(m_data.end(), data.begin(), data.end());
  }

  // SEND DATA
  glBindVertexArray(m_vao);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * m_data.size(), m_data.data(), GL_STATIC_DRAW);

  this->m_setAttributesFn(m_shader);

  glBindVertexArray(0);
}

void GLCPointSet::display() {
  if (!ready() || !m_is_shown)
    return;

  m_shader->start();
  glBindVertexArray(m_vao);
  this->m_displayAllFn(m_shader, m_cpoints, m_threshold);
  glBindVertexArray(0);
  m_shader->stop();
}

bool GLCPointSet::ready() {
  return m_shader != nullptr;
}

const std::string &GLCPointSet::getName() const {
  return m_name;
}

void GLCPointSet::addPoint(GLCPoint *point) {
  m_cpoints.insert(point);
}

void GLCPointSet::hide() {
  m_is_shown = false;
}

void GLCPointSet::show() {
  m_is_shown = true;
}
bool GLCPointSet::isShown() {
  return m_is_shown;
}
int GLCPointSet::setThreshold(int threshold) {
  m_threshold = threshold < 0 ? 0 : threshold > 100 ? 100 : threshold; //clamp
}

/******* GLCPointSetManager ********/


GLCPointSetManager::GLCPointSetManager() {
  // SHADER INIT
  m_nb_sets = 0;
}

void GLCPointSetManager::loadFile(std::string filepath) {
  std::cerr << "Loading json file\n";
  std::ifstream file(filepath);
  json json_data;
  file >> json_data;


  for (json::iterator it = json_data.begin(); it != json_data.end(); ++it) {
    m_nb_sets++;
    Shape shape;
    CShader *shader;
    GLCPointSet *pointset;

    std::string str_shape = (*it)["shape"];
    std::string name((*it).value("name", "CPoint set " + std::to_string(m_nb_sets)));
    if (str_shape == "annulus") {
      shader = m_annulus_shader;
      shape = Shape::annulus;
      void(*setAttributesFn)(CShader *) = &GLCPointAnnulus::setAttributes;
      void(*displayAllFn)(CShader *, glcpointset_t &, int) = &GLCPointAnnulus::displayAll;
      pointset = new GLCPointSet(shape, shader, setAttributesFn, displayAllFn, name);


      for (json::iterator data = (*it)["data"].begin(); data != (*it)["data"].end(); ++data) {
        auto cpoint = new GLCPointAnnulus((*data)["coords"], (*data)["radius"], (*it)["fill_ratio"]);
        pointset->addPoint(cpoint);
      }

    } else if (str_shape == "disk") {
      shader = m_disk_shader;
      shape = Shape::disk;
      void(*setAttributesFn)(CShader *) = &GLCPointDisk::setAttributes;
      void(*displayAllFn)(CShader *, glcpointset_t &, int) = &GLCPointDisk::displayAll;
      pointset = new GLCPointSet(shape, shader, setAttributesFn, displayAllFn, name);


      for (json::iterator data = (*it)["data"].begin(); data != (*it)["data"].end(); ++data) {
        auto cpoint = new GLCPointDisk((*data)["coords"], (*data)["radius"]);
        pointset->addPoint(cpoint);
      }

    } else {
      std::cerr << "Unrecognized shape : " + str_shape;
      continue;
    }

    pointset->initVboData();

    m_pointsets[name] = pointset;
  }


}

GLCPointSetManager::~GLCPointSetManager() {
  for (auto cpointset: m_pointsets)
    delete cpointset.second;
}

void GLCPointSetManager::initShaders() {

  this->m_disk_shader = new CShader(
      GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/disk.vert",
      GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_disk_shader->init()) {
    delete m_disk_shader;
    std::cerr << "Failed to initialize disk shader\n";
    exit(1);
  }
  this->m_annulus_shader = new CShader(
      GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/annulus.vert",
      GlobalOptions::RESPATH.toStdString() + "/shaders/cpoints/characteristic.frag");
  if (!m_annulus_shader->init()) {
    delete m_annulus_shader;
    std::cerr << "Failed to initialize annulus shader\n";
    exit(1);
  }

}

void GLCPointSetManager::displayAll() {
  for (auto cpointset: m_pointsets)
    cpointset.second->display();
}

//GLCPoint GLCPointFactory::makeGLCPoint(std::string type, ) {
//  if(type == "annulus")
//    return GLCPoint();
//}
}
