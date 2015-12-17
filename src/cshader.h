// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2015                                  
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
#ifndef CSHADER_H
#define CSHADER_H
#include <string>
#include <GL/glew.h>

namespace glnemo {
class CShader {
  
public:
  CShader(std::string vert_file, std::string frag_file, std::string geom_file="",bool verbose=true);
  bool init();
  void start();
  void stop();
  void sendUniformf(const char *,const float v1);
  void sendUniformi(const char *,const int   v1);  
  void sendUniformXfv(const char *,const int _dim, const int _count, const float * _v);
  GLhandleARB getProgramId() {
    return m_program;
  }

private:  
  bool verbose;
  std::string vert_file;
  std::string frag_file;
  std::string geom_file;

  std::string load(std::string); // load source code file
  bool processVertex();
  bool processPixel();
  bool processGeom();
  bool createProgram();
  void printLog(GLuint obj,std::string);
  
  // This handle stores our vertex shader information
  GLhandleARB m_vertexShader;
  // This handle stores our fragment shader information
  GLhandleARB m_pixelShader;
  // This handle stores our geometry shader information
  GLhandleARB m_geomShader;
  // handle to the shader program itself
  GLhandleARB m_program;
  
};

}
#endif // CSHADER_H
