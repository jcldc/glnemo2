// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2017
// e-mail:   Jean-Charles.Lambert@lam.fr
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)
//           Laboratoire d'Astrophysique de Marseille
//           Pôle de l'Etoile, site de Château-Gombert
//           38, rue Frédéric Joliot-Curie
//           13388 Marseille cedex 13 France
//           CNRS U.M.R 7326
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".
// ============================================================================
#ifndef CSNAPSHOTGADGETH5_H
#define CSNAPSHOTGADGETH5_H

/**
    @author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
*/
#include <QObject>
#include "snapshotinterface.h"
#include "globaloptions.h"
#include "gadgeth5.h"

namespace glnemo {

class SnapshotGadgetH5: public SnapshotInterface{
  Q_OBJECT
  Q_INTERFACES(glnemo::SnapshotInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "fr.glnemo2.gadgeth5plugin" FILE "gadgeth5plugin.json")
#endif

public:
  SnapshotGadgetH5();
  ~SnapshotGadgetH5();

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
  GH5<float> * myH5;
  void storeComponents();
  template <class U>
  bool loadCommonDataset(std::string tag,U * data, const int dim);
  template <class U>
  bool loadDataset(std::string tag,U * data, const int comp_id, const int dim);
};
}
#endif // CSNAPSHOTGADGETH5_H
