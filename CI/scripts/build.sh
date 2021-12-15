#!/bin/bash


# strip qt5 lib as a workaround on unresolved dependency
# see https://github.com/microsoft/WSL/issues/3023

# fedora/opensuse etc...
if [ -f /usr/lib64/libQt5Core.so ]; then
    strip --remove-section=.note.ABI-tag /usr/lib64/libQt5Core.so
fi

# ubuntu
if [ -f /lib/x86_64-linux-gnu/libQt5Core.so ]; then
    strip --remove-section=.note.ABI-tag /lib/x86_64-linux-gnu/libQt5Core.so
fi

cmake . -DCMAKE_SKIP_RPATH:BOOL=ON
make -j 8
strip bin/glnemo2
make package
