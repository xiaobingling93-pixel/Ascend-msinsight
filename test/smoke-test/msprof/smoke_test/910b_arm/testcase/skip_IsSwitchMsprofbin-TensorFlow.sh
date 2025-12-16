#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_train_scene -s MsprofbinTensorFlow -p , --id test_IsSwitchMsprofbin-TensorFlow

