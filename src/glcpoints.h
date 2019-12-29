//
// Created by kalterkrieg on 25/11/2019.
//

#ifndef GLNEMO2_GLCPOINTS_H
#define GLNEMO2_GLCPOINTS_H

#include "cshader.h"
#include <QColor>
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

class GLCPointset {
public:
  GLCPointset(CShader *m_shader, std::string name = "");
  virtual ~GLCPointset();
  virtual void display() = 0;
  void initVboData();
  void setAttributes();
  void addPoint(GLCPoint *point);
  void sendUniforms(int nb_vertices);
  void copyCPoints(GLCPointset*);
  const glcpointset_t& getCPoints() const;
  bool ready();
  void hide();
  void show();
  bool isShown();
  void setFilled(bool);
  const bool isFilled() const;
  const std::string &getName() const;
  void setThreshold(int threshold);

protected:
  CShader *m_shader;
  glcpointset_t m_cpoints;
  GLuint m_vao, m_vbo;
  std::vector<float> m_data;
  std::string m_name;
  QColor m_color;
  bool m_is_shown;
  bool m_is_filled;
  int m_threshold;
};

class GLCPointsetDisk : public GLCPointset {
public:
  GLCPointsetDisk(CShader *m_shader, std::string name = "");
  void display();
  const int m_nb_vertices = 100;
};

class GLCPointsetSquare : public GLCPointset {
public:
  GLCPointsetSquare(CShader *m_shader, std::string name = "");
  void display();
  const int m_nb_vertices = 4;
};

class GLCPointsetManager {
public:
  GLCPointsetManager();
  ~GLCPointsetManager();
  void loadFile(std::string filepath);
  void initShaders();
  void displayAll();
  void createNewCPointset();
  void deleteCPointset(std::string);
  void changePointsetType(std::string pointset_name, std::string new_type);

  typedef typename std::map<std::string, GLCPointset *> map_type;
  typedef typename map_type::iterator iterator;
  typedef typename map_type::const_iterator const_iterator;

  inline iterator begin() noexcept { return m_pointsets.begin(); }
  inline const_iterator cbegin() const noexcept { return m_pointsets.cbegin(); }
  inline iterator end() noexcept { return m_pointsets.end(); }
  inline const_iterator cend() const noexcept { return m_pointsets.cend(); }

  GLCPointset *&operator[](const std::string &name) {
    return m_pointsets.at(name);
  }

private:
  int m_nb_sets;
  CShader *m_disk_shader, *m_square_shader;
  std::map<std::string, GLCPointset *> m_pointsets;
  std::string defaultName() const;
};

} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
