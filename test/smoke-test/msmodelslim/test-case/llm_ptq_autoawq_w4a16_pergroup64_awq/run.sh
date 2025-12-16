#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

rm -rf $PROJECT_PATH/output/llm_ptq_autoawq_w4a16_pergroup64_awq
python run.py

if [ $? -eq 0 ]
then
    echo llm_ptq_autoawq_w4a16_pergroup64_awq: Success
else
    echo llm_ptq_autoawq_w4a16_pergroup64_awq: Failed
    run_ok=$ret_failed
fi

conda activate main_env
# 清理output
rm -rf $PROJECT_PATH/output/llm_ptq_autoawq_w4a16_pergroup64_awq

exit $run_ok
