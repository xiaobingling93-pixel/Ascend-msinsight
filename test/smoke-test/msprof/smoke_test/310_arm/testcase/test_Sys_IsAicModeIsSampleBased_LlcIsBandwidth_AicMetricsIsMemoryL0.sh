#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s Sys -p "aic-mode=sample-based","llc-profiling=bandwidth","aic-metrics=MemoryL0", --id test_Sys_IsAicModeIsSampleBased_LlcIsBandwidth_AicMetricsIsMemoryL0
