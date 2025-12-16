#!/bin/bash
output_path="/home/result_dir/test_msprof_task_based_memory_access"
rm -rf $output_path
mkdir $output_path
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_msprof_task_based_memory_access.py