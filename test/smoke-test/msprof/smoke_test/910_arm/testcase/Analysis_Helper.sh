#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_analysis_scene -s Helper -p , --id Analysis_Helper_data
