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
#include <ft2build.h>
#include <glm/glm.hpp>
#include FT_FREETYPE_H

using json = nlohmann::json;

namespace glnemo {

class CPointTextRenderer;

struct Character {
  GLuint     TextureID;  // ID handle of the glyph texture
  glm::ivec2 Size;       // Size of glyph
  glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
  GLuint     Advance;    // Offset to advance to next glyph
};

enum CPointsetTypes{
  disk,
  square,
  tag,
  sphere
};

class GLCPoint {
public:
  GLCPoint(std::array<float, 3> coords, float size, std::string text);
  const std::array<float, 3> &getCoords() const;
  const float &getSize() const;
  const std::string &getName() const;
  const int &getId() const;

  inline bool operator<(const GLCPoint &other) const { return m_size > other.getSize(); }
  inline bool operator>(const GLCPoint &other) const { return *this < other; }
  inline bool operator<=(const GLCPoint &other) const { return !(other > *this); }
  inline bool operator>=(const GLCPoint &other) const { return !(other < *this); }

private:
  void setSize(float size);
  void setCoords(std::array<float, 3>);
  void setName(std::string name);

  std::array<float, 3> m_coords;
  float m_size;
  std::string m_name;
  int m_id;

  static int next_id;

  friend class CPointset;
};

typedef std::map<int, GLCPoint *> glcpointmap_t;

struct GLCPointData {
  std::array<float, 3> coords;
  float size;
  std::string text;
};

class CPointset {
public:
  CPointset(CShader *m_shader, std::string name = "");
  virtual ~CPointset();
  virtual void display() = 0;
  virtual void setAttributes();
  virtual void sendUniforms();
  void displayText();
  void copyCPoints(CPointset*);
  bool ready();

  GLCPoint * addPoint(std::array<float, 3> coords, float size, std::string text);
  void addPoints(std::vector<GLCPointData>);
  void deletePoint(int id);

  const glcpointmap_t& getCPoints() const;
  void setShow(bool show);
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
  void setNameVisible(bool visible);
  const bool isNameVisible();

  void setCpointSize(int id, float size);
  void setCpointCoords(int id, std::array<float, 3> coords);
  void setCpointCoordsX(int id, float x);
  void setCpointCoordsY(int id, float y);
  void setCpointCoordsZ(int id, float z);
  void setCpointText(int id, std::string text);

  static int wwidth;

  static CPointTextRenderer* text_renderer;

protected:
  void genVboData();

  CShader *m_shader;
  glcpointmap_t m_cpoints;
  GLuint m_vao, m_vbo;
  std::string m_name;
  std::array<float, 3> m_color;
  bool m_is_shown;
  bool m_is_filled;
  bool m_is_name_visible;
  int m_threshold;
  CPointsetTypes m_pointset_type;
};

class CPointsetRegularPolygon : public CPointset {
public:
  CPointsetRegularPolygon(CShader *m_shader, std::string name);
  void display();
protected:
  int m_nb_vertices; //TODO make const with constructor
};

class CPointsetDisk : public CPointsetRegularPolygon {
public:
  CPointsetDisk(CShader *m_shader, std::string name);
};

class CPointsetSquare : public CPointsetRegularPolygon {
public:
  CPointsetSquare(CShader *m_shader, std::string name);
};

class CPointsetTag : public CPointset {
public:
  CPointsetTag(CShader *shape_shader, CShader *text_shader, std::string name);
  void display();
  void sendUniforms();
};

class CPointsetSphere : public CPointset {
public:
  CPointsetSphere(CShader *m_shader, std::string name);
  void display();
  void sendUniforms();
//  void setAttributes();

  static int subdivisions;
  static int nb_vertex_per_sphere;
};

class CPointsetManager {
public:
  CPointsetManager();
  ~CPointsetManager();
  void loadFile(std::string filepath);
  void initShaders();
  void displayAll();
  CPointset* createNewCPointset();
  void deleteCPointset(std::string pointset_name);
  void deleteCPoint(std::string pointset_name, int cpoint_id);
  void changePointsetType(std::string pointset_name, std::string new_type);
  static void setW(int w);

  typedef typename std::map<std::string, CPointset *> map_type;
  typedef typename map_type::iterator iterator;
  typedef typename map_type::const_iterator const_iterator;

  inline iterator begin() noexcept { return m_pointsets.begin(); }
  inline const_iterator cbegin() const noexcept { return m_pointsets.cbegin(); }
  inline iterator end() noexcept { return m_pointsets.end(); }
  inline const_iterator cend() const noexcept { return m_pointsets.cend(); }

  CPointset *&operator[](const std::string &name) {
    return m_pointsets[name];
  }
  CPointset *&at(const std::string &name) {
    return m_pointsets.at(name);
  }


  void saveToFile(std::string file_path);

  std::map<CPointsetTypes, std::string> shapeToStr;
  std::map<std::string,CPointsetTypes> strToShape;
  void setPointsetName(std::string old_name, std::string new_name);
private:
  int m_nb_sets;
  CShader *m_regular_polygon_shader, *m_tag_shader, *m_text_shader, *m_sphere_shader;
  std::map<std::string, CPointset *> m_pointsets;
  std::string defaultName() const;
  CPointset *newPointset(std::string str_shape, std::string name);
};

class CPointTextRenderer{
public:
  ~CPointTextRenderer();

  void init();
  void renderText(CPointset *pointset);

private:
  CShader *m_text_shader;
  GLuint m_text_vbo;
  GLuint m_text_vao;

  std::map<GLchar, Character> Characters;
};
} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
