// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2014                                       
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           Pole de l'Etoile, site de Chateau-Gombert                         
//           38, rue Frederic Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "camr.h"
#include "cfortio.h"

namespace ramses {
const double CAmr::XH = 0.76;
const double CAmr::mH = 1.6600000e-24;
const double CAmr::kB = 1.3806200e-16;

// ============================================================================
//
CAmr::CAmr(const std::string _indir, const bool _v)
{
  nbody=0;
  //pos = mass = vel = NULL;
  verbose=_v;
  indir = _indir;
  infile="";

  // keep filename untill last /
  int found=indir.find_last_of("/");
  if (found != (int) std::string::npos && (int) indir.rfind("output_")<found) {
    indir.erase(found,indir.length()-found);
  }
  std::cerr << "indir =[" << indir <<"]\n";
  
  found=(int) indir.rfind("output_"); 
  if (found!=std::string::npos) {
    s_run_index= indir.substr(found+7,indir.length()-1); // output_ = 7 characters
    
    while ((found=s_run_index.find_last_of("/"))>0) { // remove trailing "/"
      s_run_index.erase(found,found);
    }
    infile = indir + "/amr_" + s_run_index + ".out00001";
    testhydrofile = indir + "/hydro_" + s_run_index + ".out00001";
    info_file = indir + "/info_" + s_run_index + ".txt";
    std::cerr << "Run index = " << s_run_index <<  "  infile=[" << infile << "]\n";
  }
  // readHeader
  if (amr.open(infile)) {
    readHeader();
    amr.close();
  }
  // read info file
  readInfoFile();

  //computeNbody();
  //loadData();
}
// ============================================================================
//
CAmr::~CAmr()
{
}
// ----------------------------------------------------------------------------
//
bool CAmr::isValid()
{    
  if (amr.open(infile) && hydro.open(testhydrofile)) {
    valid=true;
    //readHeader();
    amr.close();
    hydro.close();
    std::cerr << "ncpu="<<ncpu<<"  ndim="<<ndim<< "\n";// "npart=" << npart << "\n";
    xbound[0] = nx/2;
    xbound[1] = ny/2;
    xbound[2] = nz/2;
    twotondim = pow(2,ndim);
    ordering = "hilbert";
    scale_nH =  XH/mH * 0.276090728884102e-29;
  }
  else
    valid=false;
  amr.close();
  return valid;
}
// ============================================================================
// readInfoFile
bool CAmr::readInfoFile()
{
  bool status=true;
  std::ifstream fi;

  fi.open(info_file.c_str(),std::ios::in);

  if (! fi.is_open()) {
    std::cerr << "Unable to open file ["<<info_file.c_str()<<"] for reading, aborting...\n";
    status = false;
  }
  else {
    bool stop=false;
    while (!stop && ! fi.eof()) {           // while ! eof
      std::string line;
      getline(fi,line); // Read one line
      if ( ! fi.eof()) { // ! EOF, keep reading
        int pos; // pos of "="
        if ((pos=line.find("=")) != std::string::npos) { // '=' found
          line.replace(pos,1," ",1); // replace '=' with ' '
          //std::cerr << "Line = " << line << "\n";
          std::istringstream str(line);  // stream line
          std::string key;
          double value;
          str >> key;
          str >> value;
          mapinfo[key] = value; // build info map
        }

        else { // end of file
          if (fi.eof()) {
            stop   = true;
          }
        }
      }
    }
    fi.close();
  }
  return status;
}
// ============================================================================
// readHeader
int CAmr::readHeader()
{
  int len1,len2;
  amr.readDataBlock((char *) &ncpu);

  amr.readDataBlock((char *) &ndim);

  len1=amr.readFRecord();
  amr.readData((char *) &nx,sizeof(int),1);
  amr.readData((char *) &ny,sizeof(int),1);
  amr.readData((char *) &nz,sizeof(int),1);
  len2=amr.readFRecord();
  assert(amr.good() && len1==len2);

  amr.readDataBlock((char *) &nlevelmax);
  std::cerr << "AMR Nlevel max="<<nlevelmax<<"\n";
  amr.readDataBlock((char *) &ngridmax);

  amr.readDataBlock((char *) &nboundary);
  
  amr.readDataBlock((char *) &ngrid_current);
  
  amr.readDataBlock((char *) &header.boxlen);
  std::cerr << "BOX LEN ="<<header.boxlen<<"\n";
  amr.skipBlock(3); // noutput,iout,ifout
                    // tout
                    // aout
  amr.readDataBlock((char *) &header.time);
  std::cerr << "TIME  ="<<header.time<<"\n";
  amr.skipBlock(4); // dtold
                    // dtnew
                    // nstep,nstep_coarse
                    // const,mass_tot_0,rho_tot
  len1=amr.readFRecord();
  amr.readData((char *) &header.omega_m   ,sizeof(double),1);
  amr.readData((char *) &header.omega_l   ,sizeof(double),1);
  amr.readData((char *) &header.omega_k   ,sizeof(double),1);
  amr.readData((char *) &header.omega_b   ,sizeof(double),1);
  amr.readData((char *) &header.h0        ,sizeof(double),1);
  amr.readData((char *) &header.aexp_ini  ,sizeof(double),1);
  amr.readData((char *) &header.boxlen_ini,sizeof(double),1);
  len2=amr.readFRecord();
  assert(amr.good() && len1==len2);

  len1=amr.readFRecord();
  amr.readData((char *) &header.aexp,sizeof(double),1);
  amr.readData((char *) &header.hexp,sizeof(double),1);
  amr.readData((char *) &header.aexp_old,sizeof(double),1);
  amr.readData((char *) &header.epot_tot_int,sizeof(double),1);
  amr.readData((char *) &header.epot_tot_old,sizeof(double),1);
  len2=amr.readFRecord();
  assert(amr.good() && len1==len2);

  if (len2==len1) ; // remove warning....
  
  return 1;
}
// ============================================================================
// loadData
int CAmr::loadData(float * pos, float * vel, float * rho, float * rneib, float * temp,const int *index,
                   const int nsel,   const bool load_vel)
{
  int ngridfile  [nlevelmax][ncpu+nboundary];
  int ngridlevel [nlevelmax][ncpu          ];
  int ngridbound [nlevelmax][     nboundary];
  double xc[3][8];
  int ngrida;
  std::string infile;
  QString str_status;
  if (load_vel || nsel || vel) {;} // remove compiler warning
  qsrand(0);
  nbody = 0;
  int cpt=0;
  bool count_only=false;

  if (index==NULL)  count_only=true;
  // loop on all cpus/files
  for (int icpu=0; icpu<ncpu; icpu++) {
    std::ostringstream osf;
    osf << std::fixed << std::setw(5) << std::setfill('0') <<icpu+1;
    infile = indir + "/amr_" + s_run_index + ".out" + osf.str();
    if (verbose) std::cerr << "CAmr::loadData infile-> ["<<infile << "]\n";
    str_status = std::string("Loading file : " + infile).c_str();
    emit stringStatus(str_status);
    amr.open(infile);
    amr.skipBlock(21);
    
    amr.readDataBlock((char *) &ngridlevel);
    memcpy(ngridfile,ngridlevel,sizeof(int)*nlevelmax*ncpu);
    
    amr.skipBlock();    
    
    if (nboundary>0) {
      amr.skipBlock(2);
      
      amr.readDataBlock((char *) &ngridbound);
      // must convert the following line
      //ngridfile(ncpu+1:ncpu+nboundary,1:nlevelmax)=ngridbound
      for (int i=0;i<nlevelmax;i++) {
        // copy grid level
        memcpy(&ngridfile [i][0],
               &ngridlevel[i][0],sizeof(int)*ncpu);
        // copy gridbound
        memcpy(&ngridfile [i][ncpu],
               &ngridbound[i][0],sizeof(int)*nboundary);
      }
    }
    amr.skipBlock();    
    // ROM: comment the single following line for old stuff
    amr.skipBlock();    
    if (ordering=="bisection") 
      amr.skipBlock(5);
    else 
      amr.skipBlock();
    amr.skipBlock(3);
    // Open HYDRO file and skip header
    std::string hydrofile = indir + "/hydro_" + s_run_index + ".out" + osf.str();
    //if (verbose) std::cerr << "CAmr::loadData hydrofile-> ["<<hydrofile << "]\n";
    hydro.open(hydrofile,count_only);
    hydro.skipBlock();
    hydro.readDataBlock((char *) &nvarh);
    //std::cerr << "nvarh = " << nvarh << "\n" ;
    hydro.skipBlock(4);
    // loop over levels
    for (int ilevel=0; ilevel<lmax; ilevel++) {
      
      // Geometry
      double dx=pow(0.5,ilevel+1);
      double dx2=0.5*dx;
      for (int ind=0; ind<twotondim; ind++) {
        int iz=ind/4;
        int iy=(ind-4*iz)/2;
        int ix=(ind-2*iy-4*iz);
        xc[0][ind]=(ix-0.5)*dx;
        xc[1][ind]=(iy-0.5)*dx;
        xc[2][ind]=(iz-0.5)*dx;
      }
      // allocate work arrays
      ngrida=ngridfile[ilevel][icpu];
      
      double * xg=NULL, *var=NULL;
      int * son=NULL;
      if (ngrida>0) {
        xg = new double[ngrida*ndim];
        son= new int   [ngrida*twotondim];
        if (!count_only) 
          var= new double[ngrida*twotondim*nvarh];
      }
      // Loop over domains
      for (int j=0; j<(nboundary+ncpu); j++) {
        if (ngridfile[ilevel][j]>0) {
          amr.skipBlock(); // Skip grid index
          amr.skipBlock(); // Skip next index
          amr.skipBlock(); // Skip prev index
          //
          // Read grid center
          //
          for (int idim=0;idim<ndim;idim++) {
            if (j==icpu) {
              amr.readDataBlock((char *) &xg[idim*ngrida]);
            } 
            else amr.skipBlock();
          }
          amr.skipBlock();       // Skip father index
          amr.skipBlock(2*ndim); // Skip nbor index
          //
          // Read son index
          //
          for (int ind=0;ind<twotondim;ind++) {
            if (j==icpu) {
              amr.readDataBlock((char *) &son[ind*ngrida]);
            }
            else amr.skipBlock();
          }          
          amr.skipBlock(twotondim); // Skip cpu map          
          amr.skipBlock(twotondim); // Skip refinement map
        }
        //
        // Read HYDRO data
        //
        hydro.skipBlock(2);
        if (!count_only && ngridfile[ilevel][j]>0) {
          // Read hydro variables
          for (int ind=0;ind<twotondim;ind++) {
            for (int ivar=0; ivar<nvarh; ivar++) {
              if (j==icpu) {
		hydro.readDataBlock((char *) &var[ivar*ngrida*twotondim+ind*ngrida]);
              }
              else hydro.skipBlock();
            }
          }
        }
      }      
      if (ngrida>0) {
        //  Loop over cells
        for (int ind=0;ind<twotondim;ind++) {
          // Store data cube
          for (int i=0;i<ngrida;i++) {
	    // compute cell center
            double px=xg[0*ngrida+i]+xc[0][ind]-xbound[0]; // x
            double py=xg[1*ngrida+i]+xc[1][ind]-xbound[1]; // y
            double pz=xg[2*ngrida+i]+xc[2][ind]-xbound[2]; // z
            bool ok_cell =       (
                !(son[ind*ngrida+i]>0 && ilevel<lmax) && // cells is NOT refined
                (ilevel>=lmin)                        &&
                ((px+dx2)>=xmin)                      &&
                ((py+dx2)>=ymin)                      &&
                ((pz+dx2)>=zmin)                      &&
                ((px-dx2)<=xmax)                      &&
                ((py-dx2)<=ymax)                      &&
                ((pz-dx2)<=zmax) );
            if (ok_cell) {
              if (!count_only) {
                int idx=index[nbody];
                if (idx!=-1) { // it's a valide particle
#if 0
                  pos[3*cpt+0] = px + getSign()*dx2*getRandom(0.5);
                  pos[3*cpt+1] = py + getSign()*dx2*getRandom(0.5);
                  pos[3*cpt+2] = pz + getSign()*dx2*getRandom(0.5);
#else

#if 0
                  pos[3*cpt+0] = px + getSign()*dx2*getRandomGaussian(dx2);
                  pos[3*cpt+1] = py + getSign()*dx2*getRandomGaussian(dx2);
                  pos[3*cpt+2] = pz + getSign()*dx2*getRandomGaussian(dx2);
#else
                  pos[3*cpt+0] = px ;
                  pos[3*cpt+1] = py ;
                  pos[3*cpt+2] = pz ;
#endif

#endif
                  rneib[cpt]   = dx;
                  rho[cpt] = var[0*ngrida*twotondim+ind*ngrida+i];
#if 1
                  temp[cpt]= std::max(0.0,var[4*ngrida*twotondim+ind*ngrida+i]/rho[cpt]);
                  //temp[cpt]= var[4*ngrida*twotondim+ind*ngrida+i]/rho[cpt];
#else
                  float p=var[4*ngrida*twotondim+ind*ngrida+i]; // pressure
                  float alpha=0.28;                 // fraction of He in mass
                  float mu=4./(8.-5.*alpha);
                  float mh=1.67e-24;                // proton mass in g
                  float kb=1.38e-16;                // Boltzmann constant in erg.K-1
                  temp[cpt]= mu*mh/kb*p/rho[cpt]*1.e14;    // Temperature en K
                  //temp[cpt] /= (11604.5/1000.);            // Temperature en Kev
                  temp[cpt]=std::max((float)0.0,temp[cpt]);
                  
#endif
                  cpt++;
                }
#if 0
                pos.push_back(px);  // x
                pos.push_back(py);  // y
                pos.push_back(pz);  // z
                hsml.push_back(dx); // hsml
                float rho = var[0*ngrida*twotondim+ind*ngrida+i];
                float temp= var[4*ngrida*twotondim+ind*ngrida+i]/rho;
                phys.push_back(rho); // rho var(i,ind,1) * scale_nH
#endif
              }
              nbody++;
            }
          }
        }
        // garbage collecting
        delete [] xg;
        //delete [] x;
        delete [] son;
        if (!count_only) 
          delete [] var;
      }
    } // ilevel
    amr.close();    
    hydro.close();    
  } //for (int icpu=0 .... 
  return nbody;
}
} // end of namespace ramses
