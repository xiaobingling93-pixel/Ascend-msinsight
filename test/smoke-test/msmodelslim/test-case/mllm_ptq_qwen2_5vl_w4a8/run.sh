#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

source /opt/miniconda3/etc/profile.d/conda.sh
conda deactivate
conda activate smoke_env

# 更新依赖 该用例需要transformers==4.46.0，tokenizers==0.20.4，低版本可能不兼容
echo y | pip uninstall transformers

# 定义依赖项（格式：["pip包名"]="导入名"）如 pip opencv-python 包对应 import cv2
declare -A LIB_MAP=(
    ["qwen_vl_utils"]="qwen_vl_utils"
)

for pkg in "${!LIB_MAP[@]}"; do
    import_name=${LIB_MAP[$pkg]}
    if ! python3 -c "import ${import_name}" &>/dev/null; then
        echo "正在安装 ${pkg}（导入名: ${import_name}）..."
        pip3 install ${pkg}
    else
        echo "${import_name} 已安装"
    fi
done

pip install $PROJECT_PATH/resource/pypi_whl/transformers/transformers-4.49.0-py3-none-any.whl --no-dependencies
pip install $PROJECT_PATH/resource/pypi_whl/tokenizers/tokenizers-0.21.0-cp39-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl --no-dependencies
pip install $PROJECT_PATH/resource/pypi_whl/huggingface_hub/huggingface_hub-0.26.0-py3-none-any.whl --no-dependencies

rm -rf $PROJECT_PATH/output/mllm_ptq_qwen2_5vl_w4a8

# 执行量化脚本
bash quant_qwen2_5vl_w4a8.sh

if [ $? -eq 0 ]
then
    echo mllm_ptq_qwen2_5vl_w4a8: Success
else
    echo mllm_ptq_qwen2_5vl_w4a8: Failed
    run_ok=$ret_failed
fi

conda activate main_env
# 清理output
rm -rf $PROJECT_PATH/output/mllm_ptq_qwen2_5vl_w4a8

exit $run_ok