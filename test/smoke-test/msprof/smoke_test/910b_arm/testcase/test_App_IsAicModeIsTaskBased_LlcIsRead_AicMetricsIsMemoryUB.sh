#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s App -p "aic-mode=task-based","llc-profiling=read","aic-metrics=MemoryUB", --id test_App_IsAicModeIsTaskBased_LlcIsRead_AicMetricsIsMemoryUB
