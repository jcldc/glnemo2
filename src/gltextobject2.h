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
#include "ul.h"
#include <GL/gl.h>
#include <QOpenGLExtraFunctions> 
#include <QOpenGLFunctions>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H

// -- Glyph metrics cached after FreeType rasterisation ------------------------
struct Glyph {
    GLuint  textureID;   // GL texture holding the 8-bit bitmap
    glm::ivec2 size;     // bitmap width × height  (pixels)
    glm::ivec2 bearing;  // offset from baseline to top-left of bitmap
    GLuint  advance;     // horizontal advance (in 1/64 px → divide by 64)
};

namespace glnemo { 
class GLWindow;

using namespace std;

struct TextBoundingBox {
    float left;
    float right;
    float top;
    float bottom;
    float width;   // (right - left)
    float height;  // (top - bottom)
};

class GLTextObject2 : public GLObject {
public:
    GLTextObject2(GLuint _m_shader)
        : GLObject(), m_vao(0), m_vbo(0)
        , m_screenW(800), m_screenH(600)
    {
      m_shader = _m_shader;

    }
    GLTextObject2(const GLTextObject2& copie)
        : GLObject(), m_vao(copie.m_vao), m_vbo(copie.m_vbo)
        , m_screenW(copie.m_screenW), m_screenH(copie.m_screenH)
        , m_shader(copie.m_shader), m_glyphs(copie.m_glyphs)
        , m_fontpath(copie.m_fontpath), m_pixelsize(copie.m_pixelsize)
    {
      init(m_fontpath,m_pixelsize);
      printf("m_vao [%d] m_vbo[%d] m_shader[%d]\n",m_vao,m_vbo,m_shader);
    }
    GLTextObject2(bool activated=TRUE);
    
    ~GLTextObject2() { destroy(); }

    // -- Initialise FreeType, rasterise ASCII 32-127, build VAO ---------------
    //  fontPath  : absolute path to a .ttf font file
    //  pixelSize : glyph height in pixels at scale=1.0
    bool init(const std::string& fontPath, unsigned int pixelSize = 24);
  //
    // -- Call this whenever the window is resized ------------------------------
    void setScreenSize(int width, int height)
    {
        m_screenW = width;
        m_screenH = height;
    }

    // return bounding box of a text
    TextBoundingBox getTextBoundingBox(const std::string& text, float x, float y, float scale);

    // -- Render a string -------------------------------------------------------
    //  text  : UTF-8 string (ASCII subset only in this implementation)
    //  x, y  : position in pixels, origin = bottom-left of the window
    //  scale : multiplier applied to the glyph size (1.0 = original pixel size)
    //  color : RGBA, components in [0, 1]
    void draw(const std::string& text,
              float x, float y,
              float scale,
              const glm::vec4& color);
    // -- Release all GPU resources ---------------------------------------------
    void destroy()
    {
        QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
        for (auto& pair : m_glyphs)
            f->glDeleteTextures(1, &pair.second.textureID);
        m_glyphs.clear();

        if (m_vao)    { f->glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
        if (m_vbo)    { f->glDeleteBuffers(1, &m_vbo);       m_vbo = 0; }
        if (m_shader) { f->glDeleteProgram(m_shader);         m_shader = 0; }
    }

    // No copy
    // GLTextObject2(const GLTextObject2&)            = delete;
    // GLTextObject2& operator=(const GLTextObject2&) = delete;
    //
    void setText(const QString &p_label,const QString &p_text);
    void setFont(const std::string f) { m_fontpath = f;};
    int getLabelWidth();
    int getTextWidth();
    int getHeight();
    void setPos(const int,const  int, const int);
    void display();
    private:
    std::map<unsigned char, Glyph> m_glyphs;
    GLuint  m_vao, m_vbo, m_shader;
    int     m_screenW, m_screenH;
    unsigned int m_pixelsize;
    std::string m_fontpath;
    // data
    QString label,text;
    int x,y;      // xy label text position
    int x_text;   // x offset text position

  };
} // namespace
#endif
// ============================================================================
