#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s All -p "aic-mode=task-based","llc-profiling=read","aic-metrics=ResourceConflictRatio", --id test_IsAicModeIsTaskBased_LlcIsRead_AicMetricsIsResourceConflictRatio

