#!/bin/bash
#########################################################################
#
########################################################################

. ostest.sh

rm -f .rmlist

#RM :=rm -fv

find . -name *.[oad] >> .rmlist
find . -name *.lo >> .rmlist
find . -name *.obj >> .rmlist
find . -name *.exe  >> .rmlist
find . -name *.lib  >> .rmlist
find . -name *.so.* >> .rmlist 
find . -name *.so >> .rmlist 
find . -name core  >> .rmlist
find . -name a.out  >> .rmlist
find . -name *.tds >> .rmlist
find . -name *.exe >> .rmlist
find . -name *.d.* >> .rmlist

for file in $(cat .rmlist)
do
    rm -f $file
done

cd examples
for file in *
do
    rm -f $file/$file
    rm -f $file/"$file"3
done
cd ..

rm -f -R doc/html
rm -f .rmlist
rm -f *.*~
rm -f kylix3/demo
rm -f rhide/main
rm -f examples/bitmap/window.bmp

