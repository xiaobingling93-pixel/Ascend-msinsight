#!/bin/bash
export CI=true
cd server/build
python3 download_third_party.py
python3 preprocess_third_party.py
python3 build.py build
cd ..
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}/output/linux-aarch64/bin
cd ../modules/build
python3 build.py
cd ../../e2e
npm ci
npm run test:smoke
