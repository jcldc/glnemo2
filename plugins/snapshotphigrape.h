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
#ifndef SNAPSHOTPHIBAR_H
#define SNAPSHOTPHIBAR_H
#include <QObject>
#include <string>
#include "snapshotinterface.h"
#include "zlib.h"
#include "globaloptions.h"

namespace glnemo {
  using namespace std;
class SnapshotPhiGrape: public SnapshotInterface
{
  Q_OBJECT
  Q_INTERFACES(glnemo::SnapshotInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "fr.glnemo2.phigrapePlugin" FILE "phigrapePlugin.json")
#endif

public:
    SnapshotPhiGrape();

    ~SnapshotPhiGrape();
    
	SnapshotInterface * newObject(const std::string _filename, const int x=0);
    ComponentRangeVector * getSnapshotRange();
    bool isValidData();
    int initLoading(GlobalOptions * so);
    int nextFrame(const int * index_tab, const int nsel);
    int close();
    QString endOfDataMessage();
 private:
    bool valid;
    gzFile           file;               // file handle for compressed file
    bool detectHeader();
    int full_nbody, frame_number;

  // Buffer stuffs
  char * BUFF;
  char line[300];
  unsigned int size_buff;
  std::string sbuff; // string to store the buffer

  // ============================================================================
  // getLine()
  inline bool getLine()
  {
    bool status = false;
    size_t found=sbuff.find('\n'); // look for \n
    if (found!=string::npos) { // \n found
      memcpy(line,sbuff.c_str(),found); // build new line
      line[found]='\0';       // null character terminaison
      sbuff.erase(0,found+1); // erase from beginning to \n
      status=true;
    }
    return status;
  }
  bool readBuffer();
  bool gzGetLine();
};

}
#endif // SNAPSHOTPHIBAR_H
