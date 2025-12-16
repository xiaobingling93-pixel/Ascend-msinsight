#!/bin/bash
# ***********************************************************************
# Copyright: (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# crontab for mindspore profiler performance test
# version: 
# change log:
# ***********************************************************************

root_dir=$(cd $(dirname $0)/../; pwd)

cd $root_dir

source ~/.bashrc
source /root/miniconda3/bin/activate performance310
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export PYTHONPATH=$PYTHONPATH:/home/profiler_performance/

# python ${root_dir}/task/run_daily_performance_smoke.py \
#   --model_name tiny_transformer \
#   --result_dir output/crontab_result

# python ${root_dir}/task/run_env_prepare.py

cd ${root_dir}/task/
# bash ${root_dir}/task/run_install_mstt.sh
# rm -rf ${root_dir}/task/Ascend-Insight
# git clone ssh://git@codehub-dg-y.huawei.com:2222/mindstudio/MindStudio-IDE/Ascend-Insight.git
# cp constants.ts Ascend-Insight/e2e/src/utils/
# cp playwright.config.ts Ascend-Insight/e2e/

bash ${root_dir}/task/Insight/start.sh &
sever_pid=$!

# python ${root_dir}/task/Ascend-Insight/modules/build/build.py
# cd ${root_dir}/task/Ascend-Insight/e2e
# npm install
# npx playwright install
cd ${root_dir}/task/Ascend-Insight/modules/framework
npm run staging &
front_pid=$!

cd $root_dir
python ${root_dir}/task/pt_pretrain_deepseek3_671b.py
python ${root_dir}/task/ms_pretrain_deepseek3_671b.py
python ${root_dir}/task/send_result.py


kill $sever_pid 2>/dev/null
wait $sever_pid 2>/dev/null
kill $front_pid 2>/dev/null
wait $front_pid 2>/dev/null

trap_ctrlc() {
  kill -9 ${sever_pid} && kill -9 ${front_pid}
}

trap trap_ctrlc INT
