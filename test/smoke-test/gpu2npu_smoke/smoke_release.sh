#!/bin/bash
# export LD_LIBRARY_PATH=/usr/local/python3.7.5/lib:$LD_LIBRARY_PAT
# export PATH=/usr/local/python3.7.5/bin:$PATH
base_dir=$(dirname $(readlink -f $0))
workspace=${base_dir}/WorkSpace
result_log="/home/qkd/att_release_log/amt_smoke_$(date "+%Y%m%d%H%M%S").log"
source ${base_dir}/Utils/download_install.sh
# source ${base_dir}/Utils/config.sh
export PATH=${PATH}:/usr/local/python3.7.5.2x/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/python3.7.5.2x/lib

download_api_accuracy_checking_release() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Download api_accuracy_checking Tools ----------------------------"
    local download_path=$1
    local url=$2
    local branch=$3
    export http_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export https_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export no_proxy=127.0.0.1,.huawei.com,localhost,local,.local
    if [ -d ${download_path} ]; then
         rm -rf ${download_path}
    fi
    git clone ${url} -b ${branch} ${download_path}

}

main() {
    # rm -rf ${workspace}; mkdir -p ${workspace}
    # [1] CMC下载&安装最新CANN包--OK
    # local cann_download_path="${workspace}/cann_pkg"
    # download_install_cann ${cann_download_path} ${cann_version} ${cann_b_version} ${day_info} ${arch_info} ${user} ${pwd}

    # [2] 切换conda，验证v1.11.0和master
    # conda_env_list=("pt1.11.0" "pt2.1.0")
    conda_env_list=("pt1.11.0")
    pytorch_branchs=("pytorch_v1.11.0_py37" "pytorch_v2.1.0_py38")
    for (( i=0; i<${#conda_env_list[@]}; i++)); do
        # [2.1] 切换Conda环境
        source /root/miniforge3/bin/activate ${conda_env_list[i]}
        # if [ "${pytorch_branchs[i]}" == "pytorch_v2.1.0_py38" ]; then
        #     export LD_PRELOAD=/usr/local/gcc-12.2.0/lib64/libgomp.so.1:$LD_PRELOAD
        # fi
        # [2.2] 安装PyTorch最新的包--OK
        # local pytorch_download_path="${workspace}/${conda_env_list[i]}_pkg"
        # download_install_pytorch_t ${pytorch_download_path} ${pytorch_version} ${pytorch_b_version} ${day_info} ${arch_info} ${user} ${pwd} ${pytorch_branchs[i]} 
        # [2.3] 安装ptdbg包
        # local ptdbg_download_path="${workspace}/ptdbg_pkg/tools"
        # local ptdbg_url="https://gitee.com/ascend/att.git"
        # local ptdbg_branch="master"
        # if [ ! -d ${ptdbg_download_path} ]; then
        #     download_ptdbg_tools ${ptdbg_download_path} ${ptdbg_url} ${ptdbg_branch}
        # fi
        # local ptdbg_install_path="${workspace}/${conda_env_list[i]}_ptdbg_pkg"
        # cp -r ${ptdbg_download_path} ${ptdbg_install_path}
        # install_ptdbg_tools ${ptdbg_install_path} ${conda_env_list[i]}

        # [2.4] 安装api_accuracy_checking
        local ptdbg_branch="master"
        local ptdbg_url="https://gitee.com/jiangchangting1/att_release.git"
        rm -rf ${workspace}/api_accuracy_checking
        local api_accuracy_checking_download_path="${workspace}/api_accuracy_checking"
        if [ ! -d ${api_accuracy_checking_download_path} ]; then
            download_api_accuracy_checking_release ${api_accuracy_checking_download_path} ${ptdbg_url} ${ptdbg_branch}
            echo "download finish"
        fi
        rm -rf /root/miniforge3/envs/pt1.11.0/lib/python3.7/site-packages/torch_npu/acl.json
        rm -rf ${workspace}/att_release_output; mkdir -p ${workspace}/att_release_output
        output_dir="${workspace}/att_release_output"
        # [2.5] 执行用例
        # bash ${base_dir}/Testcases/run_profiling.sh ${pytorch_branchs[i]} ${result_log} ${output_dir}
        # bash ${base_dir}/Testcases/run_gpu2npu.sh ${pytorch_branchs[i]} ${result_log} ${output_dir}
        # bash ${base_dir}/Testcases/run_ptdbg.sh ${pytorch_branchs[i]} ${result_log} ${output_dir}
        bash ${base_dir}/Testcases/run_api_accuracy_checking.sh ${pytorch_branchs[i]} ${result_log} ${output_dir}

    done

    cp /home/qkd/att_smoke_new/WorkSpace/att_release_output/api_accuracy_checking/api_accuracy_checking_result.txt /home/qkd/att_release_log/api_accuracy_checking_release_$(date "+%Y%m%d%H%M%S").txt
    # [5] 邮件发送执行结果
    # if [ -f ${result_log} ]; then
    #     cp ${result_log} /home/qkd/att_release_log/
    # fi
}

main
