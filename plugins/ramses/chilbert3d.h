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
#ifndef CHILBERT3D_H
#define CHILBERT3D_H

#include <string>
#include <map>
#include <vector>

namespace ramses {
class CHilbert3D
{
public:
  CHilbert3D(const std::string _indir );
  ~CHilbert3D();
  bool process(double xmin, double xmax,
               double ymin, double ymax,
               double zmin, double zmax
               );
  std::string getOrdering() { return ordering; }
  std::vector<int> getCpuList() { return cpu_list; }
  void printCpuList();

private:
  static const int state_diagram[12][2][8];
  int ncpu;
  bool * cpu_read;
  //int * cpu_list;
  std::vector<int> cpu_list;
  double * bound_key;
  std::string indir, info_file;
  std::string s_run_index,ordering;
  std::map<std::string, double > mapinfo;
  bool readInfoFile();

  void hilbert3D(const int x, const int y, const int z,
                 double &order, const int bit_length, const int npoint);
};
} // namespace ramses
#endif // CHILBERT3D_H
