#!/bin/bash

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
	echo "use tr5 branch: "${MSPROF_VERSION}
        bash $i
        echo "end $i testcase:======================"
    done
    echo "[DEBUG] end daily_test: "
else
    echo "[DEBUG] there none test case: "$test_List

fi
