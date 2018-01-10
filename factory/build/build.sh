#!/bin/bash

#cd /works/glnemo2
#cd GIT/glnemo2 
#rm -rf build-docker
#mkdir build-docker
cd /tmp
git clone https://gitlab.lam.fr/jclamber/glnemo2.git 
cd glnemo2
mkdir build-docker
cd build-docker
cmake ..
make -j 8
strip bin/glnemo2
make package
cp -p *.rpm *.deb /works/glnemo2

