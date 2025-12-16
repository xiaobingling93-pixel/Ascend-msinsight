#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
cp ../script/using_value.py ../../using_value.py
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s Sys -p "sys-interconnection-profiling=on", --id test_Sys_IsSwitchSys-interconnection-profiling

