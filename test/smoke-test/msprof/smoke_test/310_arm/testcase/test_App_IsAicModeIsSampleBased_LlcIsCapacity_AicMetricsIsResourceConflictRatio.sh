#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s App -p "aic-mode=sample-based","llc-profiling=capacity","aic-metrics=ResourceConflictRatio", --id test_App_IsAicModeIsSampleBased_LlcIsCapacity_AicMetricsIsResourceConflictRatio
