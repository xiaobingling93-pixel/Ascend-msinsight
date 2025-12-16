#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)

python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s Sys -p "aic-mode=task-based","llc-profiling=read","aic-metrics=PipeUtilization", --id test_Sys_IsAicModeIsTaskBased_LlcIsRead_AicMetricsIsPipeUtilization

