#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s Host -p "runtime-api=on", --id test_App_IsSwitchRuntime-api

