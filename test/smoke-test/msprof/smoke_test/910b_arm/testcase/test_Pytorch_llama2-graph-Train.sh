#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_train_scene -s Pytorch-llama2-graph -p, --id test_Pytorch_llama2-graph-Train