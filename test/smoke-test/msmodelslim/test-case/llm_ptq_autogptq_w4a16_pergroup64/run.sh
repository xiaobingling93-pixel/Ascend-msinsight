#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

rm -rf $PROJECT_PATH/output/llm_ptq_autogptq_w4a16_pergroup64
python run.py

if [ $? -eq 0 ]
then
    echo llm_ptq_autogptq_w4a16_pergroup64: Success
else
    echo llm_ptq_autogptq_w4a16_pergroup64: Failed
    run_ok=$ret_failed
fi

conda activate main_env
# 清理output
rm -rf $PROJECT_PATH/output/llm_ptq_autogptq_w4a16_pergroup64

exit $run_ok
