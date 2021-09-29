// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2020                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           P�le de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
/**
	@author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLNEMOGLGRIDOBJECT_H
#define GLNEMOGLGRIDOBJECT_H
#include "cshader.h"

#include <QGLWidget>
#include <glm/glm.hpp>

namespace glnemo {


class Grid{
public:
  Grid(int, glm::mat4* proj, glm::mat4* view, glm::mat4* model, bool, QColor);
  void genVertexBufferData();
  void display();
  ~Grid();
private:
  void sendShaderData();

  GLuint m_vertexbuffer, m_vertexarray;
  CShader *m_shader;
  bool m_display;
  QColor m_color;
  glm::mat4 *m_proj, *m_view, *m_model;


  int m_nb_lines = 25;
public:
  bool isDisplay() const;
  void setDisplay(bool display);
  const QColor &getColor() const;
  void setColor(const QColor &color);
  int getNbLines() const;
  void setNbLines(int nb_lines);
  float getLineGap() const;
  void setLineGap(float line_gap);
private:
  int m_axis;
  float m_line_gap = 1.0;
};

}

#endif
