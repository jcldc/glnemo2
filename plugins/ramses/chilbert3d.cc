// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2018
// e-mail:   Jean-Charles.Lambert@lam.fr
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)
//           Laboratoire d'Astrophysique de Marseille
//           Pole de l'Etoile, site de Chateau-Gombert
//           38, rue Frederic Joliot-Curie
//           13388 Marseille cedex 13 France
//           CNRS U.M.R 7326
// ============================================================================

/*
    @author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

#include "chilbert3d.h"

namespace ramses {
  using namespace std;

const int CHilbert3D::state_diagram[12][2][8] = {
  1, 2, 3, 2, 4, 5, 3, 5,
  0, 1, 3, 2, 7, 6, 4, 5,
  2, 6, 0, 7, 8, 8, 0, 7,
  0, 7, 1, 6, 3, 4, 2, 5,
  0, 9,10, 9, 1, 1,11,11,
  0, 3, 7, 4, 1, 2, 6, 5,
  6, 0, 6,11, 9, 0, 9, 8,
  2, 3, 1, 0, 5, 4, 6, 7,
  11,11, 0, 7, 5, 9, 0, 7,
  4, 3, 5, 2, 7, 0, 6, 1,
  4, 4, 8, 8, 0, 6,10, 6,
  6, 5, 1, 2, 7, 4, 0, 3,
  5, 7, 5, 3, 1, 1,11,11,
  4, 7, 3, 0, 5, 6, 2, 1,
  6, 1, 6,10, 9, 4, 9,10,
  6, 7, 5, 4, 1, 0, 2, 3,
  10, 3, 1, 1,10, 3, 5, 9,
  2, 5, 3, 4, 1, 6, 0, 7,
  4, 4, 8, 8, 2, 7, 2, 3,
  2, 1, 5, 6, 3, 0, 4, 7,
  7, 2,11, 2, 7, 5, 8, 5,
  4, 5, 7, 6, 3, 2, 0, 1,
  10, 3, 2, 6,10, 3, 4, 4,
  6, 1, 7, 0, 5, 2, 4, 3
};

// ----------------------------------------------------------------------------
// constructor
CHilbert3D::CHilbert3D(const std::string _indir)
{
  cpu_read=NULL;
  //cpu_list=NULL;
  bound_key=NULL;
  indir = _indir;

  // keep filename untill last /
  int found=indir.find_last_of("/");
  if ( (found != std::string::npos) &&  (indir.rfind("output_")<found)) {
    indir.erase(found,indir.length()-found);
  }
  std::cerr << "indir =[" << indir <<"]\n";

  found=(int) indir.rfind("output_");
  if (found!=std::string::npos) {
    s_run_index= indir.substr(found+7,indir.length()-1); // output_ = 7 characters

    while ((found=s_run_index.find_last_of("/"))>0) { // remove trailing "/"
      s_run_index.erase(found,found);
    }

    info_file = indir + "/info_" + s_run_index + ".txt";
  }

  // read info file
  readInfoFile();

  if (false)
    for (int k=0; k<12; k++) {
      for (int j=0; j<2; j++) {
        for (int i=0; i<8; i++) {
          std::cerr << state_diagram[k][j][i] << " ";
        }
        std::cerr << "\n";
      }
    }
}
// ----------------------------------------------------------------------------
// Destructor
CHilbert3D::~CHilbert3D()
{
  if (cpu_read) {
    delete [] cpu_read;
  }
  //if (cpu_list) {
  //  delete [] cpu_list;
  //}
  if (bound_key) {
    delete [] bound_key;
  }
}
// ============================================================================
// readInfoFile
bool CHilbert3D::readInfoFile()
{
  bool status=true;
  std::ifstream fi;

  fi.open(info_file.c_str(),std::ios::in);

  if (! fi.is_open()) {
    std::cerr << "CHilbert3D::readInfoFile(): Unable to open file ["<<info_file.c_str()<<"] for reading, aborting...\n";
    status = false;
  }
  else {
    bool stop=false;
    int cpt=1;
    while (!stop && ! fi.eof() && cpt<21) {           // while ! eof
      std::string line;
      getline(fi,line); // Read one line
      if ( ! fi.eof()) { // ! EOF, keep reading
        int pos; // pos of "="
        if ((pos=line.find("=")) != std::string::npos) { // '=' found
          line.replace(pos,1," ",1); // replace '=' with ' '
          //std::cerr << "Line = " << line << "\n";
          std::istringstream str(line);  // stream line
          std::string key;
          if (cpt!=20) {
            double value;
            str >> key;
            str >> value;
            mapinfo[key] = value; // build info map
          } else { // ordering
            str >> key;
            std::string dummy;
            str >> dummy;
            str >> ordering;
            //std::cerr << "Ordering=["<<ordering<<"]\n";
          }
        }
        else { // end of file
          if (fi.eof()) {
            stop   = true;
          }
        }
      }
      cpt++;
    } // end of while
    if (ordering=="hilbert") {
      std::string line;
      getline(fi,line); // Read one line
      int cpt=0;
      ncpu = int(mapinfo["ncpu"]);
      bound_key = new double[ncpu+1];
      cpu_read  = new bool[ncpu];
      //cpu_list  = new int[ncpu];

      for (int i=1; i<=ncpu; i++) {
        getline(fi,line); // Read one line
        std::istringstream str(line);  // stream line

        str >> cpt; // #cpu
        str >> bound_key[i-1];
        str >> bound_key[i  ];
        //std::cerr << "Bound key="<<bound_key[i-1]<<"\n";
      }
    }


    fi.close();
  }
  return status;
}
// ============================================================================
// process
bool CHilbert3D::process(double xmin, double xmax,
                         double ymin, double ymax,
                         double zmin, double zmax)
{
  bool status=false;
  if (ordering=="hilbert") {
    int lmax=100;
    status=true;

    int levelmax= int(mapinfo["levelmax"]);
    int ncpu = int(mapinfo["ncpu"]);
    int ndim = int(mapinfo["ndim"]);
    lmax=std::max(std::min(lmax, levelmax),1);
    double dmax=std::max(std::max(xmax-xmin,ymax-ymin),zmax-zmin);
    int lllmin=0;
    for (int ilevel=1; ilevel<=lmax;ilevel++) {
      double  deltax=pow(0.5,ilevel);
      lllmin=ilevel;
      if(deltax<dmax) break;
    }

    int bit_length=lllmin-1;
    int maxdom=pow(2,bit_length);

    int imin=0, imax=0, jmin=0, jmax=0, kmin=0, kmax=0;

    if  (bit_length>0) {
      imin=int(xmin*double(maxdom));
      imax=imin+1;
      jmin=int(ymin*double(maxdom));
      jmax=jmin+1;
      kmin=int(zmin*double(maxdom));
      kmax=kmin+1;
    }
    std::cerr << "maxdom="<<maxdom<<" "<<bit_length<<"\n";
    std::cerr << "lmax="<<lmax<<" "<<dmax<<" "<<lllmin<<"\n";
    double dkey=pow((double(pow(2.,(lmax+1))/double(maxdom))),ndim);
    std::cerr << "dkey="<<dkey <<"\n";
    int ndom=1;
    if (bit_length>0) ndom=8;
    int idom[8],jdom[8],kdom[8];

    idom[0]=imin; idom[1]=imax;
    idom[2]=imin; idom[3]=imax;
    idom[4]=imin; idom[5]=imax;
    idom[6]=imin; idom[7]=imax;
    jdom[0]=jmin; jdom[1]=jmin;
    jdom[2]=jmax; jdom[3]=jmax;
    jdom[4]=jmin; jdom[5]=jmin;
    jdom[6]=jmax; jdom[7]=jmax;
    kdom[0]=kmin; kdom[1]=kmin;
    kdom[2]=kmin; kdom[3]=kmin;
    kdom[4]=kmax; kdom[5]=kmax;
    kdom[6]=kmax; kdom[7]=kmax;

    double bounding_min[8], bounding_max[8];
    for (int i=0; i<ndom; i++) {
      double order_min=0.0;
      if(bit_length>0) {
        int ii=idom[i];
        int jj=jdom[i];
        int kk=kdom[i];
        hilbert3D(ii,jj,kk,order_min,bit_length,1);
        std::cerr << ">" << ii << " "<< jj << " "<< kk << " "<<order_min << "\n";
      }
      bounding_min[i]=(order_min)*dkey;
      bounding_max[i]=(order_min+1.0)*dkey;
      //std::cerr << "bounding="<<bounding_min[i]<<" "<<bounding_max[i]<<"\n";
    }
    int cpu_min[8],cpu_max[8];
    for (int i=0; i<8; i++){
      cpu_min[i]=0;
      cpu_max[i]=0;
    }
    for (int impi=1; impi<=ncpu; impi++) {
      for (int i=0; i<ndom; i++) {
        //std::cerr <<"bound_key="<<bound_key[impi-1]<<" " <<bound_key[impi] << " " << bounding_min[i] << "\n";
        if (bound_key[impi-1] <= bounding_min[i] &&
            bound_key[impi]>bounding_min[i]) {
          cpu_min[i]=impi;
          //std::cerr <<"cpu_min["<<i<<"]="<<cpu_min[i]<<"\n";
        }
        if (bound_key[impi-1]<bounding_max[i] &&
            bound_key[impi]>=bounding_max[i]){
          cpu_max[i]=impi;
          //std::cerr <<"cpu_max["<<i<<"]="<<cpu_max[i]<<"\n";
        }
      }
    }

    // set cpu_read to false
    for (int i=1; i<=ncpu; i++) {
      cpu_read[i]=false;
    }
    for (int i=0; i<ndom; i++) {
      //std::cerr<< "cpu_min["<<i<<"]="<<cpu_min[i]<<" "<<cpu_max[i]<<"\n";
    }

    cpu_list.clear();
    for (int i=0; i<ndom; i++) {
      //std::cerr << "i="<<i<<"\n";
      for (int j=cpu_min[i]; j<=cpu_max[i]; j++) {
        //std::cerr << "min/max "<<i<< " " << cpu_min[i]<<" "<< cpu_max[i]<<"\n";
        if(! cpu_read[j]) {
          cpu_list.push_back(j);
          cpu_read[j]=true;
          //std::cerr << "size="<<cpu_list.size()<<" j="<<j<<"\n";
        }
      }
    }
  }
  else { // It's not hilbert ordering
    for (int j=0; j<ncpu; j++) {
      cpu_list.push_back(j);
    }
  }
  return status;
}
// ============================================================================
// hilbert3D
void CHilbert3D::hilbert3D(const int x, const int y, const int z,
                           double &order, const int bit_length, const int npoint)
{
  assert(npoint==1);
  bool i_bit_mask[3*bit_length],
      x_bit_mask[bit_length],y_bit_mask[bit_length],z_bit_mask[bit_length];

  // convert to binary
  for (int i=0; i<bit_length; i++) {
    x_bit_mask[i]=(x>>i)&1;
    y_bit_mask[i]=(y>>i)&1;
    z_bit_mask[i]=(z>>i)&1;
  }

  // interleave bits
  for (int i=0; i<bit_length; i++) {
    i_bit_mask[3*i+2]=x_bit_mask[i];
    i_bit_mask[3*i+1]=y_bit_mask[i];
    i_bit_mask[3*i  ]=z_bit_mask[i];
  }

  // build Hilbert ordering using state diagram
  int cstate=0;
  for (int i=bit_length-1; i>=0; i--) {
    int b2=0 ; if(i_bit_mask[3*i+2])b2=1;
    int b1=0 ; if(i_bit_mask[3*i+1])b1=1;
    int b0=0 ; if(i_bit_mask[3*i  ])b0=1;
    int sdigit=b2*4+b1*2+b0;
    assert(cstate<12);
    assert(sdigit<8);
    int nstate=state_diagram[cstate][0][sdigit];
    int hdigit=state_diagram[cstate][1][sdigit];
    //std::cerr << ">>>:"<< cstate << " " << sdigit << " " << nstate << " " << hdigit << "\n";

    i_bit_mask[3*i+2]=(hdigit>>2)&1;
    i_bit_mask[3*i+1]=(hdigit>>1)&1;
    i_bit_mask[3*i  ]=(hdigit>>0)&1;
    //std::cerr <<"i_bit_mask="<< i_bit_mask[3*i+2] << " " << i_bit_mask[3*i+1] << " " << i_bit_mask[3*i] <<"\n";
    cstate=nstate;
  }

  // save Hilbert key as double precision real
  order=0.;
  for (int i=0; i<(3*bit_length); i++) {
     int b0=0 ; if(i_bit_mask[i])b0=1;
     //std::cerr << "b0="<<b0<<"\n";
     order=order+double(b0)*double(pow(2,i));
  }
}
// ============================================================================
// hilbert3D
void CHilbert3D::printCpuList()
{
  std::cerr << "#cpu read=" << cpu_list.size()<<"\n";
  for (unsigned int i=0; i<cpu_list.size();i++) {
    std::cerr << cpu_list[i] << " ";
  }
  std::cerr << "\n";
}
} // namespace ramses
