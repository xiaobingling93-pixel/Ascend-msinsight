#!/bin/bash
base_dir=$(dirname $(readlink -f $0))
workspace=${base_dir}/WorkSpace
result_log="${workspace}/amt_smoke_$(date "+%Y%m%d%H%M%S").log"
output_dir="${workspace}/output"
source ${base_dir}/Utils/download_install.sh
source ${base_dir}/Utils/config.sh

export PATH=${PATH}:/usr/local/python3.7.5.2x/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/python3.7.5.2x/lib

# 支持用户配置的参数
# 1. 验证模块
# 2. 仓链接
# 2. 仓分支
# 3. pytorch版本

while getopts ':g:b:m:v:' OPT; do
    case ${OPT} in
        "g")  # tools URL
            git_url="$OPTARG"
            ;;
        "b")  # tools 分支名
            branch="$OPTARG"
            ;;
        "m")  # 冒烟选择模块
            modules="$OPTARG"
            ;;
        "v")  # PyTorch版本
            version="$OPTARG"
            ;;
        *)
            break
            ;;
    esac
done


run_gpu2npu_ci() {
    #download_install_personal_msfmktransplt ${workspace} $1 $2
    bash ${base_dir}/Testcases/run_gpu2npu.sh $3 ${result_log} ${output_dir}
}

run_profiling_ci() {
    download_install_personal_pytorch ${workspace} $1 $2 $3
    bash ${base_dir}/Testcases/run_profiling.sh $3 ${result_log} ${output_dir}
}

run_ptdbg_ci() {
    local download_path="${workspace}/ptdbg"
    download_ptdbg_tools ${download_path} $1 $2
    install_ptdbg_tools ${download_path} $3
    bash ${base_dir}/Testcases/run_ptdbg.sh $3 ${result_log} ${output_dir}
}

run_api_accuracy_checking_ci() {
    local download_path="${workspace}/api_accuracy_checking"
    download_api_accuracy_checking_tools ${download_path} $1 $2
    bash ${base_dir}/Testcases/run_api_accuracy_checking.sh $3 ${result_log} ${output_dir}
}

run_grad_tool_ci() {
    local download_path="${workspace}/grad_tool"
    download_grad_tool_tools ${download_path} $1 $2
    bash ${base_dir}/Testcases/run_grad_tool.sh $3 ${result_log} ${output_dir}
}

main() {
    local modules=$1
    local git_url=$2
    local branch=$3
    local version=$4

    if [ ! -d  ${workspace} ]; then
        mkdir -p ${workspace}
    fi
    rm -rf ${output_dir}; mkdir -p ${output_dir}
    if [ "${version}" == "pytorch_v1.8.1_py37" ]; then
        version="pytorch_v2.1.0_py38"
    fi

    #declare -A torch_map=(["pytorch_v2.1.0_py38"]="pt2.1.0" ["pytorch_v1.11.0_py37"]="pt1.11.0")
    #source /root/miniforge3/bin/activate ${torch_map[${version}]}
    #if [ "${version}" == "pytorch_v2.1.0_py38" ]; then
        #export LD_PRELOAD=/usr/local/gcc-12.2.0/lib64/libgomp.so.1:$LD_PRELOAD
    #fi
    #local pytorch_download_path="${workspace}/${torch_map[${version}]}_pkg"
    #download_install_pytorch_t ${pytorch_download_path} ${pytorch_version} ${pytorch_b_version} ${day_info} ${arch_info} ${user} ${pwd} ${pytorch_branchs[i]} ${version}
    #rm -rf /root/miniforge3/envs/pt1.11.0/lib/python3.7/site-packages/torch_npu/acl.json
    if [ "gpu2npu" == "${modules}" ]; then
        run_gpu2npu_ci ${git_url} ${branch} ${version}
    elif [ "profiling" == "${modules}" ]; then
        run_profiling_ci ${git_url} ${branch} ${version}
    elif [ "ptdbg" == "${modules}" ]; then
        run_ptdbg_ci ${git_url} ${branch} ${version}
    elif [ "api_accuracy_checking" == "${modules}" ]; then
        run_api_accuracy_checking_ci ${git_url} ${branch} ${version}
    elif [ "grad_tool" == "${modules}" ]; then
        run_grad_tool_ci ${git_url} ${branch} ${version}
    fi
    zip_and_print ${output_dir}
    filter_and_print ${result_log}
}

main ${modules} ${git_url} ${branch} ${version}
