#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /home/l30044004/Ascend/ascend-toolkit/set_env.sh
gpu2npu_path=/home/l30044004/smoke_gpu2npu/ms_fmk_transplt
file_name=$(basename $0 .sh)

output_dir=$1
pytorch_branchs=$2
source ${cur_dir}/utils/common_utils.sh
main(){
    csv_list=("api_performance_advice.csv" "api_precision_advice.csv" "cuda_op_list.csv" "unknown_api.csv" "unsupported_api.csv")
    verify_gpu2npu_analyse_result torch_apis
}

main