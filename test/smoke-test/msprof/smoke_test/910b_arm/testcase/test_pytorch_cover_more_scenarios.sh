#!/bin/bash
output_path="/home/result_dir/test_pytorch_cover_more_scenarios"
rm -rf $output_path
mkdir $output_path
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_pytorch_cover_more_scenarios.py
