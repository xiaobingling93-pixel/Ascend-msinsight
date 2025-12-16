#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
cp ../script/using_value.py ../../using_value.py
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s App -p "l2=on --task-time=on", --id test_App_IsSwitchL2
