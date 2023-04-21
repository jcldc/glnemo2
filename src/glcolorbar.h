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
/**
	@author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
*/
#ifndef GLCOLORBAR_H
#define GLCOLORBAR_H
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

namespace glnemo {


class GLColorbar : public GLObject
{
  Q_OBJECT
public:
    GLColorbar(const GlobalOptions *, 
               bool activated=true);
    ~GLColorbar();
    bool isEnable()   { return is_activated;}
    void setEnable(bool _b) { is_activated=_b;    }
    void update( GLObjectParticlesVector *,PhysicalData * phys_select,
                GlobalOptions   *, QRecursiveMutex * );      
    void display(const int, const int);  
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
    GLTextObject * legend;    
    fntTexFont * font;
    int x[4][2];
        
    PhysicalData * phys_select;

};


}
#endif // GLCOLORBAR_H
