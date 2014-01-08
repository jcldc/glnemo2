// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2014                                  
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
#ifndef GLAXESOBJECT_H
#define GLAXESOBJECT_H
#include "globject.h"
#include <GL/glu.h>

namespace glnemo {

class GLAxesObject: public GLObject {
public:
  GLAxesObject();
  ~GLAxesObject();
  void display(const double * mScreen,const double * mScene, const int width, const int height, 
               const int loc=0, const float psize=0.1, const bool perspective=true);
private:
  void buildDisplayList();
  void buildDisplayList2();
  void buildArrow(const float length, const float radius, const int nbSubdivisions);
  GLUquadric *quadric;
};

} // namespace
#endif // GLAXESOBJECT_H
