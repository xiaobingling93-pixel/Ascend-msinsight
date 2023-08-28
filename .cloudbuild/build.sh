#!/bin/bash
# Cpp Debug Server
# Copyright Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.

set -e
wget https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general/CMake/3.20.5/package/cmake-3.20.5-linux-x86_64.tar.gz --no-check-certificate
tar -xf cmake-3.20.5-linux-x86_64.tar.gz -C $WORKSPACE
export PATH=$WORKSPACE/cmake-3.20.5-linux-x86_64/bin:$PATH

cd $WORKSPACE
wget https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general/ninja/1.10.1/package/ninja-linux.zip --no-check-certificate
if [ -d "ninja" ]; then
  rm -rf ninja
fi
mkdir ninja
unzip ninja-linux.zip -d $WORKSPACE/ninja
export PATH=$WORKSPACE/ninja:$PATH

export PATH=/opt/buildtools/gcc-7.3.0/bin/:$PATH
echo $PATH

cd $WORKSPACE/code
cd third_party/sqlite_src
if [ -d "bld" ]; then
  rm -rf bld
fi
mkdir bld
cd bld
chmod +x ../configure
../configure
make sqlite3.c

cd $WORKSPACE/code/third_party/sqlite
if [ -d "include" ]; then
  rm -rf include
fi
mkdir include
if [ -d "src" ]; then
  rm -rf src
fi
mkdir src
cd $WORKSPACE/code/third_party/sqlite_src/bld
cp sqlite3.h ../../sqlite/include
cp sqlite3ext.h ../../sqlite/include
cp sqlite3.c ../../sqlite/src
cp shell.c ../../sqlite/src
cd $WORKSPACE/code/third_party/
if [ -d "json_modern_c++" ]; then
  rm -rf json_modern_c++
fi
mkdir json_modern_c++
cd json_modern_c++
if [ -d "include" ]; then
  rm -rf include
fi
mkdir include
cd ..
cp json/single_include/nlohmann/json.hpp json_modern_c++/include


cd $WORKSPACE/code/build
python3 build.py grpc --build
python3 build.py grpc --proto
python3 build.py build --release --project_type=server --project_subtype=bin
