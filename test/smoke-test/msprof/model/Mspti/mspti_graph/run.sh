#!/bin/bash

OUTPUT_PATH=$1

export ASCEND_GLOBAL_EVENT_ENABLE=1;
export ASCEND_SLOG_PRINT_TO_STDOUT=0;
export ASCEND_PROCESS_LOG_PATH=${OUTPUT_PATH}/plog;

export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so

export DB_LOGGER_ENABLE=1

torchrun --nnodes=1 --nproc_per_node=2 --node_rank=0 --master_port=29500 ./test_replay.py
mv ./activity_log_*.db ${OUTPUT_PATH}
