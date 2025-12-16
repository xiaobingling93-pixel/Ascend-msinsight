#!/bin/bash

mkdir -p build/intermediates/host/tmp 
cd build/intermediates/host; rm -rf ./*

source /usr/local/Ascend/ascend-toolkit/set_env.sh
# source /usr/local/Ascend/CANN-6.4/bin/setenv.bash
# cmake ../../../src -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
cmake ../../../src
make
