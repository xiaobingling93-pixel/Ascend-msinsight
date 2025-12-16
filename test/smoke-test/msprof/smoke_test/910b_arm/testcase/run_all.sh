#!/bin/bash

rm result.txt

export MSPROF_VERSION=$2
parsePerformance=$3


if [ "-master" = "-$1" ];then # 不开启parsePerformance，过滤掉test_parse_performance.sh
    test_List=`ls |grep ".sh" | grep -E "test" | grep -v "Smoke" | grep -v "parse_performance"`
elif [ $parsePerformance == "no" ];then
    test_List=`ls |grep ".sh" | grep -E "Smoke" | grep -v "parse_performance"`
else
    test_List=`ls |grep ".sh" | grep -E "Smoke"`
fi

if [ ! -z "$test_List" ];then
    echo "[DEBUG] there are test_list: "$test_List
    echo "[DEBUG] start daily_test: "
    for i in $test_List
    do
        echo "start $i testcase:===================="
        bash $i
        echo "end $i testcase:======================"
    done
    echo "[DEBUG] end daily_test: "
else
    echo "[DEBUG] there none test case: "$test_List

fi

if [ "-master" = "-$1" ];then
    cd /home/msprof_smoke_test/smoke_test/910b_arm/testcase/pta_smoke
    bash run_all.sh master
fi

if [ "-master" = "-$1" ];then
    cd /home/msprof_smoke_test/smoke_test/910b_arm/testcase/ms_smoke
    bash run_all.sh
fi

if [ "-master" = "-$1" ];then
    result_file=/home/msprof_smoke_test/smoke_test/910b_arm/testcase/result.txt
    echo "[DEBUG] start executing msprof-analyze test cases."
    # 初始化mstt环境
    source /root/miniconda3/bin/activate smoke_test_env_bak
    export MSTT_PROFILER_ST_DATA_PATH=/home/msprof_smoke_test/msprof_analyze_data
    echo "[DEBUG] msprof-analyze environment initialization completed."
    # 下载mstt代码
    cd /home/msprof_smoke_test
    rm -rf mstt
    git clone https://gitcode.com/Ascend/mstt
    cd mstt/profiler/msprof_analyze
    echo "[DEBUG] msprof-analyze code download completed."
    # 编包安装
    pip uninstall -y msprof-analyze
    python setup.py bdist_wheel
    pip install dist/*.whl --force-reinstall
    echo "[DEBUG] msprof-analyze package installation completed."
    # 2.执行st用例，用例所需数据不全，必挂，且st每个pr都会跑意义不大，先不跑了
    # python test/run_st.py
    # exit_status=$?
    # if [ $exit_status -eq 0 ]; then
    #   echo "test_msprof_analyze_st_test_case pass" >> $result_file
    # else
    #   echo "test_msprof_analyze_st_test_case fail" >> $result_file
    # fi
    # echo "[DEBUG] msprof-analyze test case execution completed."
fi
