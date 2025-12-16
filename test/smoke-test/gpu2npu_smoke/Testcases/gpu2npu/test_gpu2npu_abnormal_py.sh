#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /home/l30044004/Ascend/ascend-toolkit/set_env.sh
gpu2npu_path=/home/l30044004/smoke_gpu2npu/ms_fmk_transplt
model_name=test_gpu2npu_abnormal_py

output_dir=$1
pytorch_branchs=$2
source ${cur_dir}/utils/common_utils.sh
main(){
    file_name=$(basename $0 .sh)
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi
    verify_tool_exists

    version_arg=$(get_torch_version)

    start_time=$(date "+%s")
    bash ${gpu2npu_path}/pytorch_gpu2npu.sh -i ${cur_dir}/src/${model_name} -o ${testcase_result_dir} -v ${version_arg} $1 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    verify_transfer_success

    verify_msft_exists

    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"

}

main