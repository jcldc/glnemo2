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
#ifndef GLNEMOSNAPSHOTFITS_H
#define GLNEMOSNAPSHOTFITS_H
#include <QObject>
#include "snapshotinterface.h"
#include <CCfits>
#include "globaloptions.h"
namespace glnemo {


class SnapshotFits: public SnapshotInterface{
  Q_OBJECT
  Q_INTERFACES(glnemo::SnapshotInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "fr.glnemo2.fitsPlugin" FILE "fitsPlugin.json")
#endif

public:
    SnapshotFits();

    ~SnapshotFits();
    SnapshotInterface * newObject(const std::string _filename, const int x=0);
    bool isValidData();
    ComponentRangeVector * getSnapshotRange();
    int initLoading(GlobalOptions * so);
    int nextFrame(const int * index_tab, const int nsel);
           // index_tab = array of selected indexes (size max=part_data->nbody)
           // nsel      = #particles selected in index_tab
           // particles not selected must have the value '-1'
    int close();
    QString endOfDataMessage();

private:
    bool valid;
    CCfits::FITS * pInfile;
    CCfits::HDU * image;
    std::valarray<float>  contents;
    bool take_gas, take_halo, take_stars;
};

}

#endif

