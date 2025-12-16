#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

rm -rf $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w8a8_kvsmooth

MSMODELSLIM_PATH=$(python -c "import msmodelslim; print(msmodelslim.__path__[0])")
# "
echo "msModelSlim 路径: $MSMODELSLIM_PATH"

if [ -d "$MSMODELSLIM_PATH/lab_practice" ]; then
    echo "$MSMODELSLIM_PATH/lab_practice 目录存在，即将置其中文件为750权限"
    # chmod 750 $MSMODELSLIM_PATH/lab_practice/*
    chmod -R 750 $MSMODELSLIM_PATH/lab_practice
fi

if [ -d "$MSMODELSLIM_PATH/lab_calib" ]; then
    echo "$MSMODELSLIM_PATH/lab_calib 目录存在，即将置其中文件为750权限"
    # chmod 750 $MSMODELSLIM_PATH/lab_calib/*
    chmod -R 750 $MSMODELSLIM_PATH/lab_calib
fi

chmod 640 $MSMODELSLIM_PATH/config/config.ini

MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"$PROJECT_PATH/resource/msit/msmodelslim"}

pip install transformers==4.51.0 -i http://mirrors.tools.huawei.com/pypi/simple/ --trusted-host mirrors.tools.huawei.com

msmodelslim quant \
    --model_path $PROJECT_PATH/resource/llm_ptq/Qwen3-14B \
    --save_path $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w8a8_kvsmooth \
    --device npu \
    --model_type Qwen3-14B \
    --config_path $PROJECT_PATH/test-case/modelslim_v1_qwen3_dense_w8a8_kvsmooth/dense-w8a8-kvsmooth-v1.yaml \
    --trust_remote_code True

# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/modelslim_v1_qwen3_dense_w8a8_kvsmooth"  # 修改为实际路径
FILES=(
    "config.json"
    "generation_config.json"
    "quant_model_description.json"
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

# 检查 quant_model_weights.safetensors 或其分片文件
quant_model_single="$TARGET_PATH/quant_model_weights.safetensors"
quant_model_pattern="$TARGET_PATH/quant_model_weights-*.safetensors"
quant_model_files=($quant_model_pattern)

if [ ! -e "$quant_model_single" ] && [ ${#quant_model_files[@]} -eq 0 ]; then
    echo "错误：quant_model_weights.safetensors 或其分片文件不存在" >&2
    all_exist=false
else
    if [ ! -e "$quant_model_single" ]; then
        for quant_file in "${quant_model_files[@]}"; do
            if [ ! -e "$quant_file" ]; then
                echo "错误：文件 '$quant_file' 不存在" >&2
                all_exist=false
            fi
        done
    fi
fi

# 输出最终结果
if [ "$all_exist" = true ]; then
    echo modelslim_v1_qwen3_dense_w8a8_kvsmooth: Success
else
    echo modelslim_v1_qwen3_dense_w8a8_kvsmooth: Failed
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/modelslim_v1_qwen3_dense_w8a8_kvsmooth

conda activate main_env
exit $run_ok