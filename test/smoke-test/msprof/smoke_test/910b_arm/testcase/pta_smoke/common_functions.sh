#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export ASCEND_SLOG_PRINT_TO_STDOUT=1
export ASCEND_GLOBAL_LOG_LEVEL=1

output_dir=$(realpath $1)
pytorch_branchs=$2

main(){
    file_name=$(basename $0 .sh)
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi
    start_time=$(date "+%s")
    cd ${cur_dir}/src/
    python3 ${cur_dir}/src/${file_name}.py ${testcase_result_dir}/result_dir > ${testcase_result_dir}/plog.txt 2>&1
    python_exit_code=$?
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))
    if [ 0 -ne $python_exit_code ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi

    # 校验plog有无报错
    grep "ERROR" ${testcase_result_dir}/plog.txt | grep "PROFILING"
    if [ 0 -eq $? ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi

    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
}

main

