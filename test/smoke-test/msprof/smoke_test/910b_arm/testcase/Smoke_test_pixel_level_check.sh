#!/bin/bash
output_path="/home/result_dir/pixel_level_check"
rm -rf $output_path
mkdir $output_path
mkdir $output_path/test1
mkdir $output_path/test2
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../../pixel_level_check/pixel_level_check.py
