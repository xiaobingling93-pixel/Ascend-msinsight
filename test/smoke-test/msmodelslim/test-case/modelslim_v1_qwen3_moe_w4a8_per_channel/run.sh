#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

# 引入公共模块
source $PROJECT_PATH/scripts/common_utils.sh

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env  # 可以新增conda环境，但需告知冒烟负责人，以免遗漏维护

# 用例名（用于打屏和指定输出路径）
CASE_NAME=modelslim_v1_qwen3_moe_w4a8_per_channel

rm -rf $PROJECT_PATH/output/$CASE_NAME

# 调用公共模块设置权限
set_directory_permissions

# 安装所需依赖，也可以 pip install -r requirements.txt 的形式
pip install transformers==4.51.0 -i http://mirrors.tools.huawei.com/pypi/simple/ --trusted-host mirrors.tools.huawei.com

msmodelslim quant \
    --model_path $PROJECT_PATH/resource/models/Qwen3-30B-A3B \
    --save_path $PROJECT_PATH/output/$CASE_NAME \
    --device npu \
    --model_type Qwen3-30B \
    --config_path $PROJECT_PATH/test-case/$CASE_NAME/moe-w4a8-v1.yaml \
    --trust_remote_code True

# 调用公共模块校验文件
# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/$CASE_NAME"
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

# 恢复主环境
conda activate main_env
exit $run_ok