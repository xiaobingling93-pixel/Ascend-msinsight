#!/bin/bash
##################### 所有脚本一样 ###################
cur_dir=$(dirname $(readlink -f $0))
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta
export ASCEND_GLOBAL_LOG_LEVEL=1
export MSMONITOR_USE_DAEMON=1

output_dir=$(realpath $1)
pytorch_branchs=$2

pkill -9 python
pkill -9 dynolog

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
    # 1. 拉起dynolog后台
    cd /home/msprof_smoke_test/mstt/msmonitor/third_party/dynolog/build/bin/
    if [ ! -f ./dynolog ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}, Error: dynolog file does not exist"
        exit 1
    fi
    ./dynolog --enable-ipc-monitor --certs-dir NO_CERTS --use_JSON > ${testcase_result_dir}/dynolog.txt 2>&1 &
    dynolog_pid=$!
    # 2. 拉起训练
    cd ${cur_dir}/src/
    rm -rf /dev/shm/DynamicProfileNpuShm*
    python ${cur_dir}/src/dynamic_model_train.py > ${testcase_result_dir}/plog.txt 2>&1 &
    train_pid=$!
    sleep 15
    # 3. 发送dyno消息
    cd /home/msprof_smoke_test/mstt/msmonitor/third_party/dynolog/build/bin/
    if [ ! -f ./dynolog ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}, Error: dyno file does not exist"
        exit 1
    fi
    ./dyno --certs-dir NO_CERTS nputrace --iterations 2 --start-step 10 --record-shapes --profile-memory --with-modules --analyse --l2-cache --data-simplification false --activities CPU,NPU --profiler-level Level2 --aic-metrics ArithmeticUtilization --export-type Text --host-sys cpu,mem --sys-io --sys-interconnection --log-file ${testcase_result_dir}/npu_trace > ${testcase_result_dir}/dyno.txt 2>&1 &
    if [ 0 -ne $? ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi
    cd ${cur_dir}/src/
    timeout 600 tail -f ${testcase_result_dir}/plog.txt | while read line; do
        if [[ "$line" == *"model train over..."* ]]; then
            (kill -9 "$train_pid" 2>&1) >> "${testcase_result_dir}/plog.txt" 2>&1
            (kill -9 "$dynolog_pid" 2>&1) >> "${testcase_result_dir}/plog.txt" 2>&1
            break
        fi
    done
    # 检查timeout的退出状态
    timeout_exit_code=$?
    if [ $timeout_exit_code -eq 124 ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        (kill -9 "$train_pid" 2>&1) >> "${testcase_result_dir}/plog.txt" 2>&1
        (kill -9 "$dynolog_pid" 2>&1) >> "${testcase_result_dir}/plog.txt" 2>&1
        exit 1
    fi
    # 4. 校验结果
    python ${cur_dir}/src/${file_name}.py ${testcase_result_dir}/npu_trace
    if [ 0 -ne $? ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi
    end_time=$(date "+%s")
    duration_time=$(( ${end_time} - ${start_time} ))

    # 校验plog有无报错
    grep "ERROR" ${testcase_result_dir}/plog.txt | grep "PROFILING"
    if [ 0 -eq $? ]; then
        echo "${file_name} fail ${duration_time} ${pytorch_branchs}"
        exit 1
    fi

    echo "${file_name} pass ${duration_time} ${pytorch_branchs}"
}

main