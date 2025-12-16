#!/usr/bin/env bash

# 清理output路径
rm -rf $PROJECT_PATH/output/multi_modal_quant_fa3_flux/*

# ----------------配置环境变量----------------
source $CANN_PATH
MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"~/test/Smoke_Test/smoke_auto_sh/msit/msmodelslim"}
FLUX_REPO_DIR=${FLUX_REPO_DIR:-"$PROJECT_PATH/resource/multi_modal/FLUX.1-dev-code"}
MODEL_PATH=$PROJECT_PATH/resource/multi_modal/FLUX.1-dev
# ----------------配置环境变量----------------

export CPU_AFFINITY_CONF=2
export TASK_QUEUE_ENABLE=2
# add msmodelslim package path to python path
export PYTHONPATH=${MSMODELSLIM_SOURCE_DIR}:$PYTHONPATH

cd $FLUX_REPO_DIR
# 单卡32G Flux 等价优化推理
python $MSMODELSLIM_SOURCE_DIR/example/multimodal_sd/Flux/inference_flux.py \
    --path ${MODEL_PATH} \
    --save_path "$PROJECT_PATH/output/multi_modal_quant_fa3_flux/results/quant/img" \
    --device_id 0 \
    --device "npu" \
    --prompt_path "$MSMODELSLIM_SOURCE_DIR/example/multimodal_sd/Flux/calib_prompts.txt" \
    --width 1024 \
    --height 1024 \
    --infer_steps 2 \
    --seed 42 \
    --use_cache \
    --device_type "A2-32g-single" \
    --batch_size 1 \
    --max_num_prompt 2 \
    --do_quant \
    --quant_weight_save_folder "$PROJECT_PATH/output/multi_modal_quant_fa3_flux/results/quant/safetensors" \
    --quant_dump_calib_folder "$PROJECT_PATH/resource/multi_modal_quant_fa3_flux/results/quant/cache" \
    --quant_type "w8a8_dynamic_fa3"
