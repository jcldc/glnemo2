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
	@author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
*/
#ifndef GLCOLORBAR_H
#define GLCOLORBAR_H
#include <GL/gl.h>
#include <QObject>
#include <QMouseEvent>
#include <QTimer>
#include <QMutex>
#include <QRecursiveMutex>
#include <globaloptions.h>
#include <particlesdata.h>
#include "globjectparticles.h"
#include "vec3d.h"
#include "gltextobject.h"

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <qopenglextrafunctions.h>
#include <vector>
#include <cstdio>
#include <algorithm>

namespace glnemo {


class GLColorbar : public GLObject, protected QOpenGLExtraFunctions
{
  Q_OBJECT
public:
    // Colormap orientation: which axis of the square the colormap runs along.
    enum class Direction { Horizontal, Vertical };

    GLColorbar(const GlobalOptions *, 
               bool activated=true);
    ~GLColorbar();
    bool isEnable()   { return is_activated;}
    void setEnable(bool _b) { is_activated=_b;    }
    void update( GLObjectParticlesVector *,PhysicalData * phys_select,
                GlobalOptions   *, QRecursiveMutex * );      
    void display(const int, const int);  
    // shader managing
    bool init(GLuint shader_program);
    // -- Draw the coloured square ----------------------------------------------
    void draw(float x0, float x1, float y0, float y1);
    // -- Update the colormap texture from three equal-length vectors -----------
    void setColormap(const std::vector<float>& R,
                     const std::vector<float>& G,
                     const std::vector<float>& B);
    // -- Set global opacity (1.0 = opaque, 0.0 = invisible) -------------------
    void setAlpha(float a) { m_alpha = std::clamp(a, 0.f, 1.f); }

    // -- Choose whether the colormap runs horizontally or vertically -----------
    void setDirection(Direction d) { m_direction = d; }

    // -- Call on every window resize -------------------------------------------
    void setScreenSize(int w, int h) { m_screenW = w; m_screenH = h; }
    // -- Release all GPU resources ---------------------------------------------
    void destroy()
    {
        if (m_vao)          { glDeleteVertexArrays(1, &m_vao);      m_vao          = 0; }
        if (m_vbo)          { glDeleteBuffers(1, &m_vbo);            m_vbo          = 0; }
        if (m_texColormap)  { glDeleteTextures(1, &m_texColormap);   m_texColormap  = 0; }
        if (m_shader)       { glDeleteProgram(m_shader);              m_shader       = 0; }
    }

public slots:
    void updateFont();
private:
    const GLObjectParticlesVector * gpv;
    const GlobalOptions * go;
    QRecursiveMutex * mutex_data;
    bool is_activated;
    int width,height;
    void drawBox  ();
    void drawColor();
    void drawLegend();
    void drawText(float value, int fac);
    // font stuffs
    // GLTextObject * legend;    
    // fntTexFont * font;
    int x[4][2];
        
    PhysicalData * phys_select;
    // shader managing
    GLuint m_vao, m_vbo;
    GLuint m_texColormap;
    GLuint m_shader;
    int    m_screenW, m_screenH;
    float  m_alpha;
    Direction m_direction;

};


}
#endif // GLCOLORBAR_H
