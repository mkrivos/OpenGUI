#!/bin/bash
#########################################################################
#
########################################################################

. ostest.sh

if [ "$SYST" != "win32" ] ; then
if ! test $UID = "0" ; then
echo You must be logged as root!
exit
fi
fi 


BASE_DIRECTORY="/usr"

if [ "$SYST" = "solaris" ] ; then
    PATH+=/opt/sfw/bin:/opt/sfw/lib/bin
    BASE_DIRECTORY="/usr/local"
fi

INSTALL_INC_DIR=$BASE_DIRECTORY/include
INSTALL_INC_DIR_FGL=$INSTALL_INC_DIR/fastgl
INSTALL_LIB_DIR=$BASE_DIRECTORY/lib

if [ ! -d $BASE_DIRECTORY ]; then
    bash ./inst-sh -d -g 0 -o 0 -m 755 BASE_DIRECTORY
fi

if [ ! -d $INSTALL_LIB_DIR ]; then
    bash ./inst-sh -d -g 0 -o 0 -m 755 INSTALL_LIB_DIR
fi

if [ ! -d $INSTALL_INC_DIR ]; then
    bash ./inst-sh -d -g 0 -o 0 -m 755 INSTALL_INC_DIR
fi

# INCLUDE
echo Installing includes ...
bash ./inst-sh -d -g 0 -o 0 -m 755 $INSTALL_INC_DIR_FGL

cd include
for file in fastgl/*
do
    bash ../inst-sh -c -g 0 -o 0 -m 0644 "$file" $INSTALL_INC_DIR/"$file"
    echo -Installing $file to $INSTALL_INC_DIR
done
cd ..
 
# LIBRARIES
echo
echo Installing static libraries...
cd lib
for file in *.a
do
    bash ../inst-sh -c -g 0 -o 0 -m 0644 "$file" $INSTALL_LIB_DIR/"$file"
    echo -Installing $file to $INSTALL_LIB_DIR
done

if test "$SYST" = "linux" ; then
    echo
    echo Installing dynamic libraries...
    for file in *.so.* *.so
    do
	bash ../inst-sh -c -g 0 -o 0 -m 0755 "$file" $INSTALL_LIB_DIR/"$file"
        echo -Installing $file to $INSTALL_LIB_DIR
    done
    ldconfig
fi
cd ..

echo
echo 'Include files installed to: ' $INSTALL_INC_DIR_FGL
echo 'Library files installed to: ' $INSTALL_LIB_DIR
echo
