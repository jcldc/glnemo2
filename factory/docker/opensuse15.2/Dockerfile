FROM opensuse/leap:15.2

RUN zypper -n update &&\
zypper -n install gcc-c++ gcc cmake libtirpc-devel &&\
zypper -n install libqt5-qtbase-devel &&\
zypper -n install cfitsio-devel zlib-devel &&\
zypper -n install glu-devel &&\
zypper -n install git glm-devel subversion rpm-build &&\
zypper -n install wget hdf5-devel &&\
zypper -n install sqlite3-devel vim &&\
wget http://heasarc.gsfc.nasa.gov/fitsio/ccfits/CCfits-2.6.tar.gz && tar zxvf CCfits-2.6.tar.gz && chown -R root:root CCfits-2.6 && cd CCfits-2.6 && ./configure --enable-static=yes --enable-shared=no --with-cfitsio-include=/usr/include/cfitsio && make && make install && cd .. && rm -rf CCfits-2.6 CCfits-2.6.tar.gz

cmd ["/bin/bash"]

