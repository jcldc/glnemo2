

# Build docker
```
docker build -t ubuntu20.04-app-image factory/docker/ubuntu20.04

```

# Run docker
```
# /data is where Qt6 is installed
# /glnemo2 is glnemo2 git directory

docker run -it -v /data:/data -v /home/jcl/works/GIT/glnemo2:/glnemo2 ubuntu20.04-app-image:latest
```

# cmake and gcc 11 installation mandatory to compile qt
```
# Ajouter le PPA des outils de compilation Ubuntu
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update

# Installer GCC 11 et G++ 11
sudo apt install -y gcc-11 g++-11

# set GCC 9 at priority 50
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 50 --slave /usr/bin/g++ g++ /usr/bin/g++-9

# set GCC 11 at priority 100
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11

# kitware / cmake
apt update && sudo apt install -y gpg wget ca-certificates
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | sudo tee /etc/apt/sources.list.d/kitware.list

sudo apt update
sudo apt install -y cmake
```

# qt6 archive  (from HOST)
Get a tarball of the qt6 version you want to compile.
Do it from the host, not from the docker
```
cd /data/QT6
# for LTS choose 6.8.4
# wget https://download.qt.io/official_releases/qt/6.8/6.8.4/single/qt-everywhere-opensource-src-6.8.4.tar.xz
# tar -xf qt-everywhere-opensource-src-6.8.4.tar.xz


# Qt 6.11.2
wget https://download.qt.io/official_releases/qt/6.11/6.11.2/single/qt-everywhere-src-6.11.2.tar.xz
tar -xvf qt-everywhere-src-6.11.2.tar.xz
```

# qt6 compilation (from docker)
```
cd /data/QT6/qt-everywhere-src-6.11.2
../configure -prefix /data/QT6/qt6.11.2   -release   -xcb    -nomake examples   -nomake tests   -skip qtwebengine,qtpdf,qtdoc,qtscxml,qtlocation
cmake --build . --parallel 8
cmake --install .
```


# AppImage creation
## extract
We must extract linuxdeployqt
```
# get linuxdeployqt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract

```

## create a TOML for the application

```
cat << EOF > glnemo2.desktop
[Desktop Entry]
Type=Application
Name=glnemo2
Comment=Interactive 3D visualization program for N-body snapshots
Exec=bin/glnemo2
Icon=glnemo2
Categories=Science;Graphics;
Terminal=false
EOF
```

## AppImage creation
```
cd /glnemo2
mkdir build-docker
cd build-docker
cmake .. -DCMAKE_PREFIX_PATH=/data/QT6/qt6.11.2/lib/cmake
make -j 8
VERSION=2.0.0 /usr/local/squashfs-root/AppRun glnemo2.desktop -qmake=/opt/qt6.11.2/bin/qmake -appimage

```
