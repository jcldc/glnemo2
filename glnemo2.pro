SUBDIRS += \
           utils \
           3rdparty/pfntlib \
           plugins/ramses    \
           plugins/gadget    \
           plugins/gadgeth5  \
           plugins/nemolight \
	   plugins/ftm \
           plugins/zlib \
           plugins/network \
           plugins/tipsy \
	   plugins \
	   src
TEMPLATE = subdirs 

win32 {
SUBDIRS -= plugins/tipsy
}
