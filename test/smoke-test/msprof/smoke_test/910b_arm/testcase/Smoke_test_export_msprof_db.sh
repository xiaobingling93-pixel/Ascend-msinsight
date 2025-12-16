#!/bin/bash
set -e
file_path="/home/msprof_smoke_test/model/Analysis_check_db_text_result/PROF_000001_20241102063141439_ALHDGJJOHGHNAFBB/*"
output_path="/home/result_dir/Smoke_test_export_msprof_db"
db_dir=$output_path/PROF_db
rm -rf $output_path
mkdir -p $db_dir
cp -r $file_path $db_dir
source /usr/local/Ascend/ascend-toolkit/set_env.sh
msprof --export=on --type=db --output=$db_dir
log_path=$db_dir/mindstudio_profiler_log

if grep -rn ERROR $log_path > /dev/null
then
    echo "Smoke_test_export_msprof_db fail" >> result.txt
else
    echo "Smoke_test_export_msprof_db pass" >> result.txt
fi
