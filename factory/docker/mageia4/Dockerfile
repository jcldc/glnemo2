FROM mageia:4 

COPY urpmi.cfg /etc/urpmi/
RUN urpmi --auto-update --auto &&\
urpmi --auto coreutils gcc gcc-c++ cmake make qt5-devel CCfits-devel subversion libgomp-devel lib64mesaglu1-devel rpm-build lib64hdf5-devel
COPY issue /etc/issue
CMD ["/bin/bash"]
