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

cd ${REP}

make -f mmakefile tar

mv ${REP}/glnemo2/glnemo2.tar.gz ${REP}/glnemo2/glnemo2-${RELEASE_NAME}.tar.gz

