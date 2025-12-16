#!/bin/bash

cd mspti_tools
rm -r bin
mkdir bin
cd bin
cmake ..
make
