#!/bin/bash

# useful functions
checkIfPrereqPresent ()
{
	command -v $1 >/dev/null 2>&1 || { echo "I require a binary called: $1 , but it's not installed.  Aborting."; exit 1; }
}

exitIfPathDoesNotExits()
{
  if [ ! -d $1 ]; then
    echo "The path: \"$1\" does not exit have you installed/downloaded everything? Aborting";
	exit 1;
  fi
}

createDirWhenNotFound ()
{
  if [ ! -d $1 ]; then
    mkdir $1
  fi
}

deleteDirIfFound ()
{
  if [ -d $1 ]; then
	echo "removing \"$1\""
    rm -rfv $1
  fi
}


checkIfPrereqPresent jq
checkIfPrereqPresent unzip
checkIfPrereqPresent wget


# CONFIGURATION VARIABLES
ESP_VERSION=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.espVersions.ESP8266_VER' | cut -d "\"" -f 2`
DOWNLOAD_FOLDER=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.paths.cacheFolder' | cut -d "\"" -f 2`
LIBS_FOLDER=`cat ../../Esp8266-Arduino-Makefile/config.json | jq '.paths.librariesFolder' | cut -d "\"" -f 2`

# LIBRARIES
LIB_LINKS=(`cat libraries.json | jq --compact-output '.[].weblink' | cut -d "\"" -f 2`)
LIB_NAMES=(`cat libraries.json | jq --compact-output '.[].folderName' | cut -d "\"" -f 2`)

# Make sure that the folder names in the libraries.json match the include libraries header files <fileName.h>
# This is how we detetmine in espXArduino.mk which Esp Arduino libraries have to be compiled.

# Check if the esp-8266 project and other depencies are there
exitIfPathDoesNotExits "../../Esp8266-Arduino-Makefile"
exitIfPathDoesNotExits $LIBS_FOLDER

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
    deleteDirIfFound $LIBS_FOLDER/${LIB_NAMES[$i]}
    cp -a $DOWNLOAD_FOLDER/${LIB_NAMES[$i]}/$zipInnerFolder/. $LIBS_FOLDER/${LIB_NAMES[$i]}/
    ((i++))
done
