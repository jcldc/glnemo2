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
#include <QtGui> // Mandatory for plugins management
#include <sstream>
#include "snapshotgadgeth5.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
Q_PLUGIN_METADATA(IID "fr.glnemo2.gadgetH5Plugin")
#endif

namespace glnemo {

// ============================================================================
// Constructor
SnapshotGadgetH5::SnapshotGadgetH5()
{
  valid=false;
  part_data = NULL;
  part_data = new ParticlesData();
  myH5 = NULL;
  interface_type="Gadget3";

}
// ============================================================================
// Destructor
SnapshotGadgetH5::~SnapshotGadgetH5()
{
  if (obj) delete obj;
  if (myH5) delete myH5;
  if (part_data) delete part_data;
}
// ============================================================================
// newObject
// instantiate a new object and return a pointer on it
SnapshotInterface * SnapshotGadgetH5::newObject(const std::string _filename, const int x)
{
  if (x) {;} // get rid of compiler warning
  filename = _filename;
  obj      = new SnapshotGadgetH5();
  std::cerr << "SnapshotGadgetH5:: " << _filename << "\n";
  obj->setFileName(_filename);

  return obj;
}
// ============================================================================
// isValidData()
// return true if it's a Tipsy snapshot.
bool SnapshotGadgetH5::isValidData()
{
  valid=false;
  try {
    /*
     * Turn off the auto-printing when failure occurs so that we can
     * handle the errors appropriately
     */
    Exception::dontPrint();
    myH5 = new GH5<float>(filename,H5F_ACC_RDONLY,false);
    valid=true;
    interface_type="Gadget"+std::to_string(myH5->getGadgetNumber());
  }
  catch (...) {
    valid=0;
    std::cerr << "WARNING: not a Gadget3 (HDF5) file...\n";
  }
  return valid;
}
// ============================================================================
// getSnapshotRange
ComponentRangeVector * SnapshotGadgetH5::getSnapshotRange()
{
  crv.clear();
  if (valid) {
    storeComponents();
    ComponentRange::list(&crv);
    if (first) {
      first       = false;
      crv_first   = crv;
      nbody_first = myH5->getNpartTotal();
      time_first  = (float) myH5->getHeader().Time;
    }
  }
  return &crv;
}
// ============================================================================
// initLoading()
int SnapshotGadgetH5::initLoading(GlobalOptions * so)
{
  load_vel = so->vel_req;
  select_time = so->select_time;
  std::cerr << "select_time ="<<select_time<<"\n";
  go = so;

  return 1;
}
// ============================================================================
// nextFrame()
int SnapshotGadgetH5::nextFrame(const int * index_tab, const int nsel)
{
  int status=0;
  stv.clear();
  parseSelectTime();
  load_vel = go->vel_req;

  if (valid && checkRangeTime((float) myH5->getHeader().Time)) {
    status=1;
    if (nsel > *part_data->nbody) {
      //pos
      if (part_data->pos) delete [] part_data->pos;
      part_data->pos = new float[nsel*3];
      //vel
      if (load_vel) {
        if (part_data->vel) delete [] part_data->vel;
        part_data->vel = new float[nsel*3];
      }
      //rho
      if (part_data->rho) delete part_data->rho;
      part_data->rho = new PhysicalData(PhysicalData::rho,nsel);
      for (int i=0; i<nsel; i++) part_data->rho->data[i]=-1.;
      //rneib
      if (part_data->rneib) delete part_data->rneib;
      part_data->rneib = new PhysicalData(PhysicalData::neib,nsel);
      for (int i=0; i<nsel; i++) part_data->rneib->data[i]=-1.;
      //temp
      if (part_data->temp) delete part_data->temp;
      part_data->temp = new PhysicalData(PhysicalData::temperature,nsel);
      for (int i=0; i<nsel; i++) part_data->temp->data[i]=-1.;
      //Ids
      part_data->id.clear();
      for (int i=0; i<nsel; i++) part_data->id.push_back(-1);
    }
    *part_data->nbody = nsel;
    // load positions
    try {
      loadCommonDataset("Coordinates",part_data->pos,3);
    } catch (...) {
      std::cerr << "Coordinates loading failed...\n";
    }
    if (load_vel) {
      try {
        loadCommonDataset("Velocities",part_data->vel,3);
      }
      catch (int e) {
        if (e==-666) {
          loadCommonDataset("Velocity",part_data->vel,3);
        }
      }
    }
    try {
      loadCommonDataset("ParticleIDs",&part_data->id[0],1);
    } catch (...) {
      std::cerr << "ParticleIDs loading failed...\n";
    }

    if ((myH5->getHeader().NumPart_Total[0]>0 ) &&
        (go->select_part=="all" || (go->select_part.find("gas")!=std::string::npos))) {
      loadDataset("/PartType0/Density",part_data->rho->data,0,1);
      try {
        loadDataset("/PartType0/SmoothingLength",part_data->rneib->data,0,1);
      } catch (int e) {
        if (e==-1) { // there are no Smoothing length
          for (int i=0; i<nsel; i++) part_data->rneib->data[i]=0.001;
        }
      }
    }

    part_data->computeVelNorm();
    part_data->rho->computeMinMax();
    part_data->temp->computeMinMax();
    part_data->rneib->computeMinMax();
    if ( ! part_data->rho->isValid()) {
      if (part_data->rho) delete part_data->rho;
      part_data->rho=NULL;
      if (part_data->rneib) delete part_data->rneib;
      part_data->rneib=NULL;
      if (part_data->temp) delete part_data->temp;
      part_data->temp=NULL;
    }
    if (! part_data->timu ) part_data->timu = new float;
    *part_data->timu = (float) myH5->getHeader().Time;
    end_of_data=true; // only one frame from an gadget snapshot
  } else {
    end_of_data=true;
  }
  return status;
}
// ============================================================================
// loadCommonDataset
template <class U>
bool SnapshotGadgetH5::loadCommonDataset(std::string tag,U * data, const int dim)
{
  struct mystruct {
    std::string comp;
    const int index;
  };
  mystruct ms[6]={ "gas",0,"halo",1,"disk",2,"bulge",3,"stars",4,"bndry",5};

  bool ok=false;
  int npartOffset[6];
  unsigned int npart_loaded[6];
  // compute offset in case of multiple gadget file
  int nextoffset=0;
  for (int i=0;i<=5;i++) {
    npart_loaded[i]=0;
    if (go->select_part=="all" || (go->select_part.find(ms[i].comp)!=std::string::npos)) {
      bool ok=false;
      for (unsigned int j=0; j < crv.size(); j++) {
        if (crv[j].type==ms[i].comp) {
          ok=true;
          break;
        }
      }
      if (ok) {
        //npartOffset[i] = npartOffset[i-1]+myH5->getHeader().NumPart_Total[i-1];
        npartOffset[i] = nextoffset;
        nextoffset+=myH5->getHeader().NumPart_Total[i];
        //      std::cerr
        //          << "npartOffset["<<i<<"]="<<npartOffset[i]<<" npartOffset["<<i-1<<"]="
        //          <<npartOffset[i-1]<<" header.npartTotal["<<i-1<<"]="<<myH5->getHeader().NumPart_Total[i-1]<<"\n";
      } else {
        npartOffset[i]=0;
      }
    } else {
      npartOffset[i]=0;
    }
  }

  GH5<float> * dataH5=NULL;

  for (int ifile=0; ifile<myH5->getHeader().NumFilesPerSnapshot; ifile++) {
    if (myH5->getHeader().NumFilesPerSnapshot>1) {
      // create file name
      std::size_t f1= filename.find(".hdf5"); // search ".hdf5"
      std::size_t f2= filename.find_last_of(".",f1-1); // search last "."
      std::ostringstream stm;
      stm << "." << ifile << ".hdf5";
      std::string myfile=filename.substr(0,f2);
      myfile = myfile + stm.str();
      std::cerr << "myfile = " << myfile << "\n";
      try {
        Exception::dontPrint();
        dataH5 = new GH5<float>(myfile,H5F_ACC_RDONLY,false);
      } catch (...) {
        std::cerr << "ERROR !! [" << myfile << "] not a Gadget3 (HDF5) file...\n";
      }
    } else {
      dataH5 = myH5;
    }
    for (int i=0; i<6; i++) {
      if (go->select_part=="all" || (go->select_part.find(ms[i].comp)!=std::string::npos)) {
        bool ok=false;
        for (unsigned int j=0; j < crv.size(); j++) {
          if (crv[j].type==ms[i].comp) {
            ok=true;
            break;
          }
        }
        if (ok) {
          std::ostringstream myid;
          myid << ms[i].index;
          std::string dataset="/PartType"+myid.str()+"/"+tag;
          if (1)  std::cerr << dataset << "\n";
          try {
            U dummy=(U) 1;
            std::vector<U> vec=dataH5->getDataset(dataset,dummy);
            npart_loaded[i]+=vec.size()/dim;
            assert(npart_loaded[i]<=myH5->getHeader().NumPart_Total[i]);
            memcpy(data+npartOffset[i]*dim,&vec[0],vec.size()*sizeof(U));
            npartOffset[i]+=(vec.size()/dim);
            ok=true;
          }
          catch (...) {
            if (1) {
              std::cerr << "WARNING !!! : error while reading Dataset["
                        << dataset << "]\n";
            }
            if (dataH5 != myH5) { // it's a split file
              delete dataH5;
            }
            throw -666; // wrong dataset
          }
        }
      }
    }
    if (dataH5 != myH5) { // it's a split file
      delete dataH5;
    }
  }
  return ok;
}
// ============================================================================
// loadDataset
template <class U>
bool SnapshotGadgetH5::loadDataset(std::string dataset, U  * data, const int comp_id, const int dim)
{
  bool ok = false;
  try {
    U dummy=(U) 1; // use dummy variable to help compiler to guess return type
    if (1)  std::cerr << dataset << "\n";
    GH5<float> * dataH5=NULL;
    unsigned int offset=0;
    for (int ifile=0; ifile<myH5->getHeader().NumFilesPerSnapshot; ifile++) {
      if (myH5->getHeader().NumFilesPerSnapshot>1) {
        // create file name
        std::size_t f1= filename.find(".hdf5"); // search ".hdf5"
        std::size_t f2= filename.find_last_of(".",f1-1); // search last "."
        std::ostringstream stm;
        stm << "." << ifile << ".hdf5";
        std::string myfile=filename.substr(0,f2);
        myfile = myfile + stm.str();
        std::cerr << "myfile = " << myfile << "\n";
        try {
          Exception::dontPrint();
          dataH5 = new GH5<float>(myfile,H5F_ACC_RDONLY,false);
        } catch (...) {
          std::cerr << "ERROR !! [" << myfile << "] not a Gadget3 (HDF5) file...\n";
        }
      }
      else {
        dataH5 = myH5;
      }

      // load dataset in a local vector
      std::vector<U> vec=dataH5->getDataset(dataset,dummy);
      ok=true;
      memcpy(data+offset*dim,&vec[0],vec.size()*sizeof(U));
      offset+= vec.size()/dim;

    }
    assert(offset==myH5->getHeader().NumPart_Total[comp_id]);
    if (dataH5 != myH5) { // it's a split file
      delete dataH5;
    }
  }

  catch (...) {
    std::cerr << "SnapshotGadgetH5::loadDataset ["<<dataset<<"] fails......\n";
    throw -1;
  }
  return ok;
}


// ============================================================================
// storeComponents
void SnapshotGadgetH5::storeComponents()
{
  ComponentRange cr;
  // all
  cr.setData(0,myH5->getNpartTotal()-1);
  cr.setType("all");
  crv.clear();
  crv.push_back(cr);
  // components
  const char * comp [] = { "gas", "halo", "disk", "bulge", "stars", "bndry"};
  for(int k=0,start=0; k<6; k++)  {
    if (myH5->getHeader().NumPart_Total[k]) {
      cr.setData(start,start+myH5->getHeader().NumPart_Total[k]-1,comp[k]);
      crv.push_back(cr);
      start+=myH5->getHeader().NumPart_Total[k];
    }
  }
}
// ============================================================================
// close()
int SnapshotGadgetH5::close()
{
  end_of_data=false;
  return 1;
}
// ============================================================================
// endendOfDataMessage()
QString SnapshotGadgetH5::endOfDataMessage()
{
  QString message=tr("Gadget3 Snapshot [")+QString(filename.c_str())+tr("] end of snapshot reached!");
  return message;
}
} // end of namespace
// ============================================================================

