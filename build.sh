#!/bin/bash

#./uninstall.sh
./clean.sh
./compile.sh
./makedoc.sh
./install.sh
./examples.sh

echo Build in $SECONDS seconds.
