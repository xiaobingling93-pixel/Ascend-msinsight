#!/bin/bash

prof_path=$1

export ASCEND_GLOBAL_EVENT_ENABLE=1
export ASCEND_GLOBAL_LOG_LEVEL=0
export ASCEND_SLOG_PRINT_TO_STDOUT=1

source /usr/local/Ascend/ascend-toolkit/set_env.sh

bash run.sh Ascend910B4 npu_onboard

msprof --output=${prof_path} --ascendcl=on --model-execution=on --runtime-api=on --task-time=l0 --ai-core=on --aic-freq=100 --sys-hardware-mem=on --sys-hardware-mem-freq=50 --sys-cpu-profiling=on --sys-cpu-freq=50 --sys-profiling=on --sys-sampling-freq=10 --sys-pid-profiling=on --sys-pid-sampling-freq=10 --sys-io-profiling=on --sys-io-sampling-freq=100 --dvpp-profiling=on --dvpp-freq=50 --sys-interconnection-profiling=on --sys-interconnection-freq=50 --l2=on --application=./add_npu


