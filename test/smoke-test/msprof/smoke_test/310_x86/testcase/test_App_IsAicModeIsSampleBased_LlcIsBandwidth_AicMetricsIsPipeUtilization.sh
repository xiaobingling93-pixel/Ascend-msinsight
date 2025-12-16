#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_all_switch -s App -p "aic-mode=sample-based","llc-profiling=bandwidth","aic-metrics=PipeUtilization", --id test_App_IsAicModeIsSampleBased_LlcIsBandwidth_AicMetricsIsPipeUtilization

