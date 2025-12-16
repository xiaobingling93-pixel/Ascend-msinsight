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

# if [ "-master" = "-$1" ];then
#     cd /home/msprof_smoke_test/smoke_test/910_arm/testcase/pta_smoke
#     bash run_all.sh
# fi

# if [ "-master" = "-$1" ];then
#     cd /home/msprof_smoke_test/smoke_test/910_arm/testcase/ms_smoke
#     bash run_all.sh
# fi
