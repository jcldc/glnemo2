// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2018                                  
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

#include "globject.h"
#include "glgridobject.h"
#include "globaloptions.h"
namespace glnemo {


class GLNewGridObject{
public:
  GLNewGridObject(glm::mat4*, glm::mat4*);
  void genVertexBufferData();
  void display();
  ~GLNewGridObject();
private:
  void sendShaderData();

  GLuint m_vertexbuffer, m_vertexarray;
  CShader *m_shader;
  glm::mat4* m_proj;
  glm::mat4* m_model;

  int m_nb_lines = 25;
  float m_line_gap = 1.0;
};

class GLGridObject: public GLObject {
public:

  GLGridObject(int axe_parm=0,const QColor &c=Qt::yellow, bool activated=true);

  ~GLGridObject();

  // method
  void setNbSquare(int _nsquare) { nsquare = _nsquare;}
  void setSquareSize(float _square_size) { square_size = _square_size;}
  void rebuild() { buildDisplayList(); }
  static int nsquare;        // #square
  static float square_size;  // size of square

 private:
  int axe;
  // method
  void  buildDisplayList( );
};

}

#endif
