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
  std::array<std::array<float, 2>, 4> TexCoords;  // normalized character position in texture, order : bottom left, top left, top right, bottom right
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
  GLCPoint(std::array<float, 3> coords, float size, const std::string& text);
  const std::array<float, 3> &getCoords() const;
  const float &getSize() const;
  const std::string &getName() const;
  const int &getId() const;
  bool isSelected() const;

  inline bool operator<(const GLCPoint &other) const { return m_size > other.getSize(); }
  inline bool operator>(const GLCPoint &other) const { return *this < other; }
  inline bool operator<=(const GLCPoint &other) const { return !(other > *this); }
  inline bool operator>=(const GLCPoint &other) const { return !(other < *this); }

private:
  void setSize(float size);
  void setCoords(std::array<float, 3>);
  void setName(const std::string &name);
  void select();
  void unselect();

  std::array<float, 3> m_coords;
  float m_size;
  std::string m_name;
  int m_id;
  bool m_is_selected;

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
  explicit CPointset(CShader *shader, const std::string &name);
  CPointset(CShader *shader, const CPointset &);
  virtual ~CPointset();
  virtual void display() = 0;
  virtual void setAttributes();
  virtual void sendUniforms();
  void displayText();
  void copyCPoints(const CPointset&);
  bool ready();

  GLCPoint *addPoint(std::array<float, 3> coords, float size, const std::string &text);
  void addPoints(const std::vector<GLCPointData>&);
  void deletePoint(int id);
  void select();
  void unselect();
  void selectCPoint(int id);
  void unselectCPoint(int id);
  virtual std::pair<GLCPoint*, float> getClickedCPoint(double *model, double *proj, glm::vec2 click_coords,
                                          int *viewport, int dof);

  json toJson();
  void fromJson(json j);

  const glcpointmap_t &getCPoints() const;
  void setVisible(bool visible);
  bool isVisible();
  const std::string &getName() const;
  void setName(const std::string &new_name);
  void setThreshold(int threshold);
  int getThreshold() const;
  inline int size() { return m_cpoints.size(); }
  CPointsetShapes getPointsetShape() const;
  QColor getQColor() const;
  const std::array<float, 3> &getColor() const;
  void setColor(const QColor &color);
  void setColor(std::array<float, 3>);
  const CPointsetShapes &getShape() const;
  void setNameVisible(bool visible);
  bool isNameVisible();
  void setFillratio(float fill_ratio);
  float getFillratio() const;
  void setNameSizeFactor(float name_size_factor);
  float getNameSizeFactor() const;
  void setNameOffset(float name_offset);
  float getNameOffset() const;
  void setNameAngle(int angle);
  int getNameAngle() const;
  void setNbSphereSections(int nb_sections);
  int getNbSphereSections() const;
  int getNbCpoints() const;

  void setCpointSize(int id, float size);
  void setCpointCoords(int id, std::array<float, 3> coords);
  void setCpointCoordsX(int id, float x);
  void setCpointCoordsY(int id, float y);
  void setCpointCoordsZ(int id, float z);
  void setCpointText(int id, const std::string &text);

  static int wwidth;
  static int wheight;

  static std::map<CPointsetShapes, std::string> shapeToStr;
  static std::map<std::string, CPointsetShapes> strToShape;

  static CPointTextRenderer *text_renderer;

  static glm::mat4 *m_projection_matrix_ptr, *m_view_matrix_ptr;

protected:
  void genVboData();

  CShader *m_shader;
  glcpointmap_t m_cpoints;
  GLuint m_vao, m_selected_vao, m_vbo, m_selected_vbo;
  std::string m_name;
  std::array<float, 3> m_color;
  bool m_is_visible;
  bool m_is_name_visible;
  int m_threshold;
  float m_fill_ratio;
  float m_name_size_factor;
  float m_name_offset;
  int m_name_angle;
  int m_nb_selected;
  int m_nb_sphere_sections = 12;
  CPointsetShapes m_shape;

  static const std::array<float, 3> selected_color;
};

class CPointsetRegularPolygon : public CPointset {
public:
  explicit CPointsetRegularPolygon(const std::string &name);
  explicit CPointsetRegularPolygon(const CPointset &other);
  void display() override;
  void sendUniforms() override;

  static CShader *shader;
protected:
  int m_nb_vertices; //TODO make const with constructor
};

class CPointsetDisk : public CPointsetRegularPolygon {
public:
  explicit CPointsetDisk(const std::string &name);
  explicit CPointsetDisk(const CPointset &other);
};

class CPointsetSquare : public CPointsetRegularPolygon {
public:
  explicit CPointsetSquare(const std::string &name);
  explicit CPointsetSquare(const CPointset &other);
  std::pair<GLCPoint*, float> getClickedCPoint(double *model, double *proj, glm::vec2 click_coords,
                                          int *viewport, int dof) override;
};

class CPointsetTag : public CPointset {
public:
  explicit CPointsetTag(const std::string &name);
  explicit CPointsetTag(const CPointset &other);
  void display() override;
  void sendUniforms() override;

  static CShader *shader;
};

class CPointsetSphere : public CPointset {
public:
  explicit CPointsetSphere(const std::string &name);
  explicit CPointsetSphere(const CPointset &other);
  void display() override;
  void sendUniforms() override;
  std::pair<GLCPoint*, float> getClickedCPoint(double *model, double *proj, glm::vec2 click_coords,
                                          int *viewport, int dof) override;

//  void setAttributes();
  static CShader *shader;
};

class CPointsetManager {
public:
  CPointsetManager();
  ~CPointsetManager();
  void loadFile(const std::string &filepath);
  static void initShaders(bool glsl_130);
  void displayAll();
  CPointset *createNewCPointset();
  void deleteCPointset(const std::string &pointset_name);
  void deleteCPoint(const std::string &pointset_name, int cpoint_id);
  CPointset * changePointsetShape(CPointset *pointset, const std::string &new_shape);
  static void setScreenDim(int wwidth, int wheight);
  void unselectAll();
  std::pair<CPointset *, GLCPoint*> getClickedCpoint(double *model, double *proj, glm::vec2 click_coords,
                                          int *viewport, int dof);
  static void setMatricesPointer(glm::mat4*, glm::mat4*);

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

  void saveToFile(const std::string &file_path);

  void setPointsetName(const std::string &old_name, const std::string &new_name);
private:
  int m_next_set_id;
  std::map<std::string, CPointset *> m_pointsets;
  std::string defaultName() const;
  static std::string defaultShape() ;
  static CPointset *newPointset(const std::string &str_shape, const std::string &name);
  static CPointset *newPointset(const std::string &str_shape, const CPointset &other);
};

class CPointTextRenderer {
public:
  ~CPointTextRenderer();

  void init(const std::string &shader_dir);
  void renderText(CPointset *pointset);

  glm::mat4 *m_projection_matrix_ptr = nullptr, *m_view_matrix_ptr = nullptr;
private:
  CShader *m_text_shader;
  GLuint m_text_vbo;
  GLuint m_text_vao;
  GLuint m_texture;

  std::map<int, Character> characters;
};
} // namespace glnemo
#endif // GLNEMO2_GLCPOINTS_H
