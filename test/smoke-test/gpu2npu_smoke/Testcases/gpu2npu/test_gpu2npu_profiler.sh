#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /home/l30044004/Ascend/ascend-toolkit/set_env.sh

output_dir=$1
pytorch_branchs=$2

main(){
    file_name=$(basename $0 .sh)
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi

    gpu2npu_path=/home/l30044004/smoke_gpu2npu/ms_fmk_transplt

    # 校验工具源文件是否存在
    if [ ! -f  ${gpu2npu_path}/pytorch_analyse.sh ]; then
        echo "[ERROR] The pytorch_analyse.sh file is not found. " >> ${testcase_result_dir}/run_log.txt
        echo "${file_name} fail 0 ${pytorch_branchs}"
        exit 1
    fi
    model_name=test_gpu2npu_profiler

    version_arg_f=`pip3 list | grep torch-npu`
    echo $version_arg_f
    if [[ "$version_arg_f" == *1.8* ]]; then
        version_arg="1.8.1"
    elif [[ "$version_arg_f" == *1.11* ]]; then
        version_arg="1.11.0"
    elif [[ "$version_arg_f" == *2.0.1* ]]; then
        version_arg="2.0.1"
    else
        version_arg="2.1.0"
    fi

    start_time=$(date "+%s")
    bash ${gpu2npu_path}/pytorch_gpu2npu.sh -i ${cur_dir}/src/${model_name} -o ${testcase_result_dir} -v ${version_arg} 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    grep 'MsFmkTransplt run succeeded'  ${testcase_result_dir}/plog.txt
    if [ $? -ne 0 ]; then
        echo "[ERROR] MsFmkTransplt run failed. " >> ${testcase_result_dir}/run_log.txt
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi
    if [ ! -d ${testcase_result_dir}/${model_name}_msft ]; then
        echo "[ERROR] msft not exist. " >> ${testcase_result_dir}/run_log.txt
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi

    cd ${testcase_result_dir}/${model_name}_msft
    python3 test_gpu2npu_profiler.py --print-freq 10 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))
    count=$(cat ${testcase_result_dir}/plog.txt | grep Epoch |wc -l)
    if [ $count -gt 0 ]; then
        echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
    else
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
    fi
}

main
