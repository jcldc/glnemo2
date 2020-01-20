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
#include <glm/glm.hpp>

using json = nlohmann::json;

namespace glnemo {


class CPointTextRenderer;

struct Character {
  glm::vec<4, glm::vec2> TexCoords;  // normalized character position in texture, order : bottom left, top left, top right, bottom right
  glm::ivec2 Size;       // Size of glyph
  glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
  int     Advance;    // Offset to advance to next glyph
};

enum CPointsetShapes {
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
  CPointset(CShader *shader, std::string name = "");
  CPointset(CShader *shader, const CPointset &);
  virtual ~CPointset();
  virtual void display() = 0;
  virtual void setAttributes();
  virtual void sendUniforms();
  void displayText();
  void copyCPoints(const CPointset&);
  bool ready();

  GLCPoint *addPoint(std::array<float, 3> coords, float size, std::string text);
  void addPoints(std::vector<GLCPointData>);
  void deletePoint(int id);

  json toJson();
  void fromJson(json j);

  const glcpointmap_t &getCPoints() const;
  void setVisible(bool visible);
  bool isVisible();
  void setFilled(bool);
  const bool isFilled() const;
  const std::string &getName() const;
  void setName(std::string);
  void setThreshold(int threshold);
  const int getThreshold() const;
  inline int size() { return m_cpoints.size(); }
  CPointsetShapes getPointsetShape() const;
  const QColor getQColor() const;
  const std::array<float, 3> &getColor() const;
  void setColor(const QColor &color);
  void setColor(std::array<float, 3>);
  const CPointsetShapes &getShape() const;
  void setNameVisible(bool visible);
  const bool isNameVisible();
  void setFillratio(float fill_ratio);
  const float getFillratio() const;
  void setNameSizeFactor(float name_size_factor);
  const float getNameSizeFactor() const;
  void setNameOffset(float name_offset);
  const float getNameOffset() const;
  void setNameAngle(int angle);
  const int getNameAngle() const;
  void setNbSphereSections(int nb_sections);
  const int getNbSphereSections() const;

  void setCpointSize(int id, float size);
  void setCpointCoords(int id, std::array<float, 3> coords);
  void setCpointCoordsX(int id, float x);
  void setCpointCoordsY(int id, float y);
  void setCpointCoordsZ(int id, float z);
  void setCpointText(int id, std::string text);

  static int wwidth;

  static std::map<CPointsetShapes, std::string> shapeToStr;
  static std::map<std::string, CPointsetShapes> strToShape;

  static CPointTextRenderer *text_renderer;

protected:
  void genVboData();

  CShader *m_shader;
  glcpointmap_t m_cpoints;
  GLuint m_vao, m_vbo;
  std::string m_name;
  std::array<float, 3> m_color;
  bool m_is_visible;
  bool m_is_filled;
  bool m_is_name_visible;
  int m_threshold;
  float m_fill_ratio;
  float m_name_size_factor;
  float m_name_offset;
  int m_name_angle;

  int m_nb_sphere_sections = 12;

  CPointsetShapes m_shape;
};

class CPointsetRegularPolygon : public CPointset {
public:
  CPointsetRegularPolygon(std::string name);
  CPointsetRegularPolygon(const CPointset &other);
  void display();
  void sendUniforms();

  static CShader *shader;
protected:
  int m_nb_vertices; //TODO make const with constructor
};

class CPointsetDisk : public CPointsetRegularPolygon {
public:
  CPointsetDisk(std::string name);
  CPointsetDisk(const CPointset &other);
};

class CPointsetSquare : public CPointsetRegularPolygon {
public:
  CPointsetSquare(std::string name);
  CPointsetSquare(const CPointset &other);
};

class CPointsetTag : public CPointset {
public:
  CPointsetTag(std::string name);
  CPointsetTag(const CPointset &other);
  void display();
  void sendUniforms();

  static CShader *shader;
};

class CPointsetSphere : public CPointset {
public:
  CPointsetSphere(std::string name);
  CPointsetSphere(const CPointset &other);
  void display();
  void sendUniforms();
//  void setAttributes();
  static CShader *shader;
};

class CPointsetManager {
public:
  CPointsetManager();
  ~CPointsetManager();
  int loadFile(std::string filepath);
  void initShaders();
  void displayAll();
  CPointset *createNewCPointset();
  void deleteCPointset(std::string pointset_name);
  void deleteCPoint(std::string pointset_name, int cpoint_id);
  CPointset * changePointsetShape(CPointset *pointset, std::string new_shape);
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

  void setPointsetName(std::string old_name, std::string new_name);
private:
  int m_nb_sets;
  std::map<std::string, CPointset *> m_pointsets;
  std::string defaultName() const;
  CPointset *newPointset(std::string str_shape, std::string name);
  CPointset *newPointset(std::string str_shape, const CPointset &);
};

class CPointTextRenderer {
public:
  ~CPointTextRenderer();

  void init();
  void renderText(CPointset *pointset);

private:
  CShader *m_text_shader;
  GLuint m_text_vbo;
  GLuint m_text_vao;
  GLuint m_texture;

  std::map<int, Character> characters;
};
} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
