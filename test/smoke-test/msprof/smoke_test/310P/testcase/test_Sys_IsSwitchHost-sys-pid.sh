#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)

python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s Sys -p "host-sys-pid=1 --sys-cpu-profiling=on --host-sys=cpu", --id test_Sys_IsSwitchHost-sys-pid

