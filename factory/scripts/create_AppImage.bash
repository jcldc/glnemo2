#!/bin/bash

# version.h file name
FILE="version.h"

# check if file exist
if [ ! -f "$FILE" ]; then
	    echo "Error : File $FILE does not exist."
      exit 1
fi

# extract values from file
MAJOR=$(grep -E '#define GLNEMO2_MAJOR' "$FILE" | awk -F '"' '{print $2}')
MINOR=$(grep -E '#define GLNEMO2_MINOR' "$FILE" | awk -F '"' '{print $2}')
PATCH=$(grep -E '#define GLNEMO2_PATCH' "$FILE" | awk -F '"' '{print $2}')
EXTRA=$(grep -E '#define GLNEMO2_EXTRA' "$FILE" | awk -F '"' '{print $2}')

# Optionnal : remove "-' from EXTRA if it exists" (ex: "-fcf9711" -> "fcf9711")
#EXTRA_CLEAN=$(echo "$EXTRA" | sed 's/^-//')

# build VERSION variable 
VERSION="${MAJOR}.${MINOR}.${PATCH}${EXTRA}"

# print result
echo "VERSION=$VERSION"

# copy desktop and icons
cp /glnemo2/res/desktop/glnemo2.desktop .
cp /glnemo2/res/images/glnemo2.png .

# create app image
VERSION="$VERSION" /usr/local/squashfs-root/AppRun glnemo2.desktop -qmake=/data/QT6/qt6.11.2/bin/qmake -appimage
