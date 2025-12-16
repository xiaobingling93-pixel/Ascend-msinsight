#!/bin/bash
# ***********************************************************************
# Copyright: (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# crontab for mindspore profiler performance test
# version: 
# change log:
# ***********************************************************************

root_dir=$(cd $(dirname $0)/../; pwd)

cd $root_dir

cron_add() {
    #如果是第一次使用crontab，则crontab -l返回1，为了防止脚本退出，后面加个echo 0
    crontab -l > ${root_dir}/crontab.txt 2>/dev/null | echo 0 >/dev/null
    echo "5 20 * * * $@" >> ${root_dir}/crontab.txt
    crontab ${root_dir}/crontab.txt
    rm -f ${root_dir}/crontab.txt
}

cron_del() {
    crontab -l > ${root_dir}/crontab.txt 2>/dev/null | echo 0 >/dev/null
    if [ -f "${root_dir}/crontab.txt" ]; then
        crontab ${root_dir}/crontab.txt
        rm -f ${root_dir}/crontab.txt
    fi
}

set_crontab()
{
    echo "set_crontab"

    # crontab | echo 0 >/dev/null
    echo "cron add \"5 20 * * * bash ${root_dir}/task/run_crontab_performance_smoke.sh\""
    cron_add "bash ${root_dir}/task/run_crontab_performance_smoke.sh > ${root_dir}/task/log.log 2>&1"
}

# 清理定时器
clear_crontab()
{
    echo "clear_crontab"
    cron_del
}

if [ "$1" == 0 ]; then
  clear_crontab
elif [ "$1" == 1 ]; then
  set_crontab
else
  echo "param error!!!"
fi