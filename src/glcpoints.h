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
#include <array>

using json = nlohmann::json;

#define NB_POINTS 100

namespace glnemo {

struct PPred {
  template<typename T>
  inline bool operator()(const T *a, const T *b) const { return *a < *b; }
};

enum CPointsetTypes{
  disk,
  square,
  tag
};

class GLCPoint {
public:
  GLCPoint(std::array<float, 3> coords, float size, std::string text);
  const std::array<float, 3> &getCoords() const;
  const float &getSize() const;
  const std::string &getText() const;

  inline bool operator<(const GLCPoint &other) const { return m_size > other.getSize(); }
  inline bool operator>(const GLCPoint &other) const { return *this < other; }
  inline bool operator<=(const GLCPoint &other) const { return !(other > *this); }
  inline bool operator>=(const GLCPoint &other) const { return !(other < *this); }

private:
  std::array<float, 3> m_coords;
  float m_size;
  std::string m_text;
};

typedef std::multiset<GLCPoint *, PPred> glcpointset_t;

class GLCPointset {
public:
  GLCPointset(CShader *m_shader, std::string name = "");
  virtual ~GLCPointset();
  virtual void display() = 0;
  void initVboData();
  void setAttributes();
  void addPoint(std::array<float, 3> coords, float size, std::string text);
  virtual void sendUniforms();
  void copyCPoints(GLCPointset*);
  const glcpointset_t& getCPoints() const;
  bool ready();
  void hide();
  void show();
  bool isShown();
  void setFilled(bool);
  const bool isFilled() const;
  const std::string &getName() const;
  void setName(std::string);
  void setThreshold(int threshold);
  const int getThreshold() const;
  inline int size() {return m_cpoints.size();}
  CPointsetTypes getPointsetType() const;
  const QColor getQColor() const;
  const std::array<float, 3> &getColor() const;
  void setColor(const QColor &color);
  void setColor(std::array<float, 3>);
  const CPointsetTypes &getShape() const;

  static int wwidth;

protected:
  CShader *m_shader;
  glcpointset_t m_cpoints;
  GLuint m_vao, m_vbo;
  std::vector<float> m_data;
  std::string m_name;
  std::array<float, 3> m_color;
  bool m_is_shown;
  bool m_is_filled;
  int m_threshold;
  CPointsetTypes m_pointset_type;
};

class GLCPointsetRegularPolygon : public GLCPointset {
public:
  GLCPointsetRegularPolygon(CShader *m_shader, std::string name);
  void display();
protected:
  int m_nb_vertices; //TODO make const with constructor
};

class GLCPointsetDisk : public GLCPointsetRegularPolygon {
public:
  GLCPointsetDisk(CShader *m_shader, std::string name);
};

class GLCPointsetSquare : public GLCPointsetRegularPolygon {
public:
  GLCPointsetSquare(CShader *m_shader, std::string name);
};

class GLCPointsetTag : public GLCPointset {
public:
  GLCPointsetTag(CShader *m_shader, std::string name);
  void display();
  void sendUniforms();
};

class GLCPointsetManager {
public:
  GLCPointsetManager();
  ~GLCPointsetManager();
  void loadFile(std::string filepath);
  void initShaders();
  void displayAll();
  GLCPointset* createNewCPointset();
  void deleteCPointset(std::string);
  void changePointsetType(std::string pointset_name, std::string new_type);
  static void setW(int w);

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

  void saveToFile(std::string file_path);

  std::map<CPointsetTypes, std::string> shapeToStr;
  std::map<std::string,CPointsetTypes> strToShape;
  void setPointsetName(std::string old_name, std::string new_name);
private:
  int m_nb_sets;
  CShader *m_regular_polygon_shader, *m_tag_shader;
  std::map<std::string, GLCPointset *> m_pointsets;
  std::string defaultName() const;
  GLCPointset *newPointset(std::string str_shape, std::string name);
};

} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
