FROM centos:6.6

RUN yum -y install epel-release &&\
rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-6 &&\
yum -y update &&\
yum -y install qt5-qtbase-devel.x86_64 make cmake gcc-c++ subversion cfitsio-devel mesa-libGLU-devel.x86_64 zlib-devel rpm-build wget tar hdf5-devel &&\
wget http://heasarc.gsfc.nasa.gov/fitsio/ccfits/CCfits-2.4.tar.gz && tar zxvf CCfits-2.4.tar.gz && chown -R root:root CCfits && cd CCfits && ./configure --enable-static=yes --enable-shared=no --with-cfitsio-include=/usr/include/cfitsio && make && make install && cd .. && rm -rf CCfits CCfits-2.4.tar.gz

cmd ["/bin/bash"]

