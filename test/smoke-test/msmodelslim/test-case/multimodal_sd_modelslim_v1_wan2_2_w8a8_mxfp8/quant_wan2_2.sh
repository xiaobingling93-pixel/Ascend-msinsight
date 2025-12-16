#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

rm -rf $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8
mkdir $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8

# ----------------配置环境变量----------------
source $CANN_PATH
WAN_REPO_DIR=${WAN_REPO_DIR:-"$PROJECT_PATH/resource/multi_modal/Wan2.2-code"}
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
    --model_path $PROJECT_PATH/resource/models/Wan2.2-I2V-A14B \
    --save_path $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8 \
    --device npu \
    --model_type Wan2_1 \
    --config_path $PROJECT_PATH/test-case/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8/wan2_2_w8a8_mxfp8_i2v.yaml \
    --trust_remote_code True

# 配置待检查的路径和文件列表
# 配置待检查的目标路径（1个或2个），路径间用空格分隔
TARGET_PATHS=(
    "$PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8/i2v_quant_weights_anti_high"
    "$PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8/i2v_quant_weights_anti_low"
    # 可选：添加第二个路径，取消注释即可
)

# 配置每个路径下需要检查的基础文件列表
FILES=(
    "quant_model_description_w8a8_mxfp8.json"
    # 可在此处添加更多基础文件，每行一个
)

# 量化权重文件的基础名称（不含分片后缀）
QUANT_WEIGHT_BASE="quant_model_weight_w8a8_mxfp8.safetensors"
# ==========================================================

# 初始化全局结果标志（true=所有文件都存在，false=至少一个缺失）
all_exist=true

# 遍历每个目标路径进行检查
for target_path in "${TARGET_PATHS[@]}"; do
    echo "===== 检查路径: $target_path ====="
    
    # 1. 检查当前路径是否存在
    if [ ! -d "$target_path" ]; then
        echo "错误：目标路径 '$target_path' 不存在" >&2
        all_exist=false
        # 路径不存在则跳过该路径下的文件检查
        continue
    fi

    # 2. 检查基础文件列表中的每个文件
    for file in "${FILES[@]}"; do
        full_path="$target_path/$file"
        if [ ! -e "$full_path" ]; then
            echo "错误：路径 '$target_path' 下的文件 '$file' 不存在" >&2
            all_exist=false
        fi
    done

    # 3. 检查量化权重文件（单文件 或 分片文件）
    quant_single="$target_path/$QUANT_WEIGHT_BASE"
    # 匹配分片文件（通配符 *.safetensors.00001-of-00002 等格式）
    quant_shards=("$target_path/${QUANT_WEIGHT_BASE}.*")

    # 先检查是否存在单文件
    if [ -e "$quant_single" ]; then
        echo "成功：路径 '$target_path' 下找到完整权重文件: $QUANT_WEIGHT_BASE"
    else
        # 单文件不存在，检查分片文件是否存在且有效
        # 过滤掉通配符匹配不到文件时的空值
        valid_shards=()
        for shard in "${quant_shards[@]}"; do
            if [ -e "$shard" ]; then
                valid_shards+=("$shard")
            fi
        done

        if [ ${#valid_shards[@]} -eq 0 ]; then
            echo "错误：路径 '$target_path' 下未找到 $QUANT_WEIGHT_BASE（单文件）或其分片文件" >&2
            all_exist=false
        else
            echo "成功：路径 '$target_path' 下找到分片权重文件: ${valid_shards[*]}"
        fi
    fi
    echo "----------------------------------------"
done

# 输出最终结果
if [ "$all_exist" = true ]; then
    echo multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8: Success
else
    echo multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8: Failed
    run_ok=$ret_failed
fi

# 清理output
rm -rf $PROJECT_PATH/output/multimodal_sd_modelslim_v1_wan2_1_w8a8

exit $run_ok