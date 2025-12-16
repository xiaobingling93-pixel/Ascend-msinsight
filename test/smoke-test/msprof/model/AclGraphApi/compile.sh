#! /bin/bash

# compile
home_dir=`pwd`

if [ ! -d "build" ]; then
	mkdir ./build
fi
rm -rf build
mkdir build
cd ./build
cmake .. -DSTUB=NO
make

