#!/bin/bash
# 校验capture场景，无stepId，通过profiler对象采集，采集范围包含begin end
output_path="/home/result_dir/test_torch_capture_without_schedule_by_object_profiler"
rm -rf $output_path
mkdir $output_path
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_torch_capture_without_shedule_by_object_profiler.py begin_end