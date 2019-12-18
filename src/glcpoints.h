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

using json = nlohmann::json;

#define NB_POINTS 100

namespace glnemo {
//    class GLCPointSet;

enum Shape { disk, annulus };

class GLCPoint {
public:
  GLCPoint(std::array<float, 3> coords, float size);
  const std::array<float, 3> &getCoords() const;
  const float &getSize() const;
  virtual std::vector<float> getPackedData() = 0;
  static void display(CShader *shader);

protected:
  const int nb_vertices = 100;
  std::array<float, 3> m_coords;
  float m_size;
};

class GLCPointAnnulus : public GLCPoint {
public:
  GLCPointAnnulus(std::array<float, 3> coords, float size, float fill_ratio);
  ~GLCPointAnnulus();
  std::vector<float> getPackedData();
  static void setAttributes(CShader *shader);
  static void displayAll(CShader *shader, std::vector<GLCPoint *> &cpoints);

private:
  float m_fill_ratio;
};

class GLCPointDisk : public GLCPoint {
public:
  GLCPointDisk(std::array<float, 3> coords, float size);
  ~GLCPointDisk();
  std::vector<float> getPackedData();
  static void setAttributes(CShader *shader);
  static void displayAll(CShader *shader, std::vector<GLCPoint *> &cpoints);

private:
};

class GLCPointSet {
public:
  GLCPointSet(Shape shape, CShader *shader, std::string name = "");
  ~GLCPointSet();

  void initVboData();
  bool ready();
  void display();
  void hide();
  void show();
  bool isShown();
  void addPoint(GLCPoint *point);
  const std::string &getName() const;
  int setThreshold(int threshold);

private:
  CShader *m_shader;
  std::vector<GLCPoint *> m_cpoints;
  GLuint m_vao, m_vbo;
  std::vector<float> m_data;
  std::string m_name;
  Shape m_shape;
  QColor m_color;
  bool m_is_shown;
  int m_threshold;
  void (*setAttributesFn)(CShader *);
  void (*displayAllFn)(CShader *, std::vector<GLCPoint *> &);
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
