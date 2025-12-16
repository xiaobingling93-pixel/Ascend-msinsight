#!/bin/bash

pytorch_branchs=$1
result_log=$2
output_dir=$3
testcases_dir=$(dirname $(readlink -f $0))
output_dir="$output_dir/gpu2npu"

main() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Begin Run Gpu2npu TestCase ----------------------------"
    for casename in $(ls ${testcases_dir}/gpu2npu); do
        if [[ ${casename} =~ \.sh$ ]]; then
            echo "[Info][$(date "+%F %T")] ---------------------------- Execution ${casename} ----------------------------"
            bash ${testcases_dir}/gpu2npu/${casename} ${output_dir} ${pytorch_branchs} >> ${result_log}
        fi
    done
    echo "[Info][$(date "+%F %T")] ---------------------------- Testcases execution completed ----------------------------"
}

main
