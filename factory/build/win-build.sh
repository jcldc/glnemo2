#!/bin/bash

export PATH=/home/jcl/works/GIT/mxe/usr/bin:${PATH}
REP=${PWD}

cd ${REP}/glnemo2/GIT/glnemo2

MAJOR=`grep GLNEMO2_MAJOR src/version.h |cut -d\" -f 2`
MINOR=`grep GLNEMO2_MINOR src/version.h |cut -d\" -f 2`
PATCH=`grep GLNEMO2_PATCH src/version.h |cut -d\" -f 2`
EXTRA=`grep GLNEMO2_EXTRA src/version.h |cut -d\" -f 2`

RELEASE_NAME=$MAJOR"."$MINOR"."${PATCH}${EXTRA}

echo $RELEASE_NAME

mkdir -p buildwin32
cd buildwin32
/home/jcl/works/GIT/mxe/usr/i686-w64-mingw32.static/qt5/bin/qmake -recursive ../glnemo2.pro
make -j 6
cp -p bin/release/win32/glnemo2-win32.exe  ../../../glnemo2-${RELEASE_NAME}.win32.exe
7z a ../../glnemo2-${RELEASE_NAME}.win32.zip ../../../glnemo2-${RELEASE_NAME}.win32.exe

cd ${REP}/glnemo2/GIT/glnemo2
mkdir -p buildwin64
cd buildwin64
/home/jcl/works/GIT/mxe/usr/x86_64-w64-mingw32.static/qt5/bin/qmake -recursive ../glnemo2.pro
make -j 6
cp -p bin/release/win32/glnemo2-win64.exe  ../../../glnemo2-${RELEASE_NAME}.win64.exe
7z a ../../glnemo2-${RELEASE_NAME}.win64.zip ../../../glnemo2-${RELEASE_NAME}.win64.exe



# to compress
# 7z a glnemo2-1.9.0-win64.zip glnemo2-1.9.0-win64.exe




