FROM ubuntu:20.04

RUN apt-get -y update && \
    apt-get -y upgrade && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get -y install --fix-missing \
    tzdata \
    cmake \
    dpkg-dev \
    file \
    g++ \
    gcc \
    libccfits-dev \
    make \
    subversion \
    git \
    libglm-dev \
    qt5-default \
    libhdf5-dev \
    vim libsqlite3-dev

cmd ["/bin/bash"]

