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

#ifndef GH5_H
#define GH5_H
#include <vector>
#include <map>
#include <H5Cpp.h>
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif


namespace glnemo {
// HDF5 gadget header
typedef struct h5_header {
  std::vector<double> MassTable; //[6]; "
  double   Time;
  double   Redshift;
  int      Flag_DoublePrecision;
  int      Flag_IC_Info;
  int      Flag_Metals;
  int      Flag_Cooling;
  int      Flag_Sfr;
  int      Flag_StellarAge;
  int      flag_feedback;
  std::vector<unsigned int> NumPart_Total;//[6];
  std::vector<int> NumPart_Total_HighWord;//[6];
  std::vector<int> NumPart_ThisFile;//[6];
  int      NumFilesPerSnapshot;
  double   BoxSize;
  double   Omega0;
  double   OmegaLambda;
  double   HubbleParam;
} t_h5_header;

//
// class GH5
//
template <class T> class GH5 {
public:
  GH5(const std::string _f_name,unsigned int mode, const bool verb=false);
 ~GH5();
  t_h5_header getHeader() { return header; }

  // reading
  template <class U>
  std::vector<U> getDataset(std::string dset_name, U );
  template <class U>
  std::vector<U> getAttribute(std::string attr_name);
  int getNpartTotal() { return npart_total; }
  unsigned int getGadgetNumber() { return gadget_number; }
private:
  std::map<std::string, bool> histo_group;

  bool verbose;
  void readHeaderAttributes();
  int npart_total;
  template <class U>
  DataType guessType(U);
  std::string f_name;
  H5File * myfile;
  Group header_group;
  t_h5_header header;
  unsigned int gadget_number;

};
} // namespace uns

#endif // GH5_H
