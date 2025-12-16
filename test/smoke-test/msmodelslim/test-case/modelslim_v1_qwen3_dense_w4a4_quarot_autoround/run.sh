#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

# 引入公共模块
source $PROJECT_PATH/scripts/common_utils.sh

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

rm -rf $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w4a4_quarot_autoround

# 调用公共模块设置权限
set_directory_permissions

pip install transformers==4.51.0 -i http://mirrors.tools.huawei.com/pypi/simple/ --trusted-host mirrors.tools.huawei.com

msmodelslim quant \
    --model_path $PROJECT_PATH/resource/llm_ptq/Qwen3-14B \
    --save_path $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w4a4_quarot_autoround \
    --device npu \
    --model_type Qwen3-14B \
    --config_path $PROJECT_PATH/test-case/modelslim_v1_qwen3_dense_w4a4_quarot_autoround/dense-w4a4-v1.yaml \
    --trust_remote_code True

# 调用公共模块校验文件
# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/modelslim_v1_qwen3_dense_w4a4_quarot_autoround"
FILES=(
    "config.json"
    "generation_config.json"
    "quant_model_description.json"
    "tokenizer_config.json"
    "tokenizer.json"
    # 可在此处添加更多文件，每行一个
)

if check_files_exist "$TARGET_PATH" "${FILES[@]}"; then
    echo "modelslim_v1_qwen3_dense_w4a4_quarot_autoround: Success"
else
    echo "modelslim_v1_qwen3_dense_w4a4_quarot_autoround: Failed"
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w4a4_quarot_autoround

conda activate main_env
exit $run_ok