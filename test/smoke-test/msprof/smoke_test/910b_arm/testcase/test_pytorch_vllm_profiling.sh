#!/bin/bash
output_path="/home/result_dir/test_pytorch_vllm_profiling"
rm -rf $output_path
mkdir $output_path
currentDir=$(cd "$(dirname "$0")"; pwd)
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /root/miniconda3/bin/activate smoke_test_env_vllm
source /usr/local/Ascend/nnal/atb/set_env.sh
python3 ${currentDir}/../script/test_pytorch_vllm_profiling.py
