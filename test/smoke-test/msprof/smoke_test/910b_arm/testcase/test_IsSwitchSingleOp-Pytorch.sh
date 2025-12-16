#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_train_scene -s SingleOpPyTorch -p , --id test_IsSwitchSingleOp-Pytorch

