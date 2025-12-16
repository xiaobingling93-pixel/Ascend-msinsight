#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s All -p "aic-mode=task-based","llc-profiling=capacity","aic-metrics=Memory", --id test_IsAicModeIsTaskBased_LlcIsCapacity_AicMetricsIsMemory

