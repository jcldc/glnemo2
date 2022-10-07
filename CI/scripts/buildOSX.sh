#!/bin/bash
# Build script for MacOSX gitlab runner

cmake . -DCMAKE_PREFIX_PATH=/Users/grunner/Qt/5.15.2/clang_64/lib/cmake
make -j 3
make package
