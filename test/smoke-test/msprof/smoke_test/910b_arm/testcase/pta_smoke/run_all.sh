#!/bin/bash

pta_dir=$(cd $(dirname $0)/; pwd)
export PYTHONPATH=$PYTHONPATH:/home/msprof_smoke_test/smoke_test
result_file=/home/msprof_smoke_test/smoke_test/910b_arm/testcase/result.txt
output_path=/home/result_dir/pta_smoke_case
source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta

rm -rf $output_path

if [ "$1" = "master" ]; then
    versions=("2.6.0" "2.7.1" "2.8.0") # 这里配置daily时的版本列表
else
    versions=("2.6.0") # 非daily时，只执行master版本
fi

test_list=`ls |grep ".sh" | grep -E "test_ascend_pytorch_profiler"`

# 1. 初始化msmonitor环境
function installMsmonitor() {
    cd /home/msprof_smoke_test
    rm -rf mstt
    git clone https://gitcode.com/Ascend/mstt.git
    cd mstt/msmonitor/dynolog_npu/
    sed -i '16s/ON/OFF/' CMakeLists.txt
    cd ..
    # 编dynolog和dyno二进制文件
    bash scripts/build.sh
    cd plugin
    sed -i '/find_package(Python REQUIRED COMPONENTS Interpreter Development)/a find_library(LIBUNWIND_LIBRARY unwind REQUIRED)' CMakeLists.txt
    sed -i '$a target_link_libraries(IPCMonitor PRIVATE ${LIBUNWIND_LIBRARY})' CMakeLists.txt
    # 编IPCMonitor whl包
    bash build.sh
    # 安装IPCMonitor whl包
    pip install dist/*.whl --force-reinstall --no-deps
    rm -rf IPCMonitor/
}

installMsmonitor

cd $pta_dir

# 2. 跑全量PTA用例
for version in "${versions[@]}"; do
  case "$version" in
  "2.6.0")
    pip install torchvision==0.21.0
    ;;
  "2.7.1")
    pip install torchvision==0.22.0
    ;;
  "2.8.0")
    pip install torchvision==0.23.0
    ;;
  esac
  bash install_pta_pkg.sh $version
  if [ ! -z "$test_list" ]; then
    echo "[DEBUG] there are test_list: "$test_list
    for i in $test_list
    do
        echo "start ${i}_${version} testcase:===================="
        start_time=$(date "+%s")
        bash $i $output_path $version
        script_exit_code=$?
        end_time=$(date "+%s")
        duration_time=$(( ${end_time} - ${start_time} ))
        if [ 0 -ne $script_exit_code ]; then
            echo "${i}_${version} fail ${duration_time}" >> $result_file
        else
            echo "${i}_${version} pass ${duration_time}" >> $result_file
        fi
        echo "end ${i}_${version} testcase:======================"
    done
    echo "[DEBUG] end pta_smoke_test"
  else
    echo "[DEBUG] there none test case: "$test_list
  fi
done