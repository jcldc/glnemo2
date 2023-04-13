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


#include "gadgeth5.h"
#include <iostream>
#include <assert.h>
#include <exception>
namespace glnemo {

// ============================================================================
// Constructor
template <class T>
GH5<T>::GH5(const std::string _f_name,unsigned int mode, const bool verb)
{
  verbose = verb;
  f_name=_f_name;
  myfile = NULL;
  myfile = new H5File(f_name,mode);
  gadget_number = 3;
  if (mode==H5F_ACC_RDONLY) { // reading mode only
    readHeaderAttributes();
  } else
    if (mode==H5F_ACC_TRUNC) { // read and write mode
      // we create header group
      header_group=myfile->createGroup("/Header");
    }
}
// ============================================================================
// Destructor
template <class T>
GH5<T>::~GH5()
{
  if (myfile) {
    delete myfile;
  }
}


// ============================================================================
// readHeaderAttributes
template <class T>
void GH5<T>::readHeaderAttributes()
{
  header.MassTable = getAttribute<double>("MassTable");
  assert(header.MassTable.size()==6);

  try {
    header.Time      = (double ) getAttribute<double>("Time")[0];
    header.Redshift  = (double ) getAttribute<double>("Redshift")[0];
    header.BoxSize   = (double ) getAttribute<double>("BoxSize")[0];
    header.Omega0    = (double ) getAttribute<double>("Omega0")[0];
    header.OmegaLambda = (double ) getAttribute<double>("OmegaLambda")[0];
    header.HubbleParam = (double ) getAttribute<double>("HubbleParam")[0];

    header.Flag_Cooling = (int) getAttribute<int>("Flag_Cooling")[0];
    header.Flag_DoublePrecision = (int) getAttribute<int>("Flag_DoublePrecision")[0];
    header.Flag_IC_Info = (int) getAttribute<int>("Flag_IC_Info")[0];
    header.Flag_Metals = (int) getAttribute<int>("Flag_Metals")[0];
    header.Flag_Sfr = (int) getAttribute<int>("Flag_Sfr")[0];
    header.Flag_StellarAge = (int) getAttribute<int>("Flag_StellarAge")[0];
  } catch (...) {
    std::cerr << "WARNING:readHeaderAttributes error loading some attributes\n"  ;
  }

  header.NumFilesPerSnapshot = (int) getAttribute<int>("NumFilesPerSnapshot")[0];
  header.NumPart_ThisFile = getAttribute<int>("NumPart_ThisFile");
  header.NumPart_Total = getAttribute<unsigned int>("NumPart_Total");
  try {
    header.NumPart_Total_HighWord = getAttribute<int>("NumPart_Total_HighWord");
    gadget_number=3;
  } catch (...) {
    std::cerr << "WARNING: looks like to be a Gadget4 file";
    gadget_number=4;
  }
  // compute npart_total
  npart_total=0;
  for(int k=0; k<6; k++)  {
    npart_total+=header.NumPart_Total[k];
  }

}
// ============================================================================
// getAttribute
// Read an attribute from desired group Header
// return a vector of data
template <class T>
template <class U> std::vector<U> GH5<T>::getAttribute(std::string attr_name)
{
  if (verbose) {
    std::cerr << "= = = = = = = = = = = = = = = = = =\n";
    std::cerr << "Read Attribute ["<<attr_name<< "]\n";
  }
  try {
    Group     grp  = myfile->openGroup("Header"  );
    Attribute attr = grp.openAttribute(attr_name);
    DataType  atype = attr.getDataType();
    DataSpace aspace= attr.getSpace();

    if (verbose) {
      std::cerr << "size          = "<< atype.getSize() << "\n";
      std::cerr << "storage space =" << attr.getStorageSize() << "\n";
      std::cerr << "mem data size =" << attr.getInMemDataSize() << "\n";
    }

    int arank = aspace.getSimpleExtentNdims();
    hsize_t adims_out[arank];
    aspace.getSimpleExtentDims( adims_out, NULL);
    if (verbose) {
      std::cerr << "rank " << arank << ", dimensions " ;
    }
    int nbelements=0;
    for (int i=0; i<arank; i++) {
      if (verbose) {
        std::cerr << (unsigned long)(adims_out[i]);
        if (i<arank-1) std::cerr << " x " ;
        else  std::cerr << "\n";
      }
      nbelements += adims_out[i];
    }
    std::vector<U>  vret(nbelements==0?1:nbelements);
    if (verbose)
      std::cerr << "nb elements = " << nbelements << "\n";
    DataType mem_type;
    switch (atype.getClass()) {
    case H5T_INTEGER :
      mem_type = PredType::NATIVE_INT;//H5T_NATIVE_INT;
      break;
    case H5T_FLOAT   :
      if (sizeof(U)==sizeof(double)) {
        mem_type = PredType::NATIVE_DOUBLE;//H5T_NATIVE_DOUBLE;
      } else {
        mem_type = PredType::NATIVE_FLOAT ;//H5T_NATIVE_FLOAT;
      }
      break;
    default :
      std::cerr << "We should not be here.....\n";
      assert(0);
    }

    attr.read(mem_type,&vret[0]);
    aspace.close();
    //atype.close();
    attr.close();
    grp.close();
    return vret;
  } catch (Exception &e) {
    std::cerr << "WARNING getAttribute, unable to load ["<<attr_name<<"]\n";
    throw(e);
  }
  std::vector<U> x(-1);
  return x;
}
// ============================================================================
// getDataset
// Read the corresponding dataset
// return a vector of data
// U dummy variable used to detect return type
template <class T>
template <class U> std::vector<U> GH5<T>::getDataset(std::string dset_name, U dummy)
{
  if (verbose) {
    std::cerr << "= = = = = = = = = = = = = = = = = =\n";
    std::cerr << "Dataset ["<<dset_name<< "]\n";
  }
  if (dummy) {;} // remove compiler warning
  // open dataset
  DataSet dataset = myfile->openDataSet(dset_name);
  // Get dataspace of the dataset
  DataSpace dataspace = dataset.getSpace();
  // Get the number of dimensions in the dataspace
  int rank = dataspace.getSimpleExtentNdims();
  hsize_t dims_out[rank];
  dataspace.getSimpleExtentDims( dims_out, NULL);
  if (verbose) {
    std::cerr << "rank " << rank << ", dimensions " ;
  }
  int nbelements=0;
  for (int i=0; i<rank; i++) {
    if (verbose) {
      std::cerr << (unsigned long)(dims_out[i]);
      if (i<rank-1) std::cerr << " x " ;
      else  std::cerr << "\n";
    }
    if (i==0) {
      nbelements = dims_out[i];
    } else {
      nbelements *= dims_out[i];
    }

  }
  std::vector<U>  vret(nbelements==0?1:nbelements);
  if (verbose)
    std::cerr << "nb elements = " << nbelements << "\n";

  //
  //H5T_class_t data_type = dataset.getTypeClass();
  DataType  data_type = dataset.getDataType();
  DataType mem_type;
  switch (data_type.getClass()) {
  case H5T_INTEGER :
    mem_type = PredType::NATIVE_INT;//H5T_NATIVE_INT;
    break;
  case H5T_FLOAT   :
    if (sizeof(U)==sizeof(double)) {
      mem_type = PredType::NATIVE_DOUBLE;//H5T_NATIVE_DOUBLE;
    } else {
      mem_type = PredType::NATIVE_FLOAT ;//H5T_NATIVE_FLOAT;
    }
    break;
   default :
    std::cerr << "We should not be here.....\n";
    assert(0);
  }

  // read vector of data
  dataset.read(&vret[0],mem_type,H5S_ALL,H5S_ALL);
  mem_type.close();
  data_type.close();
  dataspace.close();
  dataset.close();
  return vret;

}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Templates instantiation MUST be declared **AFTER** templates declaration
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
template std::vector<float> GH5<float>::getDataset(std::string dset_name, float dummy);
template std::vector<int  > GH5<float>::getDataset(std::string dset_name, int dummy);
template class GH5<float>;

} // end of namespace
