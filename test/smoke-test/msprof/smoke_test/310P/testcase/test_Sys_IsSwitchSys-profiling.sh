#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)

python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s Sys -p "sys-profiling=on", --id test_Sys_IsSwitchSys-profiling

