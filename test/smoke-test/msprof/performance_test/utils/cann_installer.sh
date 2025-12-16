#!/bin/bash

get_device_msg() 
{
    res=$(lspci)

    if echo "$res" | grep -q "d100"; then
        echo "310"
    elif echo "$res" | grep -q "d500"; then
        echo "310p"
    elif echo "$res" | grep -q "d801"; then
        echo "910"
    elif echo "$res" | grep -q "d802"; then
        echo "910b"
    elif echo "$res" | grep -q "d107"; then
        echo "310b"
    fi
}

check_os_architecture() {
    local arch=$(uname -m)
    
    case "$arch" in
        "x86_64")
            echo "x86"
            ;;
        "aarch64")
            echo "aarch64"
            ;;
        *)
            echo "unknow"
            ;;
    esac
}


soc=$(get_device_msg)
os_arch=$(check_os_architecture)
pkg_path="/home/temp"
source env.sh

#!/bin/bash

autoAgree() {
    local targetCmd=$1
    yes | ${targetCmd}
}

# 定义下载函数
DownloadCannPkg() {
    rm -rf ${pkg_path}/*kernels-*.run ${pkg_path}/*toolkit*.run ${pkg_path}/*Atlas*.run
    local version=$1
    echo "Downloading CANN package version $version..."
    artget pull "CANN ${version}" -ru software -rp run/${os_arch}-linux/*kernels-${soc}_*.run -ap ${pkg_path} -user ${user} -pwd ${pwd} -o CANN
    artget pull "CANN ${version}" -ru software -rp run/${os_arch}-linux/*toolkit*.run -ap ${pkg_path} -user ${user} -pwd ${pwd} -o CANN
    artget pull "CANN ${version}" -ru software -rp run/${os_arch}-linux/*Atlas*.run -ap ${pkg_path} -user ${user} -pwd ${pwd} -o CANN
}

# 定义安装函数
InstallCannPkg() {
    local version=$1
    echo "Installing CANN package version $version..."
    chmod +x ${pkg_path}/*kernels*.run && autoAgree "${pkg_path}/*kernels*.run --install"
    chmod +x ${pkg_path}/*toolkit*.run && autoAgree "${pkg_path}/*toolkit*.run --install"
    chmod +x ${pkg_path}/*Atlas*.run && autoAgree "${pkg_path}/*Atlas*.run --install"
    echo "Installing CANN package end"
}

# 定义主函数
Main() {
    echo "current platform is $soc, os_arch is $os_arch"

    # 定义一个关联数组，将参数值映射到下载函数名
    declare -A download_functions
    download_functions["cann"]="DownloadCannPkg"

    # 定义一个关联数组，将参数值映射到安装函数名
    declare -A install_functions
    install_functions["cann"]="InstallCannPkg"

    # 检查是否提供了参数
    if [ $# -eq 0 ]; then
        echo "No package specified."
        return 1
    fi

    # 遍历所有参数
    for pkg_arg in "$@"; do
        # 分离包名和版本号
        IFS='-' read -r pkg_name version <<< "$pkg_arg"
        # 检查关联数组中是否有对应的下载函数
        if [[ ${download_functions[$pkg_name]+_} ]]; then
            # 调用对应的下载函数并传递版本号
            ${download_functions[$pkg_name]} $version &
            download_pids+=($!)
        else
            echo "Package '$pkg_name' not recognized."
        fi
    done

    # 等待所有下载进程完成
    for pid in "${download_pids[@]}"; do
        wait $pid
    done

    # 清空下载进程ID数组
    unset download_pids

    # 串行安装
    for pkg_arg in "$@"; do
        IFS='-' read -r pkg_name version <<< "$pkg_arg"
        # 检查关联数组中是否有对应的安装函数
        if [[ ${install_functions[$pkg_name]+_} ]]; then
            # 调用对应的安装函数并传递版本号
            ${install_functions[$pkg_name]} $version
        fi
    done
}

# 调用主函数并传递参数
Main "$@"