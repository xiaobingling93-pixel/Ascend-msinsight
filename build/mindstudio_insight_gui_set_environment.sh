#!/bin/bash
npm install -g pnpm
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y tcl git gcc-11 g++-11 cmake ninja-build
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 --slave /usr/bin/g++ g++ /usr/bin/g++-11
apt-get install -y python3-pip
python3 -m pip install "pandas<=2.3.2" "numpy<=1.26.4" PyYaml tqdm sqlalchemy "xlsxwriter>=3.0.6"
