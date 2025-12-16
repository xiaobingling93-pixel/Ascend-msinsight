#!/bin/bash
output_path="/home/result_dir/test_pytorch_single_process_multiple_devices"
rm -rf $output_path
mkdir $output_path
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_pytorch_single_process_multiple_devices.py
