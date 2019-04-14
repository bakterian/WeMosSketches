#!/bin/bash

# CONFIGURATION VARIABLES
ESP_VERSION=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.espVersions.ESP8266_VER' | cut -d "\"" -f 2`
DOWNLOAD_FOLDER=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.paths.cacheFolder' | cut -d "\"" -f 2`
LIBS_FOLDER=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.paths.librariesFolder' | cut -d "\"" -f 2`

# LIBRARIES
LIB_LINKS=(`cat libraries.json | jq --compact-output '.[].weblink' | cut -d "\"" -f 2`)
LIB_NAMES=(`cat libraries.json | jq --compact-output '.[].folderName' | cut -d "\"" -f 2`)

# Make sure that the folder names in the libraries.json match the include libraries header files <fileName.h>
# This is how we detetmine in espXArduino.mk which Esp Arduino libraries have to be compiled.

createDirWhenNotFound ()
{
  if [ ! -d $1 ]; then
    mkdir $1
  else
    rm -rfv $1/*
  fi
}

# download folder creation if not already exists
createDirWhenNotFound $DOWNLOAD_FOLDER

# iterate over the libraries, download them and extract them alongside the ESP Core libs
i=0
for link in "${LIB_LINKS[@]}"
do
    echo "downloading from $link"
    wget --no-clobber -q $link -P $DOWNLOAD_FOLDER/${LIB_NAMES[$i]}
    zipFileName=$(basename $link)
    zipInnerFolder=`unzip -qql $DOWNLOAD_FOLDER/${LIB_NAMES[$i]}/$zipFileName | head -n1 | tr -s ' ' | cut -d' ' -f5- | sed 's/.$//'`
    unzip -o $DOWNLOAD_FOLDER/${LIB_NAMES[$i]}/$zipFileName -d $DOWNLOAD_FOLDER/${LIB_NAMES[$i]} 
    createDirWhenNotFound $LIBS_FOLDER/${LIB_NAMES[$i]}
    cp -a $DOWNLOAD_FOLDER/${LIB_NAMES[$i]}/$zipInnerFolder/. $LIBS_FOLDER/${LIB_NAMES[$i]}/
    ((i++))
done
