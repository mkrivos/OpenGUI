#!/bin/bash
#########################################################################
#
########################################################################
. ostest.sh

if test "$SYST" = "solaris" ; then
    BASE_DIRECTORY="/usr/local"
    RM="rm -f"
else
    BASE_DIRECTORY="/usr"
    RM="rm -fv"
fi

#force
#BASE_DIRECTORY="/usr"

$RM $BASE_DIRECTORY/lib/libfgl*
#$RM $BASE_DIRECTORY/lib/fgl*
$RM $BASE_DIRECTORY/lib/libtinycfg*
$RM $BASE_DIRECTORY/lib/tinycfg*
$RM $BASE_DIRECTORY/lib/libFGMesa*
#$RM $BASE_DIRECTORY/lib/FGMesa*

$RM -R $BASE_DIRECTORY/include/fastgl
$RM -R $BASE_DIRECTORY/include/tiny*
