FROM fedora:27

RUN /usr/bin/dnf -y update
RUN /usr/bin/dnf -y install qt5-qtbase-devel.x86_64 make cmake gcc-c++ gcc-gfortran git CCfits-devel mesa-libGLU-devel.x86_64 zlib-devel rpm-build hdf5-devel

cmd ["/bin/bash"]

