#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

rm -rf $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8
mkdir $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8

# ----------------配置环境变量----------------
source $CANN_PATH
WAN_REPO_DIR=${WAN_REPO_DIR:-"$PROJECT_PATH/resource/multi_modal/Wan2.1-code"}
MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"$PROJECT_PATH/resource/msit/msmodelslim"}
export PYTHONPATH=$WAN_REPO_DIR:$PYTHONPATH
# ----------------配置环境变量----------------

# 装载新的包
cd $MSMODELSLIM_SOURCE_DIR
bash install.sh

MSMODELSLIM_PATH=$(python -c "import msmodelslim; print(msmodelslim.__path__[0])")  # "
echo "msModelSlim 路径: $MSMODELSLIM_PATH"

# 示例：检查 msmodelslim/practice_lab 是否存在
if [ -d "$MSMODELSLIM_PATH/lab_practice" ]; then
    echo "$MSMODELSLIM_PATH/lab_practice 目录存在，即将置其中文件为750权限"
    chmod -R 750 $MSMODELSLIM_PATH/lab_practice
fi

if [ -d "$MSMODELSLIM_PATH/lab_calib" ]; then
    echo "$MSMODELSLIM_PATH/lab_calib 目录存在，即将置其中文件为750权限"
    chmod -R 750 $MSMODELSLIM_PATH/lab_calib
fi

msmodelslim quant \
    --model_path $PROJECT_PATH/resource/models/Wan2.1-T2V-1.3B \
    --save_path $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8 \
    --device npu \
    --model_type Wan2_1 \
    --config_path $PROJECT_PATH/test-case/multimodal_sd_modelslim_v1_wan2_1_w8a8/wan2_1_w8a8_dynamic.yaml \
    --trust_remote_code True

# 配置待检查的路径和文件列表
TARGET_PATH="$PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8"  # 修改为实际路径
FILES=(
    "quant_model_description_w8a8_dynamic.json"
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
quant_model_single="$TARGET_PATH/quant_model_weight_w8a8_dynamic.safetensors"
quant_model_pattern="$TARGET_PATH/quant_model_weight_w8a8_dynamic-*.safetensors"
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
    echo multimodal_sd_modelslim_v1_wan2_1_w8a8: Success
else
    echo multimodal_sd_modelslim_v1_wan2_1_w8a8: Failed
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8

exit $run_ok