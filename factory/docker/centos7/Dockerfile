FROM centos:7

RUN yum -y install epel-release &&\
rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-7 &&\
yum -y update &&\
yum -y install qt5-qtbase-devel.x86_64 make gcc-c++ subversion cfitsio-devel mesa-libGLU-devel.x86_64 zlib-devel rpm-build wget tar hdf5-devel &&\
wget http://heasarc.gsfc.nasa.gov/fitsio/ccfits/CCfits-2.4.tar.gz && tar zxvf CCfits-2.4.tar.gz && chown -R root:root CCfits && cd CCfits && ./configure --enable-static=yes --enable-shared=no --with-cfitsio-include=/usr/include/cfitsio && make && make install && cd .. && rm -rf CCfits CCfits-2.4.tar.gz &&\
wget https://cmake.org/files/v3.2/cmake-3.2.3-Linux-x86_64.sh && chmod +x cmake-3.2.3-Linux-x86_64.sh && ./cmake-3.2.3-Linux-x86_64.sh --prefix=/usr/local --skip-license && rm -f cmake-3.2.3-Linux-x86_64.sh

cmd ["/bin/bash"]

