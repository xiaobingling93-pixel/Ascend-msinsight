#!/bin/bash

# export LD_LIBRARY_PATH=/usr/local/python3.7.5/lib:$LD_LIBRARY_PAT
# export PATH=/usr/local/python3.7.5/bin:$PATH

base_dir=$(dirname $(readlink -f $0))
workspace=${base_dir}/WorkSpace
result_log1="${workspace}/amt_smoke1_$(date "+%Y%m%d%H%M%S").log"
result_log2="${workspace}/amt_smoke2_$(date "+%Y%m%d%H%M%S").log"
output_base_dir="${workspace}/output_version"
source ${base_dir}/Utils/download_install.sh
source ${base_dir}/Utils/config.sh
export PATH=${PATH}:/usr/local/python3.7.5.2x/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/python3.7.5.2x/lib

while getopts ':cv:pv:' OPT; do
    case ${OPT} in
        "cv")  # tools URL
            cann_version1="$OPTARG"
            ;;
        "pv")  # tools 分支名
            pytorch_version="$OPTARG"
            ;;

        *)
            break
            ;;
    esac
done

main() {
    
    # echo $0
    # echo $1
    # echo $2
    local cann_version=$1
    echo $cann_version
    local pytorch_version=$2
    echo $pytorch_version
    # rm -rf ${workspace}; mkdir -p ${workspace}
    # [1] CMC下载&安装最新CANN包--OK
    
    local cann_download_path="${workspace}/cann_pkg"
    download_install_cann_version ${cann_download_path} ${cann_version} ${cann_b_version} ${day_info} ${arch_info} ${user} ${pwd}
    pytorch_branchs=("pytorch_v1.11.0_py37" "pytorch_v2.1.0_py38")
    echo ${pytorch_branchs[0]}
    conda_env_list=("pt1.11.0" "pt2.1.0")
    local pytorch_download_path="${workspace}/${conda_env_list[i]}_pkg"
    download_install_pytorch_version ${pytorch_download_path} ${pytorch_version} ${pytorch_b_version} ${day_info} ${arch_info} ${user} ${pwd} ${pytorch_branchs[0]}
    # # [2] 切换conda，验证v1.11.0和master
    # conda_env_list=("pt1.11.0" "pt2.1.0")
    # pytorch_branchs=("pytorch_v1.11.0_py37" "pytorch_v2.1.0_py38")
    # for (( i=0; i<${#conda_env_list[@]}; i++)); do
    #     # [2.1] 切换Conda环境
    #     source /root/miniconda3/bin/activate ${conda_env_list[i]}
    #     if [ "${pytorch_branchs[i]}" == "pytorch_v2.1.0_py38" ]; then
    #         export LD_PRELOAD=/usr/local/gcc-12.2.0/lib64/libgomp.so.1:$LD_PRELOAD
    #     fi
    #     output_dir="${output_base_dir}/${pytorch_branchs[i]}"
    #     # [2.2] 安装PyTorch最新的包--OK
    #     local pytorch_download_path="${workspace}/${conda_env_list[i]}_pkg"
    #     download_install_pytorch_t ${pytorch_download_path} ${pytorch_version} ${pytorch_b_version} ${day_info} ${arch_info} ${user} ${pwd} ${pytorch_branchs[i]} 
    #     # [2.3] 安装ptdbg包
    #     local ptdbg_download_path="${workspace}/ptdbg_pkg/tools"
    #     local ptdbg_url="https://gitee.com/ascend/att.git"
    #     local ptdbg_branch="develop"
    #     if [ ! -d ${ptdbg_download_path} ]; then
    #         download_ptdbg_tools ${ptdbg_download_path} ${ptdbg_url} ${ptdbg_branch}
    #     fi
    #     local ptdbg_install_path="${workspace}/${conda_env_list[i]}_ptdbg_pkg"
    #     cp -r ${ptdbg_download_path} ${ptdbg_install_path}
    #     install_ptdbg_tools ${ptdbg_install_path} ${conda_env_list[i]}

    #     # [2.4] 安装api_accuracy_checking
    #     local api_accuracy_checking_download_path="${workspace}/api_accuracy_checking"
    #     if [ ! -d ${api_accuracy_checking_download_path} ]; then
    #         download_api_accuracy_checking_tools ${api_accuracy_checking_download_path} ${ptdbg_url} ${ptdbg_branch}
    #     fi
    #     rm -rf /root/miniconda3/envs/pt1.11.0/lib/python3.7/site-packages/torch_npu/acl.json
    #     # [2.5] 执行用例
    #     bash ${base_dir}/Testcases/run_gpu2npu.sh ${pytorch_branchs[i]} ${result_log1} ${output_dir}
    #     bash ${base_dir}/Testcases/run_ptdbg.sh ${pytorch_branchs[i]} ${result_log1} ${output_dir}
    #     bash ${base_dir}/Testcases/run_api_accuracy_checking.sh ${pytorch_branchs[i]} ${result_log1} ${output_dir}
    #     bash ${base_dir}/Testcases/run_profiling.sh ${pytorch_branchs[i]} ${result_log2} ${output_dir}

    # done
    # [5] 邮件发送执行结果
    # if [ -f ${result_log1} || -f ${result_log2} ]; then
    #     python3 ${base_dir}/Utils/send_email.py ${result_log1} ${result_log2}
    # fi
}

# main ${CANN_VERSION}
main "$1" "$2"