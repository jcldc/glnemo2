// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2026                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           Pole de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
/**
	@author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLNEMOGLCUBEOBJECT2_H
#define GLNEMOGLCUBEOBJECT2_H
#include <QOpenGLWidget>
#include <QtOpenGL>
#include <QOpenGLExtraFunctions> 
#include <QOpenGLFunctions>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <qcolor.h>

#include "globject.h"

namespace glnemo {


class GLCubeObject2: public GLObject {
public:

  GLCubeObject2( float square_size=1.0, const QColor &c=Qt::yellow, bool activated=true)
          : m_squareSize(square_size)
        , m_vao(0)
        , m_vbo(0)
        , m_vertexCount(0)
    {
      
        // Set color
        setColor(c);
   }

  ~GLCubeObject2();
  void build();
  void rebuild(float square_size);
  void setColor(const QColor& color);

  void draw(GLuint shaderProgram,
              const glm::mat4& model,
              const glm::mat4& view,
              const glm::mat4& projection) const;

  void setVec4FromQColor(glm::vec4& vec, const QColor& color) {
      vec.r = color.redF();
      vec.g = color.greenF();
      vec.b = color.blueF();
      vec.a = color.alphaF();
  }
  void setSquareSize(float _square_size) { m_squareSize = _square_size;}
  void destroy();

private:
    float     m_squareSize;
    glm::vec4    m_color;
    GLuint    m_vao;
    GLuint    m_vbo;
    GLsizei   m_vertexCount;
};
}

#endif
