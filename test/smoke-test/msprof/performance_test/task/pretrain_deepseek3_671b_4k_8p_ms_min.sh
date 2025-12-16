#!/bin/bash

source /usr/local/Ascend/ascend-toolkit/set_env.sh;
source /usr/local/Ascend/nnal/atb/set_env.sh

CUR_DIR=$(dirname "$(readlink -f "$0")")
MindSpeed_Core_MS_PATH=$CUR_DIR/../../../../
export PYTHONPATH=${MindSpeed_Core_MS_PATH}/Megatron-LM:$PYTHONPATH
export PYTHONPATH=${MindSpeed_Core_MS_PATH}/MindSpeed:$PYTHONPATH
export PYTHONPATH=${MindSpeed_Core_MS_PATH}/MindSpeed-LLM:$PYTHONPATH
#export PYTHONPATH=${MindSpeed_Core_MS_PATH}/MSAdapter/mindtorch:$PYTHONPATH
export PYTHONPATH=${MindSpeed_Core_MS_PATH}/msadapter:$PYTHONPATH
export PYTHONPATH=${MindSpeed_Core_MS_PATH}/msadapter/msa_thirdparty:$PYTHONPATH

export CUDA_DEVICE_MAX_CONNECTIONS=1
export PYTORCH_NPU_ALLOC_CONF=expandable_segments:True
export HCCL_CONNECT_TIMEOUT=3600
export HCCL_ALGO="alltoall=level0:NA;level1:pipeline"
export HCCL_BUFFSIZE=400
#export PYTHONPATH=/home/y30062407/mstt/debug/accuracy_tools:$PYTHONPATH
export HCCL_DETERMINISTIC=true
export ASCEND_LAUNCH_BLOCKING=1
export NCCL_DETERMINISTIC=1
#内存采集
#export MS_DEV_HOST_BLOCKING_RUN=1
#export MS_DEV_LAUNCH_BLOCKING=1
#export MS_ALLOC_CONF="memory_tracker:True"

NPUS_PER_NODE=8
MASTER_ADDR=localhost #MASTER IP
MASTER_PORT=6000
NNODES=1
NODE_RANK=0
WORLD_SIZE=$(($NPUS_PER_NODE*$NNODES))

# CKPT_SAVE_DIR="./ckpt"
DATA_PATH="/home/profiler_performance_bak/enwiki/enwiki_text_document"
TOKENIZER_PATH="/home/profiler_performance_bak/data/jenkins0_workspace/l30009791/ds_dataset/deepseek3-hf"
# CKPT_LOAD_DIR="./ckpt"


TP=1
PP=2
EP=4
CP=1
CP_TYPE='ulysses_cp_algo'
NUM_LAYERS=4
SEQ_LEN=4096
#SEQ_LEN=1024
MBS=1
GBS=16

DISTRIBUTED_ARGS="
    --master_addr $MASTER_ADDR \
    --node_rank $NODE_RANK \
    --worker_num $WORLD_SIZE \
    --local_worker_num $NPUS_PER_NODE \
    --master_port $MASTER_PORT \
    --log_dir=msrun_log \
    --join=True \
"

MLA_ARGS="
    --multi-latent-attention \
    --qk-pos-emb-head-dim 64 \
    --qk-head-dim 128 \
    --q-lora-rank 1536 \
    --kv-lora-rank 512 \
    --v-head-dim 128 \
    --qk-layernorm \
    --mla-fa-without-pad \
"

MOE_ARGS="
    --moe-router-dtype fp32 \
    --moe-grouped-gemm \
    --moe-permutation-async-comm \
    --moe-token-dispatcher-type alltoall_seq \
    --first-k-dense-replace 1 \
    --moe-layer-freq 1 \
    --n-shared-experts 1 \
    --num-experts 32 \
    --moe-router-topk 8 \
    --moe-ffn-hidden-size 2048 \
    --moe-router-load-balancing-type none \
    --moe-router-num-groups 8 \
    --moe-router-group-topk 4 \
    --moe-router-topk-scaling-factor 2.5 \
    --moe-aux-loss-coeff 0.0001 \
    --seq-aux \
    --norm-topk-prob \
    --moe-router-score-function sigmoid \
    --moe-router-enable-expert-bias \
    --moe-tp-extend-ep
"

MTP_ARGS="
    --mtp-num-layers 1 \
    --mtp-loss-scaling-factor 0.3 \
    --mtp-mem-efficient-logits \
"

ROPE_ARGS="
    --beta-fast 32 \
    --beta-slow 1 \
    --rope-scaling-factor 40 \
    --rope-scaling-mscale 1.0 \
    --rope-scaling-mscale-all-dim 1.0 \
    --rope-scaling-original-max-position-embeddings 4096 \
    --rope-scaling-type yarn
"

DUALPIPE_ARGS="
    --moe-fb-overlap \
    --schedules-method dualpipev \
"

MEM_ARGS="
    --use-distributed-optimizer \
    --recompute-method uniform \
    --recompute-granularity full \
    --recompute-num-layers 1 \
"

GPT_ARGS="
    --spec mindspeed_llm.tasks.models.spec.deepseek_spec layer_spec \
    --no-gradient-accumulation-fusion \
    --reset-position-ids \
    --noop-layers 3 \
    --no-shared-storage \
    --use-flash-attn \
    --use-mcore-models \
    --tensor-model-parallel-size ${TP} \
    --pipeline-model-parallel-size ${PP} \
    --expert-model-parallel-size ${EP} \
    --sequence-parallel \
    --context-parallel-size ${CP} \
    --context-parallel-algo  ${CP_TYPE} \
    --num-layers ${NUM_LAYERS} \
    --hidden-size 7168 \
    --ffn-hidden-size 18432 \
    --num-attention-heads 128 \
    --tokenizer-type PretrainedFromHF  \
    --tokenizer-name-or-path ${TOKENIZER_PATH} \
    --seq-length ${SEQ_LEN} \
    --max-position-embeddings 163840 \
    --micro-batch-size ${MBS} \
    --global-batch-size ${GBS} \
    --make-vocab-size-divisible-by 1 \
    --lr 1.0e-5 \
    --train-iters 15 \
    --lr-decay-style cosine \
    --untie-embeddings-and-output-weights \
    --disable-bias-linear \
    --attention-dropout 0.0 \
    --init-method-std 0.02 \
    --hidden-dropout 0.0 \
    --position-embedding-type rope \
    --normalization RMSNorm \
    --use-fused-rotary-pos-emb \
    --use-rotary-position-embeddings \
    --use-fused-swiglu \
    --use-fused-rmsnorm \
    --swiglu \
    --no-masked-softmax-fusion \
    --attention-softmax-in-fp32 \
    --min-lr 1.0e-7 \
    --weight-decay 1e-2 \
    --lr-warmup-iters 2 \
    --clip-grad 1.0 \
    --adam-beta1 0.9 \
    --adam-beta2 0.999 \
    --initial-loss-scale 65536 \
    --vocab-size 129280 \
    --padded-vocab-size 129280 \
    --rotary-base 10000 \
    --norm-epsilon 1e-6 \
    --no-load-optim \
    --no-load-rng \
    --bf16 \
    --use-ascend-coc \
    --coc-fused-kernel \
    --distributed-timeout-minutes 120
"

DATA_ARGS="
    --data-path $DATA_PATH \
    --split 100,0,0
"

OUTPUT_ARGS="
    --log-interval 1 \
    --save-interval 2000 \
    --eval-interval 2000 \
    --eval-iters 0 \
    --no-save-optim \
    --no-save-rng
"

PROFILE_ARGS="
    --profile \
    --profile-step-start 2 \
    --profile-step-end 12 \
    --profile-ranks 0 1 2 3 4 5 6 7 \
    --profile-level level0  \
    --profile-with-cpu  \
    --profile-save-path /home/profiler_performance/task/output/profiler/dp_result/ \
"

msrun $DISTRIBUTED_ARGS pretrain_gpt.py \
    $GPT_ARGS \
    $DATA_ARGS \
    $OUTPUT_ARGS \
    $MLA_ARGS \
    $ROPE_ARGS \
    $MOE_ARGS \
    $MTP_ARGS \
    $MEM_ARGS \
    $PROFILE_ARGS \
    --distributed-backend nccl \
    --ai-framework mindspore \
    2>&1 | tee logs/ms_pretrain_deepseek3_671b_4k_8p_ptd.log
