#!/bin/bash


# strip qt5 lib as a workaround on unresolved dependency
# see https://github.com/microsoft/WSL/issues/3023
strip --remove-section=.note.ABI-tag /lib64/libQt5Core.so

cmake .
make -j 8
strip bin/glnemo2
make package
