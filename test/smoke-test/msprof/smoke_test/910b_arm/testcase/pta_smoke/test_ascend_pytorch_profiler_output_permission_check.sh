#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export ASCEND_SLOG_PRINT_TO_STDOUT=1
export ASCEND_GLOBAL_LOG_LEVEL=1

output_dir=$(realpath $1)
pytorch_branchs=$2

# 文件权限640，目录权限750
function traverse_and_check() {
    for file in $(ls $1)
    do
        if [ -d "$1/$file" ]; then
            if [[ $file != "." && $file != ".." && $file != ".git" ]]; then
                # Check目录是否为750
                permission_value=`stat -c %a $1"/"$file`
                if [[ ${permission_value} > 750 ]]; then
                    echo "[ERROR] Failed to check dir:$1/$file permission:${permission_value}."
                    echo "${2} fail ${3} ${4}"
                    exit 1
                fi
                traverse_and_check "$1/$file" $2 $3 $4
            fi
        else
            permission_value=`stat -c %a $1"/"$file`
            if [[ ${permission_value} > 640 ]]; then
                echo "[ERROR] Failed to check file:$1/$file permission:${permission_value}."
                echo "${2} fail ${3} ${4}"
                exit 1
            fi
        fi
    done
}

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
    umask_value=$(umask)
    umask 0077
    python3 ${cur_dir}/src/test_ascend_pytorch_profiler_all_params.py ${testcase_result_dir}/result_dir > ${testcase_result_dir}/plog.txt 2>&1
    if [ 0 -ne $? ]; then
        echo "${file_name} fail 0 ${pytorch_branchs}"
        exit 1
    fi
    umask ${umask_value}
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    # 校验plog有无报错
    grep "ERROR" ${testcase_result_dir}/plog.txt | grep "PROFILING"
    if [ 0 -eq $? ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi
    if [ -d ${testcase_result_dir}/result_dir ]; then
        traverse_and_check ${testcase_result_dir}/result_dir ${file_name} ${duration_time} ${pytorch_branchs}
    fi
    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
}

main
