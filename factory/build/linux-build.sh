#!/bin/bash

docker run -it -v ${PWD}:/works glnemo2/mageia4 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/mageia5 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/opensuse13.2 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/opensuse42.1 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/ubuntu14.04 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/ubuntu15.04 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/ubuntu15.10 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/ubuntu16.04 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/fedora20 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/fedora21 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/fedora22 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/fedora23 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/centos6 /works/build.sh
docker run -it -v ${PWD}:/works glnemo2/centos7 /works/build.sh
