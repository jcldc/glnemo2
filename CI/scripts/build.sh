#!/bin/bash

echo "Before striping"
ldd /usr/lib64/qt5/bin/uic

strip --remove-section=.note.ABI-tag /lib64/libQt5Core.so

echo "After stripping"
ldd /usr/lib64/qt5/bin/uic


cmake .
make -j 8
strip bin/glnemo2
make package
