# Introduction

**Glnemo2** is an interactive 3D visualization program which displays particles positions of the different components (gas, stars, disk, dark mater halo, bulge) of an N-body snapshot. It's a very useful tool for everybody running N-body simulations from isolated galaxies to cosmological simulations. It can show quickly a lot of information about data by revealing shapes, dense areas, formation of structures such as spirals arms, bars, peanuts or clumps of galaxies. Glnemo2 has been designed to meet the requirements of the user, with simplicity in mind, easy to install, easy to use with an interactive and responsive graphical user interface (based on QT 5.X API) , powerful with a fast 3D engine (OPenGL and GLSL), and generic with the possibility to load different kinds of input files.

You can zoom in/out, rotate, scale, translate, select different particles
and plot them in different blending colors, color particles according to their density,
play with the density threshold, trace orbits, play different time steps, take automatic
screenshots to make movies, select particles using the mouse, fly over your simulation
using camera path. All this features are accessible from a very intuitive graphic user interface.

Currently, glnemo2 reads:
* NEMO files (http://carma.astro.umd.edu/nemo)
* Gadget 1 and 2 -little and big endian- files (http://www.mpa-garching.mpg.de/gadget/)
* Gadget 3 and 4 (HDF5) (http://www.mpa-garching.mpg.de/gadget4/)
* RAMSES files (http://www.ics.uzh.ch/~teyssier/ramses/RAMSES.html)
* TIPSY files (http://www-hpcc.astro.washington.edu/tools/tipsy/tipsy.html)
* FITS files, 2D and 3D datacube (http://fits.gsfc.nasa.gov/)
* FTM files (Clayton Heller's sph/nbody code)
* phiGRAPE file (http://www-astro.physik.tu-berlin.de/~harfst/index.php?pid=8)
* list of files stored in a file
* realtime gyrfalcON simulation via a network plugin (see $NEMO/usr/jcl/glnemo2/gyrfalcon/README)

Glnemo2 uses a plugin mechanism to load data, so it's easy to add a new file reader.
It uses the latest OpenGL technology like, shaders (glsl), vertex buffer object, frame buffer object, and takes in account the power of your graphic card to accelerate the rendering. Millions of particles can be rendered, in real time, with a fast GPU.

Glnemo2 runs fine on Linux, Windows(using minGW compiler), and MaxOSX, thanks to the QT5 API.

0. how to compile and install glnemo2, see **INSTALL**.
 
1. how to use it, see **MANUAL**.
 
2. test purpose files: see **snapshot/README**

# Webpage
 Glnemo2 has a dedicated web site : http://projets.lam.fr/projects/glnemo2/wiki

# License

Glnemo2 is governed by the CeCILL2  license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info/index.en.html" and/or LICENSE file.                       
              
                             
# © Copyright
                             --  Glnemo2 --
                                 
                Copyright © Jean-Charles LAMBERT 2007-2023
                 http://projets.lam.fr/projects/glnemo2
 
                       Jean-Charles.Lambert_AT_lam_DOT_fr
                         Dynamique des Galaxies
              Centre de donnéS Astrophysique de Marseille (CeSAM)
                  Laboratoire d'Astrophysique de Marseille
                           (CNRS U.M.R 7326)
                 Pôle de l'Étoile Site de Château-Gombert
                       38, rue Frédéric Joliot-Curie
                          13388 Marseille cedex 13
                                  FRANCE

