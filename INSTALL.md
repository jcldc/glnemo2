  
# Requirements:

Glnemo2 compiles and runs fine on Linux, Windows and MacOSX platform.
To compile it, you need QT6 library development kit.
Go to https://download.qt.io/official_releases/qt/ to download QT.

You also need a decent video card with a fast opengl driver. Glnemo2
has been successfully tested on Nvidia, Intel and  ATI GPU cards. 
visit http://projets.lam.fr/projects/glnemo2/wiki/Wiki#Installation

## Libraries dependencies

To compile glnemo2 2.x.x you need the following dev packages and libraries : 
- HDF5 lib
- Freetype2 lib
- cfitsio lib
- CCFits lib
- OpenGL lib
- QT6 lib
- cmake

# Compilation :


```
# git clone the project
git clone https://gitlab.lam.fr/jclamber/glnemo2.git
cd glnemo2

# change branch if necessary
# git checkout origin/develop -b develop

# run cmake
# let assume that QT directory is located at /data/QT6/qt6.11.2 
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/data/QT6/qt6.11.2/lib/cmake
make -j 4

```


if the compilation fails, please send me a full report by e-mail : 
jean-charles.lambert_at_lam.fr 

it should take a while to compile, at the end you should have a 'glnemo2' binary located in
"bin/glnemo2" directory.


# Binaries

You can download and install directly glnemo2 binary for different linux distributions, MacOSX and windows 32 et 64 bits platform.
See http://projets.lam.fr/projects/glnemo2/wiki/Download

