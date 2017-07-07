FROM opensuse:13.2 

RUN zypper -n update &&\
zypper -n install gcc-c++ gcc cmake &&\
zypper -n install libqt5-qtbase-devel &&\
zypper -n install cfitsio-devel zlib-devel &&\
zypper -n install glu-devel &&\
zypper -n install subversion rpm-build &&\
zypper -n install wget hdf5-devel &&\
wget http://heasarc.gsfc.nasa.gov/fitsio/ccfits/CCfits-2.4.tar.gz && tar zxvf CCfits-2.4.tar.gz && chown -R root:root CCfits && cd CCfits && ./configure --enable-static=yes --enable-shared=no --with-cfitsio-include=/usr/include/cfitsio && make && make install && cd .. && rm -rf CCfits CCfits-2.4.tar.gz

cmd ["/bin/bash"]

