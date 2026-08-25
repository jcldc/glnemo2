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
#ifndef GLAXESOBJECT_H
#define GLAXESOBJECT_H

#pragma once
#include "globject.h"
#include "globaloptions.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace glnemo {

struct GizmoVertex {
    glm::vec3 position;
    glm::vec3 normal;
};


class GLAxesObject: public GLObject, protected  QOpenGLFunctions_3_3_Core {
public:
  GLAxesObject(GlobalOptions * _go):GLObject(),go(_go) {};
  ~GLAxesObject() { cleanup() ; }
  // Non-copyable: this class owns raw GL resource handles (VAO/VBO),
  // copying it would duplicate the handles without duplicating the GPU resources.
  // GLAxesObject(const GLAxesObject&) = delete;
  GLAxesObject& operator=(const GLAxesObject&) = delete;

  void init(GLuint shader_program);
  void render(const glm::mat4 &viewRotationOnly, int viewportW, int viewportH);
  void cleanup();

private:
  void buildArrowGeometry(std::vector<GizmoVertex> &verts,
                             float shaftRadius, float shaftLength,
                             float headRadius, float headLength,
                             int segments);

  GLuint vao = 0, vbo = 0;
  int vertexCount = 0;
  GLuint shader = 0;
  bool initialized = false;
  const GlobalOptions * go;
};

} // namespace
#endif // GLAXESOBJECT_H
