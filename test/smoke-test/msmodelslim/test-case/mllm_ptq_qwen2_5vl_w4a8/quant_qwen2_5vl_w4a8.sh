#!/usr/bin/env bash

# ----------------配置环境变量----------------
MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"~/resource/msit/msmodelslim"}
# ----------------配置环境变量----------------

cd $MSMODELSLIM_SOURCE_DIR/example/multimodal_vlm/Qwen2.5-VL
# 执行量化
python quant_qwen2_5vl.py \
    --model_path $PROJECT_PATH/resource/mllm/Qwen2.5-VL-7B-Instruct \
    --calib_images $PROJECT_PATH/resource/mllm/coco_pic \
    --save_directory $PROJECT_PATH/output/mllm_ptq_qwen2_5vl_w4a8 \
    --w_bit 4 \
    --a_bit 8 \
    --act_method 1 \
    --device_type npu \
    --trust_remote_code True \
    --anti_method m4 \
    --open_outlier False \
    --is_dynamic True \
    --is_lowbit True \
    --group_size 256
