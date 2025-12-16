#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

# 引入公共模块
source $PROJECT_PATH/scripts/common_utils.sh

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

# 调用公共模块设置权限
set_directory_permissions

MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"$PROJECT_PATH/resource/msit/msmodelslim"}

pip install transformers==4.51.0 -i http://mirrors.tools.huawei.com/pypi/simple/ --trusted-host mirrors.tools.huawei.com

msmodelslim analyze \
    --model_type Qwen3-14B \
    --model_path $PROJECT_PATH/resource/llm_ptq/Qwen3-14B

if [ $? -eq 0 ]
then
    echo analyze_qwen3_14b: Success
else
    echo analyze_qwen3_14b: Failed
    run_ok=$ret_failed
fi

conda activate main_env
exit $run_ok