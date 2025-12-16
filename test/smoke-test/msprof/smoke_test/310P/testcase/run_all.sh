#!/bin/bash

source /usr/local/Ascend/tfplugin/set_env.sh; source /usr/local/Ascend/ascend-toolkit/set_env.sh; cd /home/msprof_smoke_test/model/RunDevId0/out/; ./main; cd -

rm result.txt

export MSPROF_VERSION=$2
if [ "-master" = "-$1" ];then
    test_List=`ls |grep ".sh" | grep -E "test" | grep -v "Smoke"`
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

