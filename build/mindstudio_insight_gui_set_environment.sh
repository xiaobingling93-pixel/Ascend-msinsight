#!/bin/bash
npm install -g pnpm
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y tcl git gcc-11 g++-11 cmake ninja-build
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 --slave /usr/bin/g++ g++ /usr/bin/g++-11
