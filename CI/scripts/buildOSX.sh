#!/bin/bash
# Build script for MacOSX gitlab runner

cmake . -DCMAKE_PREFIX_PATH=/Users/jcl/Qt2/5.10.0/clang_64/lib/cmake 
make -j 3
make package
