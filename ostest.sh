#!/bin/bash

OSTYPE=`uname -s`

if test "$OS" = "Windows_NT"  ; then
    SYST="win32"
else
    SYST="unknown"
fi


if test "$OSTYPE" = "GNU/Linux" -o "$OSTYPE" = "Linux"; then
    SYST="linux"
fi

if test "$OSTYPE" = "SunOS" ; then
    SYST="solaris"
fi

if test "$SYST" = "unknown" ; then
    SYST="linux"
fi

export SYST
echo Detected OS: $SYST
