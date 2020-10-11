#!/bin/bash

. ostest.sh

echo
echo Building examples ...

cd examples

for file in *
do
    if test "$file" != "data" -a "$file" != "Makefile"; then
        echo - building $file
	gmake -f Makefile target=$file/$file
#    	gmake -B -f Makefile target=$file/$file
    fi
done

cd ..
