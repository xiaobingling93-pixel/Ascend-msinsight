#!/usr/bin/env bash

source /opt/miniconda3/bin/activate osp1.2

export ASCEND_LAUNCH_BLOCKING=1
export PYTORCH_NPU_ALLOC_CONF="max_split_size_mb:256"


# 获取当前脚本的路径
SCRIPT_DIR=$(dirname "$(realpath "$0")")  # "
echo "当前脚本路径: $SCRIPT_DIR"


# add msmodelslim package path to pythonpath
 MSMODELSLIM_SOURCE_DIR=${MSMODELSLIM_SOURCE_DIR:-"${PROJECT_PATH}/resource/msit/msmodelslim"}
export PYTHONPATH=${MSMODELSLIM_SOURCE_DIR}:$PYTHONPATH


# 设置参数
# 输入路径
MODEL_PATH=$PROJECT_PATH/resource/multi_modal/opensoraplan_project
TEXT_PROMPT=$PROJECT_PATH/resource/ditcache-t2v-prompt/prompt_list_1.txt
FLOAT_CALIB_VIDEO_DIR=$PROJECT_PATH/resource/multi_modal/opensoraplan_project/sample-optimize-calibration-videos-29x480p
CALIB_VIDEO_PATH=$PROJECT_PATH/resource/multi_modal/opensoraplan_project/sample-optimize-calibration-videos-29x480p


# 输出路径
TIMESTEP_SAVE_DIR=$SCRIPT_DIR/result
TIMESTEP_SAVE_PATH=$TIMESTEP_SAVE_DIR/searched_schedule.txt
SAVE_IMG_PATH=$SCRIPT_DIR/result/generated_vids_using_new_timestep

rm $SAVE_IMG_PATH/*.mp4

NUM_STEPS=3


echo "开始设置 DEVICES 环境变量..."

# 检查是否设置了 ASCEND_RT_VISIBLE_DEVICES 环境变量
if [ -n "${DEVICES}" ]; then
    echo "检测到 DEVICES 环境变量已设置，值为: ${DEVICES}"
    # 
else
    echo "DEVICES 环境变量未设置，继续检查 ASCEND_RT_VISIBLE_DEVICES 环境变量..."
    
    # 检查是否设置了 ASCEND_RT_VISIBLE_DEVICES 环境变量
    if [ -n "${ASCEND_RT_VISIBLE_DEVICES}" ]; then
        DEVICES=${ASCEND_RT_VISIBLE_DEVICES}
        echo "检测到 ASCEND_RT_VISIBLE_DEVICES 环境变量已设置，值为: ${ASCEND_RT_VISIBLE_DEVICES}"
    else
        echo "ASCEND_RT_VISIBLE_DEVICES 环境变量未设置，使用默认值: 1,2,3,4"
        DEVICES="1,2,3,4"
    fi
fi

IFS=',' read -r -a devices <<< "$DEVICES"
device_count=${#devices[@]}


# 输出结果以验证
echo "TEXT_PROMPT: $TEXT_PROMPT"
echo "CACHE_SAVE_PATH: $CACHE_SAVE_PATH"
echo "SAVE_IMG_PATH: $SAVE_IMG_PATH"
echo "Using $device_count devices to run"

# run sample_optimization timestep search
export ASCEND_RT_VISIBLE_DEVICES="${DEVICES}"
(
torchrun --nnodes=1 --nproc_per_node=$device_count --master_port ${PORT:-29503} \
    -m example.osp1_2.search_t2v_sp \
    --model_path ${MODEL_PATH}/Open-Sora-Plan-v1.2.0/29x480p/ \
    --num_frames 29 \
    --height 480 \
    --width 640 \
    --cache_dir "../cache_dir" \
    --text_encoder_name ${MODEL_PATH}/mt5-xxl-original/ \
    --text_prompt ${TEXT_PROMPT} \
    --ae CausalVAEModel_D4_4x8x8 \
    --ae_path "${MODEL_PATH}/Open-Sora-Plan-v1.2.0/vae/" \
    --fps 24 \
    --guidance_scale 7.5 \
    --num_sampling_steps ${NUM_STEPS} \
    --enable_tiling \
    --tile_overlap_factor 0.125 \
    --max_sequence_length 512 \
    --sample_method EulerAncestralDiscrete \
    --model_type "dit" \
    --save_memory \
    --search_type "restep" \
    --videos_path $CALIB_VIDEO_PATH \
    --save_dir ${TIMESTEP_SAVE_DIR} 
)  || { echo "Search failed with exit status $?"; exit 1; }

## run inference with dit-cache
export ASCEND_RT_VISIBLE_DEVICES="${DEVICES}"
(
torchrun --nnodes=1 --nproc_per_node=$device_count --master_port 29503 \
    -m example.osp1_2.sample_t2v_sp  \
    --model_path ${MODEL_PATH}/Open-Sora-Plan-v1.2.0/29x480p/ \
    --num_frames 29 \
    --height 480 \
    --width 640 \
    --cache_dir "../cache_dir" \
    --text_encoder_name ${MODEL_PATH}/mt5-xxl-original/ \
    --text_prompt ${TEXT_PROMPT} \
    --ae CausalVAEModel_D4_4x8x8 \
    --ae_path ${MODEL_PATH}/Open-Sora-Plan-v1.2.0/vae/ \
    --save_img_path ${SAVE_IMG_PATH} \
    --fps 24 \
    --guidance_scale 7.5 \
    --num_sampling_steps ${NUM_STEPS} \
    --save_memory \
    --max_sequence_length 512 \
    --sample_method EulerAncestralDiscrete \
    --model_type "dit" \
    --enable_tiling \
    --tile_overlap_factor 0.125 \
    --schedule_timestep ${TIMESTEP_SAVE_PATH}

) || { echo "Inference failed with exit status $?"; exit 1; }
