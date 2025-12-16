#!/bin/bash

prof_path=$1

export ASCEND_DEVICE_ID=0
export ASCEND_GLOBAL_EVENT_ENABLE=1
export ASCEND_GLOBAL_LOG_LEVEL=0
export ASCEND_SLOG_PRINT_TO_STDOUT=1 # 日志打屏设置: 0不开启打屏, 1开启打屏

if [ -d "cache" ]; then
    rm -r cache
    rm -r kernel_meta_*
    rm -r PROF_*
fi

if [ -d "result_dir" ]; then
    rm -r result_dir/*
fi

if [ -f "fusion_result.json" ]; then
    rm -f fusion_result.json
fi

chmod +x *
./SingleOp_for_Pytorch.py
/usr/local/Ascend/ascend-toolkit/latest/tools/profiler/bin/msprof --export=on --output=./result_dir/
cp -r ./result_dir/*_ascend_pt ${prof_path}/

