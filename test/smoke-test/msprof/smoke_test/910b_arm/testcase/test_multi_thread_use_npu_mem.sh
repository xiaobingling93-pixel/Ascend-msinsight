#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/multi_thread_use_npu_mem.py -m multi -s multi -p , --id test_multi_thread_use_npu_mem