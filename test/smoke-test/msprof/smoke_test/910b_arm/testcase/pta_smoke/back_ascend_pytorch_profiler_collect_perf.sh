#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta
export ASCEND_SLOG_PRINT_TO_STDOUT=1
export ASCEND_GLOBAL_LOG_LEVEL=1

output_dir=$(realpath $1)
pytorch_branchs=$2

main() {
    file_name=$(basename $0 .sh)
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi
    start_time=$(date "+%s")
    cd ${cur_dir}/src/Bert_ascend_pytorch_profiler_collect_perf
    export ASCEND_WORK_PATH="${testcase_result_dir}/result_dir"
    python3 ${cur_dir}/src/Bert_ascend_pytorch_profiler_collect_perf/train.py > ${testcase_result_dir}/plog.txt 2>&1
    if [ 0 -ne $? ]; then
        echo "${file_name} fail 0 ${pytorch_branchs}"
        exit 1
    fi
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    ratio=`grep -rw "Ration" ${testcase_result_dir}/plog.txt | awk -F ":" '{print $2}'`
    compare_flag=$(echo "scale=2; ${ratio} < 8.0" | bc)
    if [[ ${compare_flag} -ne 1 ]]; then
        echo "${ratio}"
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi
    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
}


main
