#!/bin/bash
file_path="/home/msprof_smoke_test/model/Analysis_check_db_text_result/PROF_000001_20241102063141439_ALHDGJJOHGHNAFBB/*"
output_path="/home/result_dir/test_analysis_db_result_is_the_same_as_text_result"
# 校验默认导出统一db，不再单独跑--type=db，--type=db有test_pixel_level_check看护
# db_dir=$output_path/PROF_db
text_dir=$output_path/PROF_text
rm -rf $output_path
# mkdir -p $db_dir
mkdir -p $text_dir
# cp -r $file_path $db_dir
cp -r $file_path $text_dir
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /root/miniconda3/bin/activate smoke_test_env_bak
# msprof --export=on --type=db --output=$db_dir > /dev/null 2>&1 & msprof --export=on --type=text --output=$text_dir > /dev/null 2>&1
msprof --export=on --output=$text_dir > /dev/null 2>&1
# 调用python脚本
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/check_db_and_text_result.py
