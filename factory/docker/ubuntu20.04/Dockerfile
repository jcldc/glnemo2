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
    gpg wget ca-certificates \
    git \
    tmux \
    libglm-dev \
    libhdf5-dev \
    libfreetype-dev \
    libfuse2 \
    pkg-config \
    software-properties-common \
    curl \
    htop \
    libxkbcommon-x11-0 \
    libfontconfig \
    libdbus-1-3 \
    libx11-xcb1 \
    libxcb-util1 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shm0 \
    libxcb-sync1 \
    libxcb-xfixes0 \
    libxcb-render0 \
    libxcb-shape0 \
    libglib2.0-0 \
    libxcb-cursor0 libxcb-cursor-dev \
    vim libsqlite3-dev \
    libxcb1-dev libxcb-render0-dev libxcb-shape0-dev libxcb-xfixes0-dev libxcb-icccm4-dev libxcb-image0-dev \
    libxcb-keysyms1-dev libxcb-randr0-dev libxcb-render-util0-dev libxcb-shm0-dev libxcb-sync-dev \
    libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libx11-xcb-dev \
    libxcb-xinerama0-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-render-util0-dev libxcb-util-dev libxcb-cursor-dev \
    libgl1-mesa-dev libglu1-mesa-dev libegl1-mesa-dev libvulkan-dev \
    python3 python3-pip libgl1-mesa-dev libglu1-mesa-dev \
    libx11-dev libxcb1-dev libx11-xcb-dev libxcb-util-dev libxcb-image0-dev \
    libxcb-keysyms1-dev libxcb-render-util0-dev libxcb-icccm4-dev \
    libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
    libxcb-render0-dev libxcb-glx0-dev libxcb-xinerama0-dev libxkbcommon-dev \
    libxkbcommon-x11-dev libfontconfig1-dev libdbus-1-dev libzstd-dev

# install gcc 11
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt update -y && \
    apt install -y gcc-11 g++-11 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 50 --slave /usr/bin/g++ g++ /usr/bin/g++-9 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11


# install latest cmake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | \
    tee /etc/apt/sources.list.d/kitware.list && \
    apt update -y && \
    apt install -y cmake

# wget linuxdeployqt
# extract it in /usr/local
RUN cd /tmp && \
    wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -P /tmp && \
    chmod +x /tmp/linuxdeployqt-continuous-x86_64.AppImage && \
    cd /usr/local && \
    /tmp/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract

cmd ["/bin/bash"]

