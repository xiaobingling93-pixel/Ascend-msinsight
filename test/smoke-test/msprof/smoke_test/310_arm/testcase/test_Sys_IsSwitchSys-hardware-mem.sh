#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s Sys -p "sys-hardware-mem=on --llc-profiling=capacity", --id test_Sys_IsSwitchSys-hardware-mem

