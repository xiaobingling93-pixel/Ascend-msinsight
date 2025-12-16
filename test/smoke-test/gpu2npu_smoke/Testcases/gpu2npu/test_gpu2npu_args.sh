#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /home/l30044004/Ascend/ascend-toolkit/set_env.sh
gpu2npu_path=/home/l30044004/smoke_gpu2npu/ms_fmk_transplt
model_name=test_gpu2npu_args

output_dir=$1
pytorch_branchs=$2
source ${cur_dir}/utils/common_utils.sh
file_name=$(basename $0 .sh)
main(){
    verify_gpu2npu_run_result -s
}

main