#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_analysis_scene -s ffts_on_hccl_l0 -p , --id test_Analysis_ffts_on_hccl_l0_data
