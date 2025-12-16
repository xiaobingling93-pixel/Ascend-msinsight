#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

rm -rf $PROJECT_PATH/output/app_naive_quantization_qwen2.5_7b_w8a8

MSMODELSLIM_PATH=$(python -c "import msmodelslim; print(msmodelslim.__path__[0])")
# "
echo "msModelSlim 路径: $MSMODELSLIM_PATH"


msmodelslim quant \
  --model_path $PROJECT_PATH/resource/llm_ptq/Qwen2.5-7B-Instruct_all \
  --save_path $PROJECT_PATH/output/app_naive_quantization_qwen2.5_7b_w8a8 \
  --device npu \
  --model_type \
  Qwen2.5-7B-Instruct \
  --quant_type w8a8 \
  --trust_remote_code True

# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/app_naive_quantization_qwen2.5_7b_w8a8"  # 修改为实际路径
FILES=(
    "config.json"
    "generation_config.json"
    "quant_model_description.json"
    "quant_model_weight_w8a8.safetensors"
    "tokenizer_config.json"
    "tokenizer.json"
    # 可在此处添加更多文件，每行一个
)

# 检查路径是否存在
if [ ! -d "$TARGET_PATH" ]; then
    echo "错误：目标路径 '$TARGET_PATH' 不存在" >&2
    exit 1
fi

# 初始化结果标志
all_exist=true

# 遍历检查每个文件
for file in "${FILES[@]}"; do
    full_path="$TARGET_PATH/$file"
    if [ ! -e "$full_path" ]; then
        echo "错误：文件 '$file' 不存在" >&2
        all_exist=false
    fi
done

# 输出最终结果
if [ "$all_exist" = true ]; then
    echo app_naive_quantization_qwen2.5_7b_w8a8: Success
    exit 0
else
    echo app_naive_quantization_qwen2.5_7b_w8a8: Failed
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/app_naive_quantization_qwen2.5_7b_w8a8

conda activate main_env
exit $run_ok