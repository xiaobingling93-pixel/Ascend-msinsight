#!/bin/bash

prof_path=$1

export ASCEND_GLOBAL_EVENT_ENABLE=1
export ASCEND_GLOBAL_LOG_LEVEL=0
export ASCEND_SLOG_PRINT_TO_STDOUT=1

bash compile.sh

cd out; chmod +x *

./main ${prof_path}

prof_name=`find ${prof_path}/ -name "PROF_*"`
/usr/local/Ascend/ascend-toolkit/latest/tools/profiler/bin/msprof --export=on --output=${prof_name}
