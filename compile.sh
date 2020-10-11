#!/bin/bash
#########################################################################
#
#########################################################################
# all
# clean
# doc
# -dist
#

. ostest.sh

build_opengui()
{
	cd src
	echo 'Building library for 8-bit colors ...'
	gmake -j 2 -f Makefile $1 BPP=8 LIBNAME=fgl
	echo 'Building library for 16-bit colors ...'
	gmake -j 2 -f Makefile $1 BPP=16 LIBNAME=fgl16
	echo 'Building library for 32-bit colors ...'
	gmake -j 2 -f Makefile $1 BPP=32 LIBNAME=fgl32
	cd ..
}

make_objs_dirs()
{
	for file in linux solaris win32 mesa x11 agg2
	do
#		mkdir obj/gcc/8/"$file"
#		mkdir obj/gcc/15/"$file"
#		mkdir obj/gcc/16/"$file"
#		mkdir obj/gcc/32/"$file"
	:
	done
}

increment_version()
{
    File=version
    {
	read MAJOR
	read MINOR
	read PATCH
	read BUILD
    } < $File
    STR=$MAJOR.$MINOR.$PATCH.$BUILD
    echo '#'define FG_VERSION '"'$STR'"' > include/fastgl/fgversion.h
    echo '#'define FG_MAJOR $MAJOR >> include/fastgl/fgversion.h
    echo '#'define FG_MINOR $MINOR >> include/fastgl/fgversion.h
    echo '#'define FG_PATCH $PATCH >> include/fastgl/fgversion.h
    let "BUILD+=1"
    echo $MAJOR > version
    echo $MINOR >> version
    echo $PATCH >> version
    echo $BUILD >> version
}

make_objs_dirs

if test "$SYST" = "solaris"; then
   PATH=$PATH:/opt/sfw/bin:/opt/sfw/lib/bin
   export PATH
fi

if test "$SYST" = "win32"; then
   cp -f /bin/make.exe /bin/gmake.exe
fi

increment_version
build_opengui

echo OpenGUI version: $STR was built
