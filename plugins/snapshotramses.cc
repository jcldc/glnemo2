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
#include <QtGui>  // Mandatory for plugins management
#include "snapshotramses.h"
#include <limits>
#include <iostream>
#include <sstream>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
Q_PLUGIN_METADATA(IID "fr.glnemo2.phigrapePlugin")
#endif

namespace glnemo {

// ============================================================================
// constructor
SnapshotRamses::SnapshotRamses():SnapshotInterface()
{
  valid=false;
  part_data = NULL;
  part_data = new ParticlesData();
  interface_type    ="Ramses";
  full_nbody = 0;
  namr=0;
  nstars=0;
  ndm=0;
  take_gas = take_halo = take_stars = false;
  h3d = NULL;
}

// ============================================================================
// destructor
SnapshotRamses::~SnapshotRamses()
{
  std::cerr << "SnapshotRamses::~SnapshotRamses()\n";
  if (obj)       delete obj;
  if (part_data) delete part_data;
  
  if (pos)   free ((float *) pos);
  if (vel)   free ((float *) vel);
  if (h3d)   delete h3d;
  if (valid) close();
}
// ============================================================================
// newObject                                                                   
// instantiate a new object and return a pointer on it                         
SnapshotInterface * SnapshotRamses::newObject(const std::string _filename, const int x)
{
  if (x) {;} // get rid of compiler warning
  filename = _filename;
  obj      = new SnapshotRamses();
  obj->setFileName(_filename);
  return obj;
}
// ============================================================================
// isValidData()                                                               
// return true if it's a Ramses snapshot.                                      
bool SnapshotRamses::isValidData()
{
  valid=false;
  part = new ramses::CPart(filename,2);
  amr  = new ramses::CAmr(filename);
  h3d  = new ramses::CHilbert3D(filename);

  if (part->isValid() || amr->isValid()) {
    connect(amr, SIGNAL(stringStatus(const QString)),this,SLOT(slotStringStatus(QString)));    //SLOT(slotStringStatus(const Qstring)));
    connect(part,SIGNAL(stringStatus(const QString)),this,SLOT(slotStringStatus(QString)));
    valid=true;
    int ndim=std::max(part->getNdim(),amr->getNdim());
    std::stringstream ss;
    ss << ndim << "D";
    interface_type = interface_type + ss.str();
  }

  return valid; 
}
// ============================================================================
// nextFrame()                                                                 
int SnapshotRamses::nextFrame(const int * index_tab, const int nsel)
{
  int status=0;
  stv.clear();
  parseSelectTime();
  load_vel = go->vel_req;

#if 0
  // set boundaries
  float x[8];

  setBoundaries(x); // we set boundaries again in case of interactive mode
  h3d->process(x[0],x[1],x[2],x[3],x[4],x[5]);
#endif

  if (go->select_part=="all" || (go->select_part.find("gas")!=std::string::npos))
    take_gas = true;
  else namr=0;
  if (go->select_part=="all" || (go->select_part.find("halo")!=std::string::npos))
    take_halo = true;
  else ndm=0;
  if (go->select_part=="all" || (go->select_part.find("stars")!=std::string::npos))
    take_stars = true;
  else nstars=0;

#if 0
  // we must re count in case of interactive xyz min/max selection, sux...
  if (take_gas) {
    namr=amr->loadData(*h3d); // count gas particles
  }
  if ((take_halo) || (take_stars)) {
    part->loadData(*h3d,take_halo,take_stars);     // count dm+stars particles
    part->getNbody(&ndm,&nstars);
  }
#endif
  if (valid && checkRangeTime(0)) {
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
    }
    *part_data->nbody = nsel;
    
    if (take_gas&&namr) {         // there are gas particles requested
      amr->loadData(*h3d,part_data->pos,part_data->vel,part_data->rho->data, part_data->rneib->data,part_data->temp->data,
                    index_tab,nsel,load_vel);
      std::cerr << "after amr->loadata\n";
    }
    
    if ((take_halo&&ndm) || (take_stars&&nstars)) { // there are halo|stars particles requested
      part->loadData(*h3d,take_halo,take_stars,part_data->pos,part_data->vel,index_tab,nsel,load_vel,namr);
    }
    
    //part_data->computeMaxSize();
    // rescale particles
    for (int i=0;i <nsel; i++) {
      part_data->pos[i*3+0]     *= go->scale;//*amr->getHeader()->boxlen;
      part_data->pos[i*3+1]     *= go->scale;//*amr->getHeader()->boxlen;
      part_data->pos[i*3+2]     *= go->scale;//*amr->getHeader()->boxlen;
      if (part_data->rneib->data[i]!=-1)
        part_data->rneib->data[i] *= go->scale;//*amr->getHeader()->boxlen;
       //std::cerr << part_data->rho->data[i] <<"\n";
      //part_data->rho->data[i]=1.0;

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
    //*part_data->timu = amr->getHeader()->time; // !!!!!! gadget_io->getTime();
    *part_data->timu = amr->getMapInfo("time")*amr->getMapInfo("unit_t") /(60.*60.*24.*365.15*1E9) ;
    end_of_data=true; // only one frame from an gadget snapshot

  }
  return status;
}
// ============================================================================
// getSnapshotRange
ComponentRangeVector * SnapshotRamses::getSnapshotRange()
{
  crv.clear();
  if (valid) {
    ComponentRange cr;
    cr.setData(0,namr+ndm+nstars-1);
    cr.setType("all");
    crv.push_back(cr);
    // store components
    int start=0;
    if (namr) {
      cr.setData(start,start+namr-1,"gas");
      crv.push_back(cr);
      start+=namr;
    }
    if (ndm) {
      cr.setData(start,start+ndm-1,"halo");
      crv.push_back(cr);
      start+=ndm;
    }
    if (nstars) {
      cr.setData(start,start+nstars-1,"stars");
      crv.push_back(cr);
      start+=nstars;
    }
    //ComponentRange::list(&crv);
    if (first) {
      first       = false;
      crv_first   = crv;
      nbody_first = namr+ndm+nstars;
      //time_first  = 0.0;
    }
  }
  return &crv;
}
// ============================================================================
// setBoundaries()
void SnapshotRamses::setBoundaries(float x[])
{
  // boundary box
  if (go->xmin==0.0 && go->xmax== 0.0) {
    x[0]=std::numeric_limits<float>::min();
    x[1]=std::numeric_limits<float>::max();
  } else {
    x[0] = go->xmin;
    x[1] = go->xmax;
  }
  if (go->ymin==0.0 && go->ymax== 0.0) {
    x[2]=std::numeric_limits<float>::min();
    x[3]=std::numeric_limits<float>::max();
  } else {
    x[2] = go->ymin;
    x[3] = go->ymax;
  }
  if (go->zmin==0.0 && go->zmax== 0.0) {
    x[4]=std::numeric_limits<float>::min();
    x[5]=std::numeric_limits<float>::max();
  } else {
    x[4] = go->zmin;
    x[5] = go->zmax;
  }
  x[6] = go->lmin;
  x[7] = go->lmax;
  amr->setBoundary(x);
  part->setBoundary(x);

}

// ============================================================================
// initLoading()                                                               
int SnapshotRamses::initLoading(GlobalOptions * so)
{
  go = so;
  load_vel = so->vel_req;
  select_time = so->select_time;  
  std::cerr << "SnapshotRamses::initLoading IN\n";

  // set boundaries
  float x[8];
  setBoundaries(x);

  //ramses::CHilbert3D h3d(filename);
  h3d->process(x[0],x[1],x[2],x[3],x[4],x[5]);
  //std::vector<int> cpu_list=h3d.getCpuList();
  h3d->printCpuList();
  //std::exit(1);
  //

  if (so->select_part=="" || so->select_part=="all" || (so->select_part.find("gas")!=std::string::npos))
    take_gas = true;
  if (so->select_part=="" || so->select_part=="all" || (so->select_part.find("halo")!=std::string::npos))
    take_halo = true;
  if (so->select_part=="" || so->select_part=="all" || (so->select_part.find("stars")!=std::string::npos))
    take_stars = true;
  
  if (take_gas && amr->isValid()) {
    take_gas = true;
    namr=amr->loadData(*h3d); // count gas particles
  } else {
    namr=0;
  }
  
  if ((take_halo || take_stars) && part->isValid()) {
    part->loadData(*h3d,take_halo,take_stars);     // count dm+stars particles
    part->getNbody(&ndm,&nstars);
  } else {
    ndm=0; nstars=0;
  }
  // reset take_xxx variables if no particles selected from the command line
  if (so->select_part=="") {
    take_gas = take_halo = take_stars = false;
  }
  std::cerr << "SnapshotRamses::initLoading OUT\n";
  return 1;
}
// ============================================================================
// endendOfDataMessage()                                                       
QString SnapshotRamses::endOfDataMessage()
{
  QString message=tr("Ramses Snapshot [")+QString(filename.c_str())+tr("] end of snapshot reached!");
  return message;
}
}
// You have to export outside of the namespace "glnemo"
// BUT you have to specify the namespace in the export:
// ==> glnemo::SnapshotRamses                         

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ramsesplugin, glnemo::SnapshotRamses);
#endif
