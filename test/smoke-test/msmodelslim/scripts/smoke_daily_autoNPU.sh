#!/usr/bin/env bash

FOLDER=$(ls -r /path/smoke/CANN | head -1)
CANN_PATH="/path/smoke/CANN/${FOLDER}/ascend-toolkit/set_env.sh"
echo CANN_PATH=$CANN_PATH

GIT_PATH="GIT路径"
GIT_BANCH="GIT分支"
REPO_NAME=msit

MAIN_PATH=/path/smoke_auto_sh

clone_success=false
max_retries=10  # 最大重试次数
retry_delay=5  # 重试延迟时间，单位：秒

cd $MAIN_PATH
rm -rf msit

for ((i=1; i<=max_retries; i++)); do
    echo "Attempt $i to clone repository..."
    if git clone -b $GIT_BANCH $GIT_PATH $REPO_NAME; then
        clone_success=true
        break
    else
        echo "Clone failed. Retrying in ${retry_delay} seconds..."
        sleep $retry_delay

        # 清理残留文件
        rm -rf msit
    fi
done

if [ "$clone_success" = false ]; then
    echo "Fatal: Failed to clone repository after $max_retries attempts."
    exit 1
fi

echo "success cloned!"

# 切换环境安装
source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env
cd $MAIN_PATH
echo y | pip uninstall msmodelslim
source $CANN_PATH
cd $MAIN_PATH
cd msit/msmodelslim/
find . -name "*.so" -delete
bash install.sh

# msit包暂留一下，存在用例需要跑example脚本
export MSMODELSLIM_SOURCE_DIR=$MAIN_PATH/msit/msmodelslim

# 压缩编译
cd /path/.conda/envs/smoke_env/lib/python3.10/site-packages/msmodelslim/pytorch/weight_compression/compress_graph/
chmod -R 777 build
bash build.sh ${ASCEND_HOME_PATH}
chmod -R 550 build

# 设置内存使用率阈值（例如 10%）
threshold=12
# 最大等待时间（两小时 = 7200秒）（三小时 = 10800秒）
max_wait_seconds=10800
# 需要获取的空闲卡数量
target_count=2
# 开始时间
start_time=$(date +%s)

# 循环查找空闲 NPU
while true; do
    # 获取NPU数量
    npu_count=$(npu-smi info -l | grep 'NPU ID' | wc -l)
    echo "npu_count: "$npu_count

    # 存储空闲 NPU 的数组
    free_devices=()

    # 假设有 NPU 卡存在
    if [ "$npu_count" -gt 0 ]; then
        # 遍历每个 NPU 卡，检查其状态
        for ((i=0; i<npu_count; i++)); do
            # 获取 NPU 卡的使用率信息
            npu_usage=$(npu-smi info -t usages -i $i 2>/dev/null | grep 'HBM Usage Rate(%)')
            # ' 此处无用，引号用来调整显示格式
            echo "npu_usage=="$npu_usage
            npu_memory_used=$(echo $npu_usage | awk '{print $5}' | tr -d '%')

            # 检查 NPU 卡是否空闲（例如，内存使用率低于 10%）
            if [[ $npu_memory_used -lt $threshold ]]; then
                echo "NPU $i 当前内存使用率: ${npu_memory_used}%"
                free_devices+=($i)  # 将空闲设备加入数组
            fi
        done

        # 检查是否找到足够的空闲设备
        if [ ${#free_devices[@]} -ge $target_count ]; then
            selected_devices="${free_devices[0]},${free_devices[1]}"
            echo "找到空闲 NPU: ${free_devices[@]}"
            echo "选择前两个空闲设备: $selected_devices"
            # 找到空卡，执行冒烟
            export PYTORCH_NPU_ALLOC_CONF=expandable_segments:False
            export ASCEND_RT_VISIBLE_DEVICES=$selected_devices
            # export ASCEND_RT_VISIBLE_DEVICES=0,1

            # 传入给docker时获取
            export CANN_PATH=$CANN_PATH
            export GIT_PATH=$GIT_PATH
            export GIT_BANCH=$GIT_BANCH

            cd /path/ModelSlim_Smoke/ModelSlim
            bash scripts/run-modelslim.sh -v rc1

            exit 0
        else
            echo "未找到足够空闲 NPU，继续监控（间隔 5min ）..."
            sleep 300
        fi
    else
        echo "No NPU cards found."
        exit 1
    fi

    # 检查是否超过最大等待时间（两小时）
    current_time=$(date +%s)
    elapsed_time=$((current_time - start_time))
    if [ "$elapsed_time" -ge "$max_wait_seconds" ]; then
        echo "Max wait time reached (3 hours). Exiting..."
        exit 1
    fi
done
