#!/bin/bash

cd /works/glnemo2
cd GIT/glnemo2 
rm -rf build
mkdir build
cd build
cmake ..
make -j 6
strip bin/glnemo2
make package
cp -p *.rpm *.deb /works/glnemo2

