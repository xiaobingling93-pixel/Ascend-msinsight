#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

# 引入公共模块
source $PROJECT_PATH/scripts/common_utils.sh

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

# 用例名（用于打屏和指定输出路径）
CASE_NAME=modelslim_v1_qwen3_dense_w8a8_flexsmooth

rm -rf $PROJECT_PATH/output/$CASE_NAME

# 调用公共模块设置权限
set_directory_permissions

MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"$PROJECT_PATH/resource/msit/msmodelslim"}

pip install transformers==4.51.0 -i http://mirrors.tools.huawei.com/pypi/simple/ --trusted-host mirrors.tools.huawei.com

msmodelslim quant \
    --model_path $PROJECT_PATH/resource/llm_ptq/Qwen3-14B \
    --save_path $PROJECT_PATH/output/$CASE_NAME \
    --device npu \
    --model_type Qwen3-14B \
    --config_path $PROJECT_PATH/test-case/$CASE_NAME/dense-w8a8-flexsmooth-v1.yaml \
    --trust_remote_code True

# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/$CASE_NAME"  # 修改为实际路径
FILES=(
    "config.json"
    "generation_config.json"
    "quant_model_description.json"
    "tokenizer_config.json"
    "tokenizer.json"
    # 可在此处添加更多文件，每行一个
)

if check_files_exist "$TARGET_PATH" "${FILES[@]}"; then
    echo "$CASE_NAME: Success"
else
    echo "$CASE_NAME: Failed"
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/$CASE_NAME

conda activate main_env
exit $run_ok