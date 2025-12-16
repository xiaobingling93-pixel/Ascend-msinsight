#!/bin/bash

prof_path=$1

export ASCEND_GLOBAL_EVENT_ENABLE=1
export ASCEND_GLOBAL_LOG_LEVEL=0
export ASCEND_SLOG_PRINT_TO_STDOUT=1

chmod +x *

python3 lenet_no_profiling.py
