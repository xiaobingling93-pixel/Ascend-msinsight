#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s All -p "aic-mode=sample-based","llc-profiling=capacity","aic-metrics=MemoryUB", --id test_IsAicModeIsSampleBased_LlcIsCapacity_AicMetricsIsMemoryUB

