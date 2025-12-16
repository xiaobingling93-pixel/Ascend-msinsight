#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_train_scene -s InnerOneStepPyTorch -p , --id test_IsSwitchInnerOneStep-Pytorch

