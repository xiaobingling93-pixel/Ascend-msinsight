#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_api_scene -s AddOp -p 2, --id test_IsSwitchAddOpDevId2

