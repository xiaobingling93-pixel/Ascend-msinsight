#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)

python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s Sys -p "aic-mode=sample-based","llc-profiling=read","aic-metrics=MemoryL0", --id test_Sys_IsAicModeIsSampleBased_LlcIsRead_AicMetricsIsMemoryL0
