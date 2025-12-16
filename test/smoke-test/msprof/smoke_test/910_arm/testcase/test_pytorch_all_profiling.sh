#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_pytorch_profiling -s all_profiling -p , --id test_pytorch_all_profiling