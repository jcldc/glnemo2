// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2011                                  
// e-mail:   Jean-Charles.Lambert@oamp.fr                                      
// address:  Dynamique des galaxies                                            
//           Laboratoire d'Astrophysique de Marseille                          
//           P�le de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 6110                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
/**
	@author Jean-Charles Lambert <jean-charles.lambert@oamp.fr>
*/
#ifndef GLNEMOSNAPSHOTRAMSES_H
#define GLNEMOSNAPSHOTRAMSES_H
#include <QObject>
#include "snapshotinterface.h"
#include "camr.h"
#include "cpart.h"
#include "globaloptions.h"

namespace glnemo {

class SnapshotRamses: public QObject,
                      public SnapshotInterface
{
    Q_OBJECT
    Q_INTERFACES(glnemo::SnapshotInterface);

public:
    SnapshotRamses();

    ~SnapshotRamses();
    
    SnapshotInterface * newObject(const std::string _filename, const int x=0);
    ComponentRangeVector * getSnapshotRange();
    bool isValidData();
    int initLoading(GlobalOptions * so);
    int nextFrame(const int * index_tab, const int nsel);
    int close() { return 1;};
    QString endOfDataMessage();

private:
    ramses::CAmr * amr;
    ramses::CPart * part;
    int namr;
    int nstars;
    int ndm;
    bool valid;
    int full_nbody;
    GlobalOptions * go;
    bool take_gas, take_halo, take_stars;
};

}

#endif
