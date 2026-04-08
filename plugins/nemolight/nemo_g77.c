/* =================================================================
|  Copyright Jean-Charles LAMBERT - 2010                            
|  e-mail:   Jean-Charles.Lambert@lam.fr                           
|  address:  Dynamique des galaxies                                 
|            Laboratoire d'Astrophysique de Marseille               
|            2, place Le Verrier                                    
|            13248 Marseille Cedex 4, France                        
|            CNRS U.M.R 6110                                        
| ==================================================================
|* Main program necessary with g77 compiler                         
+----------------------------------------------------------------  */
void f_setarg(int argc, char ** argv);
void f_setsig();
void f_init();
void _gfortran_set_args (int argc, char ** argv);
extern int MAIN__();
int main(int argc, char ** argv)
{
#if G77
#pragma message "We assume that the default fortran compiler is g77" 
  f_setarg(argc, argv);
  f_setsig();
  f_init();
#endif
#if GFORT
#pragma message "We assume that the default fortran compiler is gfortran" 
  _gfortran_set_args (argc, argv);
#endif
  MAIN__();
  return 0; /* For compilers that complain of missing return values; */
}
/* ----------------------------------------------------------------
|  End of nemo_g77.c                                               
+---------------------------------------------------------------- */
