#!/bin/bash

strip --remove-section=.note.ABI-tag /lib64/libQt5Core.so

cmake .
make -j 8
strip bin/glnemo2
make package
