#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_pytorch_profiling -s cann_profiling -p , --id test_pytorch_cann_profiling