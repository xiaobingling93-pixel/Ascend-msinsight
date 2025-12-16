#!/bin/bash

# Define Color Codes
RED='\e[0;31m'
GREEN='\e[0;32m'
WHITE='\e[0;37m'
NC='\e[0m'

source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_ms
result_file=/home/msprof_smoke_test/smoke_test/910b_arm/testcase/result.txt
output_path=/home/result_dir/ms_smoke_case

rm -rf $output_path

python src/mindspore_installer.py

test_list=`ls |grep ".sh" | grep -E "test_ascend_mindspore"`
num_of_cases=$(echo "$test_list" | wc -l)

if [ ! -z "$test_list" ]; then
    echo -e "${WHITE}========================================${NC}"
    echo -e "${GREEN}[DEBUG] There are $num_of_cases test cases:${NC}"
    echo -e "${WHITE}----------------------------------------${NC}"
    echo -e "$test_list" | sed 's/^/    /'
    echo -e "${WHITE}========================================${NC}"

    for i in $test_list
    do
        echo -e "${WHITE}====================${i%.sh} Test Case ====================${NC}"
        bash $i $output_path
        if [ 0 -ne $? ]; then
            echo "$i fail" >> $result_file
            echo -e "--------------------------------------${WHITE}${i%.sh}${NC}------${RED}FAIL${NC}"
        else
            echo "$i pass" >> $result_file
            echo -e "--------------------------------------${WHITE}${i%.sh}${NC}------${GREEN}PASS${NC}"
        fi
    done
    echo -e "${GREEN}[DEBUG] End ms_smoke_test${NC}"
else
    echo -e "${RED}[DEBUG] No test cases: $test_list${NC}"
fi
