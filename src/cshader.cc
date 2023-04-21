// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2023                                  
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
#include "cshader.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <glwindow.h>
#include <QFile>
#include <QString>
#include <QTextStream>
using namespace glnemo;
using namespace std;
// ============================================================================
// constructor        
CShader::CShader(std::string _vert_file, std::string _frag_file,  std::string _geom_file, bool _v)
{
  vert_file  = _vert_file;
  frag_file  = _frag_file;
  geom_file  = _geom_file;
  verbose = _v;
}

// ============================================================================
// init      
bool CShader::init()
{
    bool ret=false;

    if (processVertex()) {
        if(processPixel()) {
            if (processGeom()) {
                if (createProgram()) {
                    ret=true;
                }
            }
        }
    }
    return ret;
}
// ============================================================================
// start
void CShader::start() {
  //QT6 glUseProgramObjectARB(m_program);
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  f->glUseProgram(m_program);
  
}
// ============================================================================
// stop
void CShader::stop() {
  //QT6  glUseProgramObjectARB(0);
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  f->glUseProgram(0);
}
// ============================================================================
// sendUniformXfv
void CShader::sendUniformXfv(const char * s,const int _dim, const int _count, const float * _v)
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  GLint loc = f->glGetUniformLocation(m_program, s);
  if (loc == -1) {
    std::cerr << "Error occured when sending \""<<s<<"\" to shader..\n";
//    exit(1);
  }
  switch (_dim) {
  case 1: f->glUniform1fv(loc,_count,_v); break;// 
  case 2: f->glUniform2fv(loc,_count,_v); break;// 
  case 3: f->glUniform3fv(loc,_count,_v); break;// 
  case 4: f->glUniform4fv(loc,_count,_v); break;//
  case 16:f->glUniformMatrix4fv(loc,1,false,_v); break;
  default: 
    std::cerr << "CShader::sendUniformXfv unknown dimension ["<<_dim<<"], abort\n";
    std::exit(1);
  }
}// ============================================================================
// sendUniformXfv
void CShader::sendUniformXiv(const char * s,const int _dim, const int _count, const int * _v)
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  GLint loc = f->glGetUniformLocation(m_program, s);
  if (loc == -1) {
    std::cerr << "Error occured when sending \""<<s<<"\" to shader..\n";
//    exit(1);
  }
  switch (_dim) {
  case 1: f->glUniform1iv(loc,_count,_v); break;//
  case 2: f->glUniform2iv(loc,_count,_v); break;//
  case 3: f->glUniform3iv(loc,_count,_v); break;//
  case 4: f->glUniform4iv(loc,_count,_v); break;//
  default:
    std::cerr << "CShader::sendUniformXiv unknown dimension ["<<_dim<<"], abort\n";
    std::exit(1);
  }
}
// ============================================================================
// sendUniformf
void CShader::sendUniformf(const char * s,const float _v)
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  GLint loc = f->glGetUniformLocation(m_program, s);
  if (loc == -1) {
    std::cerr << "CShader::sendUniformf Error occured when sending \""<<s<<"\" to shader..\n";
//    exit(1);
  }
  f->glUniform1f(loc, _v);  
}
// ============================================================================
// sendUniformi
void CShader::sendUniformi(const char * s,const int _v)
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  GLint loc = f->glGetUniformLocation(m_program, s);
  if (loc == -1) {
    std::cerr << "CShader::sendUniformi Error occured when sending \""<<s<<"\" to shader..\n";
//    exit(1);
  }
  f->glUniform1i(loc, _v);  
}
// ============================================================================
// createProgram()      
bool CShader::createProgram()
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  bool ret=true;
  std::cerr << "Creating program\n";
  m_program = f->glCreateProgram();
  std::cerr << "Attaching vertex shader["<<vert_file<<"]\n";
  f->glAttachShader(m_program, m_vertexShader);
  std::cerr << "Attaching pixel shader["<<frag_file<<"]\n";
  f->glAttachShader(m_program, m_pixelShader);
  if (geom_file!="") {
      std::cerr << "Attaching geom shader["<<geom_file<<"]\n";
      f->glAttachShader(m_program, m_geomShader);
  }
  // bind attribute
  //glBindAttribLocation(m_program, 100, "a_sprite_size");
  std::cerr << "Linking  program\n";
  f->glLinkProgram(m_program);
  GLWindow::checkGLErrors("link Shader program");
  printLog(m_program,"Linking  program");
  int  link_status;
  f->glGetProgramiv(m_program, GL_LINK_STATUS, &link_status);
  if(link_status != GL_TRUE) {
    cerr << "Unable to LINK Shader program .....\n";
    return false;
  }
  f->glDeleteShader(m_vertexShader);
  f->glDeleteShader(m_pixelShader);
  if (geom_file!="") {
      f->glDeleteShader(m_geomShader);
  }
  std::cerr << "ending init shader\n";
  return ret;
}
// ============================================================================
// processVertex()      
bool CShader::processVertex()
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  bool ret=true;
  // process VERTEX
  std::string vert_src = load(vert_file);
  if (vert_src.size()>0) {
    m_vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    if (!m_vertexShader) {
      cerr << "Unable to create VERTEX SHADER["<<vert_file<<"]......\n";
      return false;
    }
    const char * v =  vert_src.c_str();
    f->glShaderSource(m_vertexShader, 1, &v , NULL);
    std::cerr << "Compiling vertex shader["<<vert_file<<"]\n";
    GLint compile_status;
    f->glCompileShader(m_vertexShader);
    GLWindow::checkGLErrors("compile Vertex Shader");
    printLog(m_vertexShader,"Compiling vertex shader");
    f->glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE) {
      cerr << "Unable to COMPILE VERTEX SHADER["<<vert_file<<"].....\n";
      return false;
    }
    
  }
  return ret;
}
// ============================================================================
// processPixel()      
bool CShader::processPixel()
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  bool ret=true;
  // process PIXEL
  std::string  frag_src = load(frag_file);
  if (frag_src.size()>0) {
    m_pixelShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    if (!m_pixelShader) {
      cerr << "Unable to create PIXEL SHADER["<<frag_file<<"].....\n";
      return false;
    }
    const char * v =  frag_src.c_str();
    f->glShaderSource(m_pixelShader, 1, &v , NULL);
    std::cerr << "Compiling pixel shader["<<frag_file<<"]\n";
    GLint compile_status;
    f->glCompileShader(m_pixelShader);
    GLWindow::checkGLErrors("compile Pixel Shader");
    printLog(m_pixelShader,"Compiling pixel shader");
    f->glGetShaderiv(m_pixelShader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE) {
      cerr << "Unable to COMPILE PIXEL SHADER["<<frag_file<<"]......\n";
      return false;
    }
  }
  return ret;
}
// ============================================================================
// processGeom()
bool CShader::processGeom()
{
  bool ret=true;
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  // process PIXEL
  std::string  geom_src = load(geom_file);
  if (geom_file!= "" && geom_src.size()>0) {
    m_geomShader = f->glCreateShader(GL_GEOMETRY_SHADER);
    if (!m_geomShader) {
      cerr << "Unable to create GEOMETRY SHADER["<<geom_file<<"]......\n";
      return false;
    }
    const char * v =  geom_src.c_str();
    f->glShaderSource(m_geomShader, 1, &v , NULL);
    std::cerr << "Compiling geometry shader\n";
    GLint compile_status;
    f->glCompileShader(m_geomShader);
    GLWindow::checkGLErrors("compile geometry Shader");
    printLog(m_geomShader,"Compiling geometry shader");
    f->glGetShaderiv(m_geomShader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE) {
      cerr << "Unable to COMPILE GEOMETRY SHADER["<<geom_file<<"].....\n";
      return false;
    }
  }
  return ret;
}
// ============================================================================
// load      
std::string CShader::load(std::string filename)
{
    std::ifstream fi;
    std::string src="";
    if (filename!="") {
        // Open file
        QFile infile(filename.c_str());
        if (!infile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cerr << "CShader::load Unable to open file ["<<filename<<"] for reading, ...\n";
            std::exit(1);
        }
        else {
            QTextStream in(&infile);
            QString line;
            do {
                line = in.readLine();
                if (!line.isNull()) {
                    src = src + "\n" + line.toStdString();
                }
            } while (!line.isNull());
            infile.close();
        }
    }
    return src;
}
// ============================================================================
// printLog      
void CShader::printLog(GLuint obj,std::string s)
{
  int infologLength = 0;
  int maxLength;
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  
  if(f->glIsShader(obj))
    f->glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
  else
    f->glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
  
  char infoLog[maxLength];
  
  if (f->glIsShader(obj))
    f->glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
  else
    f->glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
  
  if (infologLength > 0) {
    std::cerr << "CShader::printLog [" << s<< "]:"<<infoLog<<"\n";
  }
  
}
