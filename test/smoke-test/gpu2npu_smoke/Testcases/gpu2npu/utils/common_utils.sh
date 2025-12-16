#!/bin/bash

# Requires one parameters: error information
handle_error() {
    echo "$1"
    echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
    echo "${file_name} fail: $1" | tee -a ${testcase_result_dir}/run_log.txt
    exit 1
}

verify_tool_exists() {
    if [ ! -f ${gpu2npu_path}/pytorch_analyse.sh ]; then
        handle_error "[ERROR] The pytorch_analyse.sh file is not found. "
    fi
    if [ ! -f ${gpu2npu_path}/pytorch_gpu2npu.sh ]; then
        handle_error "[ERROR] The pytorch_gpu2npu.sh file is not found. "
    fi
}

verify_transfer_success() {
    grep 'MsFmkTransplt run succeeded' ${testcase_result_dir}/plog.txt
    if [ $? -ne 0 ]; then
        handle_error "[ERROR] MsFmkTransplt run failed, please find the reason. "
    fi
}

verify_analyse_success() {
    grep 'Analyse run succeeded, welcome to the next use.' ${testcase_result_dir}/plog.txt
    if [ $? -ne 0 ]; then
        handle_error "[ERROR] Analyse run failed, please find the reason. "
    fi
}

verify_msft_exists() {
    if [ ! -d ${testcase_result_dir}/${model_name}_msft ]; then
        handle_error "[ERROR] msft not exist, please find the reason. "
    fi
}

get_torch_version() {
    version_arg_f=`pip3 list | grep torch-npu`
    if [[ "$version_arg_f" == *1.11* ]]; then
        version_arg="1.11.0"
    elif [[ "$version_arg_f" == *2.1.0* ]]; then
        version_arg="2.1.0"
    elif [[ "$version_arg_f" == *2.2.0* ]]; then
        version_arg="2.2.0"
    elif [[ "$version_arg_f" == *2.3.1* ]]; then
        version_arg="2.3.1"
    elif [[ "$version_arg_f" == *2.4.0* ]]; then
        version_arg="2.4.0"
    else
        version_arg="2.8.0"
    fi
    echo $version_arg
}

verify_gpu2npu_run_result() {
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

    cd ${testcase_result_dir}/${model_name}_msft
    python3 ${model_name}.py --print-freq 10 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    count=$(cat ${testcase_result_dir}/plog.txt | grep "Run successfully." |wc -l)
    if [ $count -gt 0 ]; then
        echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
    else
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
    fi
}

verify_transfer_to_npu_run_result() {
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi

    start_time=$(date "+%s")
    python3 ${cur_dir}/src/${file_name}.py --print-freq 10 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))
    count=$(cat ${testcase_result_dir}/plog.txt | grep "Run successfully." |wc -l)
    if [ $count -gt 0 ]; then
        echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
    else
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
    fi
}

verify_gpu2npu_analyse_result() {
    testcase_result_dir="${output_dir}/${file_name}"
    if [ ! -d ${testcase_result_dir} ]; then
        mkdir -p ${testcase_result_dir}
    else
        rm -rf ${testcase_result_dir}/*
    fi

    verify_tool_exists

    version_arg=$(get_torch_version)

    start_time=$(date "+%s")
    bash ${gpu2npu_path}/pytorch_analyse.sh -i ${cur_dir}/src/${file_name} -o ${testcase_result_dir} -v ${version_arg} -m $1 2>&1 | tee -a ${testcase_result_dir}/plog.txt >> /dev/null
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    verify_analyse_success

    if [ ! -d ${testcase_result_dir}/${file_name}_analysis ]; then
        handle_error "[ERROR] analysis result not exist. "
    fi
    if [ ${version_arg} == "1.11.0" ]; then
        template_dirname=${file_name}_analysis_1
    else
        template_dirname=${file_name}_analysis_2
    fi

    if [ ${file_name} == "test_gpu2npu_dynamic_shape" ]; then
        diff -r --exclude=pytorch_analysis.txt --exclude=__pycache__ ${testcase_result_dir}/${file_name}_analysis ${cur_dir}/src/${template_dirname} > ${testcase_result_dir}/diff_result.txt
    fi

    count1=$(find ${testcase_result_dir}/${file_name}_analysis -type f | wc -l)
    count2=$(find ${cur_dir}/src/${template_dirname} -type f | wc -l)
    if [[ "$count1" -ne "$count2" ]]; then
        handle_error "[ERROR] The number of analysis files is abnormal, bench ${count2}, result ${count1}. "
    fi

    for item in "${csv_list[@]}"; do
        # 三方库分析得到的csv文件，单元格中含有很多行字符串，是乱序的，无法diff, 校验方式为比较csv大小
        if [ ${file_name} == "test_gpu2npu_third_party" ]; then
          size1=$(wc -c < ${testcase_result_dir}/${file_name}_analysis/$item)
          size2=$(wc -c < ${cur_dir}/src/${template_dirname}/$item)
          if [[ "$size1" -ne "$size2" ]]; then
              handle_error "[ERROR] The analysis files size is incorrect, $item, bench ${size2}, result ${size1}. "
          fi
        else
            diff <(sort ${testcase_result_dir}/${file_name}_analysis/$item) <(sort ${cur_dir}/src/${template_dirname}/$item) > ${testcase_result_dir}/diff_result.txt
        fi
    done

    if [ -s ${testcase_result_dir}/diff_result.txt ]; then
        handle_error "[ERROR] The analysis files content is incorrect through diff. "
    fi
    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
}
