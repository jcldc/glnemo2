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
	@author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLCUBEOBJECT_H
#define GLCUBEOBJECT_H
#include <QOpenGLWidget>

#include "globject.h"
namespace glnemo {
  
class GLCubeObject : public GLObject {
public:
  GLCubeObject(float,const QColor &c=Qt::yellow, bool activated=true);
    ~GLCubeObject();
    void setSquareSize(float);
    void rebuild() { 
         buildDisplayList(); 
    }
private:
    float square_size;  // size of square
    // method
    void  buildDisplayList(); 
};
}
#endif // GLCUBEOBJECT_H
