FROM ubuntu:22.04

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
    libqt5opengl5-dev \
    libhdf5-dev \
    vim libsqlite3-dev

cmd ["/bin/bash"]

