######################################################################
# Automatically generated by qmake (2.01a) Tue Oct 2 16:28:22 2007
######################################################################
# include global configuration
include(../../config.arch)

OBJECTS_DIR = .obj/$${ARCH}/$$COMPILEMODE
DESTDIR = lib/$${ARCH}/$$COMPILEMODE
TARGET = gadgeth5

CONFIG += $$GLOBAL warn_on \
          lib  \
	  static \
          staticlib
          
TEMPLATE = lib

INCLUDEPATH += ../../src

HEADERS +=  \
    gadgeth5.h

SOURCES +=  \
    gadgeth5.cc

INCLUDEPATH += ../../src /usr/include/hdf5/serial

#DISTFILES += \
#    ../gadgeth5plugin.json
