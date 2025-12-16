#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s App -p "host-sys-usage=cpu.mem --task-time=on", --id test_App_IsSwitchHost-sys-usage

