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
	@author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
 */
#ifndef GLNEMOORBITS_H
#define GLNEMOORBITS_H
#include <iostream>
#include <list>
#include "globject.h"
#include "particlesdata.h"
#include <QObject>

namespace glnemo {

class Orbits;
typedef std::list <Orbits> OrbitsList;
//class GLObject;

// Class Orbits used to manage the orbits
class Orbits : public GLObject {
  public:
    Orbits(const ParticlesData *, const int);
    Orbits(const Orbits&);
    Orbits();
    ~Orbits();
    const Orbits& operator=(const Orbits&);
    // methods
    inline float x() { return pos[0];}
    inline float y() { return pos[1];}
    inline float z() { return pos[2];}
  private:
    // members
    float time; //  current time                
    float pos[3];//  orbits coordinates
    //void buildDisplayList();
  };

}

#endif
