#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_train_scene -s ImageNetTrain -p, --id test_Mini-ImageNet-Pytorch