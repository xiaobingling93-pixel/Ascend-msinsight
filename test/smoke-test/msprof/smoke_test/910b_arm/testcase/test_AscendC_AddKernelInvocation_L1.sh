#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_api_scene -s AddKernelInvocation_L1 -p 0, --id test_AscendC_AddKernelInvocation_L1

