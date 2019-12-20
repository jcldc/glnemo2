//
// Created by kalterkrieg on 25/11/2019.
//

#ifndef GLNEMO2_GLCPOINTS_H
#define GLNEMO2_GLCPOINTS_H

#include "cshader.h"
#include <QtGui/QColor>
#include <iostream>
#include <json.hpp>
#include <map>
#include <set>

using json = nlohmann::json;

#define NB_POINTS 100

namespace glnemo {

struct PPred {
  template<typename T>
  inline bool operator()(const T *a, const T *b) const { return *a < *b; }
};

class GLCPoint {
public:
  GLCPoint(std::array<float, 3> coords, float size);
  const std::array<float, 3> &getCoords() const;
  const float &getSize() const;

  inline bool operator<(const GLCPoint &other) const { return m_size > other.getSize(); }
  inline bool operator>(const GLCPoint &other) const { return *this < other; }
  inline bool operator<=(const GLCPoint &other) const { return !(other > *this); }
  inline bool operator>=(const GLCPoint &other) const { return !(other < *this); }

private:
  std::array<float, 3> m_coords;
  float m_size;
  std::string m_text;
};

typedef std::set<GLCPoint *, PPred> glcpointset_t;

class GLCPointSet {
public:
  GLCPointSet(CShader *shader, std::string name = "");
  ~GLCPointSet();
  virtual void display() = 0;
  void initVboData();
  void setAttributes();
  void addPoint(GLCPoint *point);
  void sendUniforms();
  bool ready();
  void hide();
  void show();
  bool isShown();
  const std::string &getName() const;
  int setThreshold(int threshold);

protected:
  CShader *m_shader;
  glcpointset_t m_cpoints;
  GLuint m_vao, m_vbo;
  std::vector<float> m_data;
  std::string m_name;
  QColor m_color;
  bool m_is_shown;
  int m_threshold;
  const int nb_vertices = 100;
};

class GLCPointSetAnnulus : public GLCPointSet {
public:
  GLCPointSetAnnulus(CShader *shader, std::string name = "");
  void display();
};

class GLCPointSetDisk : public GLCPointSet {
public:
  GLCPointSetDisk(CShader *shader, std::string name = "");
  void display();
};

class GLCPointSetManager {
public:
  GLCPointSetManager();
  ~GLCPointSetManager();
  void loadFile(std::string filepath);
  void initShaders();
  void displayAll();

  typedef typename std::map<std::string, GLCPointSet *> map_type;
  typedef typename map_type::iterator iterator;
  typedef typename map_type::const_iterator const_iterator;

  inline iterator begin() noexcept { return m_pointsets.begin(); }
  inline const_iterator cbegin() const noexcept { return m_pointsets.cbegin(); }
  inline iterator end() noexcept { return m_pointsets.end(); }
  inline const_iterator cend() const noexcept { return m_pointsets.cend(); }

  GLCPointSet *&operator[](const std::string &name) {
    return m_pointsets.at(name);
  }

private:
  int m_nb_sets;
  CShader *m_disk_shader;
  CShader *m_annulus_shader;
  std::map<std::string, GLCPointSet *> m_pointsets;
};

} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
