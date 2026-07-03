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
//                                                                             
// GLTextObject class definition                                               
//                                                                             
// Manage OpenGL Text Object used on the On Screen Display Display             
// ============================================================================
#ifndef GL_TEXT_OBJECT2_H
#define GL_TEXT_OBJECT2_H

//#include <qgl.h>
#include "globject.h"
#include "gltextrender.h"
#include "ul.h"
#include <GL/gl.h>
#include <QOpenGLExtraFunctions> 
#include <QOpenGLFunctions>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include FT_FREETYPE_H

namespace glnemo { 
class GLWindow;

using namespace std;

class GLTextObject2 : public GLObject {
public:
    GLTextObject2(GLTextRender * _gtr)        
        : GLObject(),m_gtr(_gtr)
    {
    }
    GLTextObject2(bool activated=TRUE);
    
    ~GLTextObject2() { };

    void setText(const QString &p_label,const QString &p_text);
    int getLabelWidth();
    int getTextWidth();
    int getHeight();
    void setPos(const int,const  int, const int);
    void display();
    QString getLabel() { return label;}
    QString getText() { return text;}
    private:
    // data
    QString label,text;
    int x,y;      // xy label text position
    int x_text;   // x offset text position
    // OpenGL Text Rendering object 
    GLTextRender * m_gtr;

  };
} // namespace
#endif
// ============================================================================
