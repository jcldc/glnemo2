#!/bin/bash

cmake .
make -j 8
strip bin/glnemo2
make package
