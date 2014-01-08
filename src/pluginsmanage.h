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
#ifndef GLNEMOPLUGINSMANAGE_H
#define GLNEMOPLUGINSMANAGE_H
#include <QObject>
#include <string>

namespace glnemo {

// IMPORTANT : it's mandatory to prototype the class which belong to
// the namespace INSIDE the namespace....                           
class SnapshotInterface;

class PluginsManage: public QObject
{
  Q_OBJECT
public:
    PluginsManage();
    ~PluginsManage();

    SnapshotInterface * getObject(const std::string filename);
};

}

#endif
