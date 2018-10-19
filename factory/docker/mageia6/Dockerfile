FROM mageia:6 

RUN urpmi.addmedia --distrib --mirrorlist 'http://mirrors.mageia.org/api/mageia.6.$ARCH.list' && urpmi --auto --auto-update && urpmi --auto coreutils gcc gcc-c++ cmake make qt5-devel CCfits-devel subversion libgomp-devel lib64mesaglu1-devel rpm-build hdf5-devel git glm-devel tmux lib64sqlite3-devel python-numpy python3-numpy python-setuptools python3-setuptools swig lib64python-devel lib64python3-devel python-numpy-devel python3-numpy-devel   
CMD ["/bin/bash"]
