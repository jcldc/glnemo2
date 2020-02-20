FROM centos:8

RUN yum -y install epel-release &&\
rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-8 &&\
yum -y install dnf-plugins-core &&\
yum config-manager --set-enabled PowerTools &&\
yum -y update &&\
yum -y install qt5-qtbase-devel.x86_64 wget make gcc-c++ git cfitsio-devel mesa-libGLU-devel.x86_64 zlib-devel libidn libtirpc-devel rpm-build glm-devel hdf5-devel &&\
wget http://heasarc.gsfc.nasa.gov/fitsio/ccfits/CCfits-2.4.tar.gz && tar zxvf CCfits-2.4.tar.gz && chown -R root:root CCfits && cd CCfits && ./configure --enable-static=yes --enable-shared=no --with-cfitsio-include=/usr/include/cfitsio && make && make install && cd .. && rm -rf CCfits CCfits-2.4.tar.gz &&\
wget https://cmake.org/files/v3.2/cmake-3.2.3-Linux-x86_64.sh && chmod +x cmake-3.2.3-Linux-x86_64.sh && ./cmake-3.2.3-Linux-x86_64.sh --prefix=/usr/local --skip-license && rm -f cmake-3.2.3-Linux-x86_64.sh


cmd ["/bin/bash"]

