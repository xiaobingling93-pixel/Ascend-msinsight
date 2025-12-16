#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_single_switch -s App -p "ascendcl=on", --id test_App_IsSwitchAscendcl

