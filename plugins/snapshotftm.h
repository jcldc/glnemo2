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
#ifndef GLNEMOSNAPSHOTFTM_H
#define GLNEMOSNAPSHOTFTM_H
#include <QObject>
#include "snapshotinterface.h"
#include "globaloptions.h"
#include "ftmio.h"
namespace glnemo {


class SnapshotFtm: public SnapshotInterface
{
  Q_OBJECT
  Q_INTERFACES(glnemo::SnapshotInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "fr.glnemo2.nemoPlugin" FILE "ftmPlugin.json")
#endif

public:
    SnapshotFtm();

    ~SnapshotFtm();
    
	SnapshotInterface * newObject(const std::string _filename, const int x=0);
    ComponentRangeVector * getSnapshotRange();
    bool isValidData();
    int initLoading(GlobalOptions * so);
    int nextFrame(const int * index_tab, const int nsel);
    int close();
    QString endOfDataMessage();

private:
    int full_nbody;
    bool valid;
    ftm::FtmIO * ftm_io;
};

}

#endif
