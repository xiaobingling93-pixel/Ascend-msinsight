#!/bin/echo Warning:this is a library should be sourced!

download_install_cann() {
    echo "[INFO][$(date "+%F %T")] ---------------------------- Install CANN ----------------------------"
    local download_path=$1
    local cann_version=$2
    local cann_b_version=$3
    local day_info=$4
    local arch_info=$5
    local user=$6
    local pwd=$7
    if [ ! -d ${download_path} ]; then
        mkdir -p ${download_path}
    fi
    local version_info="${download_path}/version.txt"
    local install_log="${download_path}/install.log"
    # Download CANN-Toolkit
    artget search "CANN ${cann_version}" | grep "CANN ${cann_version}.B${cann_b_version}-${day_info}" | grep "Release-Snapshot" > ${version_info}
    build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    if [ -n ${build_version} ]; then
        new_day_info=$(date -d"yesterday ${day_info}" +%Y%m%d)
        artget search "CANN ${cann_version}" | grep "CANN ${cann_version}.B${cann_b_version}-${new_day_info}" | grep "Release-Snapshot" > ${version_info}
        build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
        if [ ! -n ${build_version} ]; then
            return
        fi
    fi
    artget pull "CANN ${build_version}" -ru software -rp run/${arch_info}-linux/Ascend-cann-toolkit_*_linux-${arch_info}.run -ap ${download_path} -user ${user} -pwd ${pwd} -o CANN > ${install_log}
    artget pull "CANN ${build_version}" -ru software -rp run/Ascend-cann-kernels-910_*.run -ap ${download_path} -user ${user} -pwd ${pwd} -o CANN > ${install_log}
    # Uninstall CANN-Toolkit
    if [ -f ${download_path}/*cann-toolkit*.run ]; then
        rm -rf /etc/Ascend/ascend_cann_install.info
        rm -rf /home/jct/Ascend2/ascend-toolkit
        # Install CANN-Toolkit
        chmod +x ${download_path}/*.run
        echo "Y" | ${download_path}/*cann-toolkit*.run --install --force >> ${install_log}
        echo "Y" | ${download_path}/Ascend-cann-kernels-910_*.run --install >> ${install_log}
    fi
}

download_install_cann_version() {
    echo "[INFO][$(date "+%F %T")] ---------------------------- Install CANN ----------------------------"
    local download_path=$1
    local cann_version=$2
    local cann_b_version=$3
    local day_info=$4
    local arch_info=$5
    local user=$6
    local pwd=$7
    if [ ! -d ${download_path} ]; then
        mkdir -p ${download_path}
    fi
    local version_info="${download_path}/version.txt"
    local install_log="${download_path}/install.log"
    echo ${cann_version}
    # Download CANN-Toolkit
    artget search "CANN ${cann_version}" | grep "CANN ${cann_version}" | grep "Release-Snapshot" > ${version_info}
    # build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    # if [ -n ${build_version} ]; then
    #     new_day_info=$(date -d"yesterday ${day_info}" +%Y%m%d)
    #     artget search "CANN ${cann_version}" | grep "CANN ${cann_version}.B${cann_b_version}-${new_day_info}" | grep "Release-Snapshot" > ${version_info}
    #     build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    #     if [ ! -n ${build_version} ]; then
    #         return
    #     fi
    # fi
    artget pull "CANN ${cann_version}" -ru software -rp run/${arch_info}-linux/Ascend-cann-toolkit_*_linux-${arch_info}.run -ap ${download_path} -user ${user} -pwd ${pwd} -o CANN > ${install_log}
    artget pull "CANN ${cann_version}" -ru software -rp run/Ascend-cann-kernels-910_*.run -ap ${download_path} -user ${user} -pwd ${pwd} -o CANN > ${install_log}
    # # Uninstall CANN-Toolkit
    # if [ -f ${download_path}/*cann-toolkit*.run ]; then
    #     rm -rf /etc/Ascend/ascend_cann_install.info
    #     rm -rf /home/jct/Ascend2/ascend-toolkit
    #     # Install CANN-Toolkit
    #     chmod +x ${download_path}/*.run
    #     echo "Y" | ${download_path}/*cann-toolkit*.run --install --force >> ${install_log}
    #     echo "Y" | ${download_path}/Ascend-cann-kernels-910_*.run --install >> ${install_log}
    # fi
}

download_install_pytorch_t () {
    echo "[Info][$(date "+%F %T")] ---------------------------- Install PyTorch T Version----------------------------"
    local download_path=$1
    local pytorch_version=$2
    local pytorch_b_version=$3
    local day_info=$4
    local arch_info=$5
    local user=$6
    local pwd=$7
    local pytorch_branch=$8
    if [ ! -d ${download_path} ]; then
        mkdir -p ${download_path}
    fi
    local version_info="${download_path}/version.txt"
    local install_log="${download_path}/install.log"

    # Download PyTorch
    artget search "FrameworkPTAdapter ${pytorch_version}" | grep "FrameworkPTAdapter ${pytorch_version}-${day_info}" | grep "ComponentVersion-Snapshot" > ${version_info}
    local build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    if [ -n ${build_version} ]; then
        new_day_info=$(date -d"yesterday ${day_info}" +%Y%m%d)
        artget search "FrameworkPTAdapter ${pytorch_version}" | grep "FrameworkPTAdapter ${pytorch_version}-${new_day_info}" | grep "ComponentVersion-Snapshot" > ${version_info}
        build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
        if [ ! -n ${build_version} ]; then
            return
        fi
    fi

    # Install torch
    declare -A torch_npu_map=(["pytorch_v1.11.0_py37"]="torch-1.11.0-cp37*linux_${arch_info}*.whl" ["pytorch_v2.1.0_py38"]="torch-2.1.0-cp38*${arch_info}*.whl")
    artget pull "FrameworkPTAdapter ${build_version}" -ru software -rp torch/${torch_npu_map[${pytorch_branch}]} -ap ${download_path} -user ${user} -pwd ${pwd} > ${install_log}
    pip3 install ${download_path}/${torch_npu_map[${pytorch_branch}]} --force-reinstall --no-deps

    # Install torch_npu
    artget pull "FrameworkPTAdapter ${build_version}" -ru software -rp ${pytorch_branch}.tar.gz -ap ${download_path} -user ${user} -pwd ${pwd} > ${install_log}
    pytorch_extract_dir=${download_path}/${pytorch_branch}
    mkdir -p ${pytorch_extract_dir}
    tar -xvf ${download_path}/${pytorch_branch}.tar.gz -C ${pytorch_extract_dir}
    pip3 install ${pytorch_extract_dir}/torch_npu-*${arch_info}.whl --force-reinstall
    pip3 install ${pytorch_extract_dir}/apex-0.1_ascend_*${arch_info}.whl --force-reinstall
}

download_install_pytorch_version () {
    echo "[Info][$(date "+%F %T")] ---------------------------- Install PyTorch version Version----------------------------"
    local download_path=$1
    local pytorch_version=$2
    local pytorch_b_version=$3
    local day_info=$4
    local arch_info=$5
    local user=$6
    local pwd=$7
    local pytorch_branch=$8
    if [ ! -d ${download_path} ]; then
        mkdir -p ${download_path}
    fi
    local version_info="${download_path}/version.txt"
    local install_log="${download_path}/install.log"
    # Download PyTorch
    artget search "FrameworkPTAdapter ${pytorch_version}" | grep "FrameworkPTAdapter ${pytorch_version}" | grep "ComponentVersion-Snapshot" > ${version_info}
    # local build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    # if [ -n ${build_version} ]; then
    #     new_day_info=$(date -d"yesterday ${day_info}" +%Y%m%d)
    #     artget search "FrameworkPTAdapter ${pytorch_version}" | grep "FrameworkPTAdapter ${pytorch_version}-${new_day_info}" | grep "ComponentVersion-Snapshot" > ${version_info}
    #     build_version=$(cat ${version_info} | head -1 | awk -F " " '{print $3}')
    #     if [ ! -n ${build_version} ]; then
    #         return
    #     fi
    # fi

    # Install torch
    declare -A torch_npu_map=(["pytorch_v1.11.0_py37"]="torch-1.11.0-cp37*linux_${arch_info}*.whl" ["pytorch_v2.1.0_py38"]="torch-2.1.0-cp38*${arch_info}*.whl")
    echo ${pytorch_version}
    echo ${pytorch_branch}
    artget pull "FrameworkPTAdapter ${pytorch_version}" -ru Inner -rp torch/${torch_npu_map[${pytorch_branch}]} -ap ${download_path} -user ${user} -pwd ${pwd} > ${install_log}
    pip3 install ${download_path}/${torch_npu_map[${pytorch_branch}]} --force-reinstall --no-deps

    # Install torch_npu
    artget pull "FrameworkPTAdapter ${pytorch_version}" -ru Inner -rp ${pytorch_branch}.tar.gz -ap ${download_path} -user ${user} -pwd ${pwd} > ${install_log}
    pytorch_extract_dir=${download_path}/${pytorch_branch}
    mkdir -p ${pytorch_extract_dir}
    tar -xvf ${download_path}/${pytorch_branch}.tar.gz -C ${pytorch_extract_dir}
    pip3 install ${pytorch_extract_dir}/torch_npu-*${arch_info}.whl --force-reinstall
    pip3 install ${pytorch_extract_dir}/apex-0.1_ascend_*${arch_info}.whl --force-reinstall
}

download_ptdbg_tools() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Download Ptdbg Tools ----------------------------"
    local download_path=$1
    local url=$2
    local branch=$3
    export http_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export https_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export no_proxy=127.0.0.1,.huawei.com,localhost,local,.local
    if [ -d ${download_path} ]; then
         rm -rf ${download_path}
    fi
    #git clone ${url} -b ${branch} ${download_path}

    #分pr
    if echo $url | grep -q "https://gitee.com/ascend";then
        flag=0
        echo "install ptdbg accroding original link"
        git clone ${url} -b ${branch} ${download_path}
    elif echo $url | grep -q "pull";then
        flag=1
        echo "install ptdbg accroding fetch method"
        git clone -b ${branch} https://gitee.com/ascend/att.git ${download_path}
    else
        echo "Input command is incorrect."
        exit 1
    fi

    if [ ! -d ${download_path}/debug/accuracy_tools/ptdbg_ascend ]; then
        echo "install ptdbg fail"
        exit 1
    fi

    if [ $flag -eq 1 ];then
        cd ${download_path}/debug/accuracy_tools
        echo $url
        git fetch https://gitee.com/ascend/att.git $url
        echo 'get pr_id'
        pr_id=`echo $url | awk -F "head:" '{print $2}'`
        echo $pr_id
        #git checkout $pr_id
        git merge $pr_id
    fi

}

install_ptdbg_tools() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Install Ptdbg Tools ----------------------------"
    local install_path=$1
    local conda_env=$2
    if [ ! -d ${install_path} ]; then
        echo "[ERROR] Failed to download ptdbg tools."
        exit 1
    fi
    cd ${install_path}/debug/accuracy_tools/ptdbg_ascend
    #cd ${install_path}/ptdbg_ascend
    export ADAPTER_TARGET_PYTHON_PATH=/root/miniforge3/envs/${conda_env}/bin/python3
    bash ./configure
    mkdir build
    cd build
    cmake ..
    make -j64
    make install
    if [ ! -f ./ptdbg_ascend/dist/ptdbg_ascend-*-py3-none-any.whl ]; then
        echo "[ERROR] The ptdbg tool fails to be compiled."
        exit 1
    fi
    echo "Y" | pip3 uninstall ptdbg-ascend
    pip3 install ./ptdbg_ascend/dist/ptdbg_ascend-*-py3-none-any.whl --upgrade --force-reinstall --no-deps
}

download_install_personal_msfmktransplt() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Install msfmktransplt Tools ----------------------------"
    local download_path=$1/ms_fmk_transplt
    local git_url=$2
    local git_branch=$3
    if [ -d ${download_path} ]; then
        rm -rf ${download_path}
    fi
    export http_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export https_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export no_proxy=127.0.0.1,.huawei.com,localhost,local,.local
    git clone ${git_url} -b ${git_branch} ${download_path}
    if [ -d ${download_path} ]; then
        local tools_path="/home/jct/Ascend2/ascend-toolkit/latest/tools/ms_fmk_transplt"
        local real_path=$(readlink "${tools_path}")
        local link_path=${tools_path}/../${real_path}
        local abs_path=$(cd ${link_path}; pwd)
        rm -rf ${abs_path}
        cp -r ${download_path}/src/ms_fmk_transplt ${abs_path}
    fi
}

download_install_personal_pytorch() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Install Personal PyTorch ----------------------------"
    local download_path=$1/pytorch_personal
    local git_url=$2
    local git_branch=$3
    local torch_version=$4
    if [ -d ${download_path} ]; then
        rm -rf ${download_path}
    fi
    export http_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export https_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export no_proxy=127.0.0.1,.huawei.com,localhost,local,.local
    git clone ${git_url} -b ${git_branch} ${download_path}
    if [ -d ${download_path} ]; then
        cd ${download_path}
        if [ "${torch_version}" == "pytorch_v2.1.0_py38" ]; then
            bash ci/build.sh --python=3.8
        else
            bash ci/build.sh --python=3.7
        fi
        if [ ! -f ${download_path}/dist/*.whl ]; then
            exit 1
        fi
        pip3 install ${download_path}/dist/*.whl --force-reinstall
    else
        exit 1
    fi
}

download_api_accuracy_checking_tools() {
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
    #git clone ${url} -b ${branch} ${download_path}

    #分pr
    if echo $url | grep -q "https://gitee.com/ascend";then
        flag=0
        echo "install api_accuracy_checker accroding original link"
        git clone ${url} -b ${branch} ${download_path}
    elif echo $url | grep -q "pull";then
        flag=1
        echo "install api_accuracy_checker accroding fetch method"
        git clone -b ${branch} https://gitee.com/ascend/att.git ${download_path}
    elif echo $url | grep -q "att_release";then
        flag=2
        echo "install api_accuracy_checker in att_release"
        git clone https://gitee.com/jiangchangting1/att_release.git ${download_path}
    else
        echo "Input command is incorrect."
        exit 1
    fi

    if [ ! -d ${download_path}/debug/accuracy_tools/api_accuracy_checker ]; then
        echo "install api_accuracy_checker fail"
        exit 1
    fi

    if [ $flag -eq 1 ];then
        cd ${download_path}/debug/accuracy_tools
        echo $url
        git fetch https://gitee.com/ascend/att.git $url
        echo 'get pr_id'
        pr_id=`echo $url | awk -F "head:" '{print $2}'`
        echo $pr_id
        #git checkout $pr_id
        git merge $pr_id
    fi
    
}

download_grad_tool_tools() {
    echo "[Info][$(date "+%F %T")] ---------------------------- Download grad_tool Tools ----------------------------"
    local download_path=$1
    local url=$2
    local branch=$3
    export http_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export https_proxy="http://p_atlas:proxy%40123@proxy.huawei.com:8080"
    export no_proxy=127.0.0.1,.huawei.com,localhost,local,.local
    if [ -d ${download_path} ]; then
         rm -rf ${download_path}
    fi
    #git clone ${url} -b ${branch} ${download_path}

    #分pr
    if echo $url | grep -q "https://gitee.com/ascend";then
        flag=0
        echo "install grad_tool accroding original link"
        git clone ${url} -b ${branch} ${download_path}
    elif echo $url | grep -q "pull";then
        flag=1
        echo "install grad_tool accroding fetch method"
        git clone -b ${branch} https://gitee.com/ascend/att.git ${download_path}
    elif echo $url | grep -q "att_release";then
        flag=2
        echo "install grad_tool in att_release"
        git clone https://gitee.com/jiangchangting1/att_release.git ${download_path}
    else
        echo "Input command is incorrect."
        exit 1
    fi

    if [ ! -d ${download_path}/debug/accuracy_tools/grad_tool ]; then
        echo "install grad_tool fail"
        exit 1
    fi

    if [ $flag -eq 1 ];then
        cd ${download_path}/debug/accuracy_tools
        echo $url
        git fetch https://gitee.com/ascend/att.git $url
        echo 'get pr_id'
        pr_id=`echo $url | awk -F "head:" '{print $2}'`
        echo $pr_id
        #git checkout $pr_id
        git merge $pr_id
    fi
    
}

filter_and_print() {
    success_info=()
    failed_info=()
    while read line; do
        if [[ $line =~ "fail" ]]; then
            failed_info+=("${line}")
        elif [[ $line =~ "pass" ]]; then
            success_info+=("${line}")
        fi
    done < $1
    for success_entry in "${success_info[@]}"; do
        echo $success_entry
    done
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    for failed_entry in "${failed_info[@]}"; do
        echo $failed_entry
    done
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    if [ -n "$failed_info" ]; then
        echo "att smoke not pass. Please locate the reason."
        exit 1
    fi
}

zip_and_print() {
    local output=$1
    time_info=$(date "+%Y%m%d-%H%M%S")
    local zip_output="$(dirname ${output})/output_${time_info}.tar.gz"
    tar -cvf ${zip_output} ${output}
    echo "echo \"Hz!*St-119.112\" | scp -r root@10.175.119.112:${zip_output} ."
}
