#!/bin/bash

snapshot_version=$1
command_type=$2
msprof_version=$3
smokeScriptUrl=$4
updateSmokeScript=$5
parsePerformance=$6
source env.sh

function CheckSlaveStatus()
{
    ssh-keygen -f "/root/.ssh/known_hosts" -R ${host_ip} | tee -a ${run_log}
    local NUM=0
    while true
    do
        if [ "$NUM" -gt "40" ];then
            date_time=`date +%Y%m%d`"."`date +%H%M%S`
            echo "####################################################################" | tee -a ${run_log}
            echo "$date_time ERROR : HOST restart Fail, please check" | tee -a ${run_log}
            echo "####################################################################" | tee -a ${run_log}
            exit 1
        fi
        PING=`ping -c 3 ${host_ip} | grep '3 received' | wc -l`
        if [ "$PING" -eq "0" ];then
            echo "ping ${host_ip} Fail, continue." | tee -a ${run_log}
            let NUM++
            continue
        fi
        echo "ping ${host_ip} Success, try to connect." | tee -a ${run_log}
        ./auto_ssh.sh "touch test.txt" ${host_ip} ${host_user} ${host_pwd} 
        if [ "$?" -eq "0" ];then
            return 0
        fi
        let NUM++
        sleep 5
    done
    return 1
}

function ClearRunPath()
{
    if [ "${snapshot_version}" == "master" ] || [[ "${snapshot_version}" =~ "CANN" ]];then
        ./auto_ssh.sh "mkdir -p ${pkg_path}; rm -f ${pkg_path}/*.*" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    else
        ./auto_ssh.sh "mkdir -p ${pkg_path}; rm -f ${pkg_path}/*-msprof_*" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    fi
    ./auto_ssh.sh "cd /home/result_dir/; rm -rf test_* " ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
}

function UninstallRun()
{
    if [ "${snapshot_version}" == "master" ] || [[ "${snapshot_version}" =~ "CANN" ]];then
        ./auto_ssh.sh "cd /usr/local/Ascend; rm -rf ascend-toolkit tfplugin " ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    fi
}

function GetVersionUrl()
{
    python get_url.py
    if [ $? -ne 0 ];then
        echo "newest url not found" | tee -a ${run_log}
    else
        hisi_version=`cat ${version} | head -1`
        hisi_path=`cat ${version} | tail -1`
        pkg=(Ascend${form}-driver-${hisi_version}-${pkg_system}.${os_arch}.run
     	     Ascend${form}-firmware-${hisi_version}.run)
            # cloud_asic_ubuntu_docker_aarch64.tar.gz)
    fi
}

function DownloadPkg()
{
    GetVersionUrl
    if [ $? -ne 0 ];then
        return 1
    fi
    export http_proxy=
    export https_proxy=
    for package in ${pkg[@]};do
        echo -e "\e[33m=================寮€濮嬩笅杞?{package}================= \e[m"
        download_path="http://hdfs-ngx0.turing-ci.hisilicon.com:14000/webhdfs/v1${hisi_path}/${package}?op=OPEN&user.name=balong"
        ./auto_ssh.sh "wget '${download_path}' -O ${pkg_path}/${package} > /dev/null 2>&1" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    done
    ./auto_ssh.sh "ls -l ${pkg_path}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
}

function InstallPkg()
{
    ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./*-firmware-* --upgrade --quiet" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    grep "ERROR" ${run_log}
    if [ $? -eq 0 ];then
        echo "firmware install Fail" | tee ${run_log}
        ./auto_ssh.sh "bash /usr/local/Ascend/driver/script/uninstall.sh" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        ./auto_ssh.sh "bash /usr/local/Ascend/firmware/script/uninstall.sh" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        RebootMachine
        CheckSlaveStatus
        ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./*-driver-* --full --quiet" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./*-firmware-* --full --quiet" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        grep "ERROR" ${run_log}
        if [ $? -eq 0 ];then
            echo "install Fail" | tee -a ${run_log}
            return 1
        fi
    else
        ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./*-driver-* --upgrade --quiet" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        grep "ERROR" ${run_log}
        if [ $? -eq 0 ];then
            echo "driver install Fail" | tee -a ${run_log}
            return 1
        fi
    fi
    RebootMachine
    CheckSlaveStatus
    return 0
}

function RebootMachine()
{
    if [ "${bmc_status}" == "on" ];then
        RebootDev
    fi
    ./auto_ssh.sh "reboot" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    sleep 30
}

function RebootDev()
{
    local ctime=0
    while true
    do
        if [ ${ctime} -ge 30 ];then
            echo "login root is timeout" | tee -a ${run_log}
            return 1
        fi
        (echo "";
        sleep 2; echo "${host_user}"; echo "";
        sleep 2; echo "${bmc_pwd}"; echo "";
        sleep 2; echo "${bmc_pwd}"; echo "";
        sleep 2) | telnet ${bmc_ip} ${bmc_port} | tee -a ${run_log}
        tail -10 ${run_log} | grep "# Huawei12#"
        if [ 0 -ne $? ];then
            echo "login root is Fail"
            let ctime++
            continue
        else
            (sleep 2;echo "ls";echo " ";
            sleep 2;echo "devmem 0x50000008 8 0xff";echo " ";
            sleep 2;echo "devmem 0x50000008 8 0x00";echo " ";
            sleep 2;echo "devmem 0x50000008 8 0xff";echo " ";sleep 2) | telnet ${bmc_ip} ${bmc_port} | tee -a ${run_log}
            sleep 120
            local devmemNum=`grep devmem ${run_log} | grep Hi |wc -l`
            if [ "${devmemNum}" -ge "3" ];then
                echo "reboot device is Success" | tee -a ${run_log}
                return 0
            else
                echo "reboot device is Fail" | tee -a ${run_log}
                return 1
            fi
        fi
    done
}

function CheckDevStatus()
{
    local num=0
    while true
    do
        if [ $num -gt 10 ];then
            echo "ERROR: Dev not connect, please check." | tee -a ${run_log}
            return 1
        fi
        ./auto_ssh.sh "/usr/local/Ascend/driver/tools/upgrade-tool --device_index -1 --component -1 --version; /usr/local/Ascend/driver/tools/upgrade-tool --device_index -1 --status" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        grep "count failed" ${run_log}
        if [ $? -eq 0 ];then
            echo "device is Fail" | tee -a ${run_log}
            let num++
            InstallPkg
            continue
        fi
        # ./auto_scp.sh restart_docker.sh ${host_ip} ${pkg_path} ${host_user} ${host_pwd}
        # ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./restart_docker.sh " ${host_ip} ${host_user} ${host_pwd}
        # if [ $? -eq 1 ];then
        #     echo "docker install Fail" | tee -a ${run_log}
        #     return 1
        # fi
        return 0
    done
    return 1
}

function DownloadCannPkg()
{
    day=`date +%Y%m%d`
    CANN_VERSION="7.0.RC1.B105"
    count=1
    while [ $count -le 10 ]
    do
        curl -k "https://cmc-szver-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-snapshot/CANN/CANN%206/" | grep "${CANN_VERSION}-${day}" | grep "onclick" > ${version}
        local cann_version=$(cat ${version} | awk -F ">" '{print $2}' | awk -F "CANN " '{print $2}' | awk -F "/" '{print $1}' | head -1)
        if [[ ${cann_version} == "" ]];then
            curl -k "https://dgg.artifactory.cd-cloud-artifact.tools.huawei.com/artifactory/sz-software-snapshot/cann/snapshot/CANN/" | grep "${CANN_VERSION}-${day}" | grep "onclick" > ${version}
            local cann_version=$(cat ${version} | awk -F ">" '{print $2}' | awk -F "/" '{print $1}' | head -1)
        fi
        if [[ "${snapshot_version}" =~ "CANN" ]];then
            local cann_version=$(echo ${snapshot_version} | awk -F "CANN-" '{print $2}')
        fi
        ./auto_ssh.sh "artget pull \"CANN ${cann_version}\" -ru software -rp run/${os_arch}-linux -ap ${pkg_path} -user ${user} -pwd ${pwd} -o CANN" ${host_ip} ${host_user} ${host_pwd} | tee ${run_log}
	./auto_ssh.sh "artget pull \"CANN ${cann_version}\" -ru software -rp run/Ascend-cann-kernels-${form}_* -ap ${pkg_path} -user ${user} -pwd ${pwd} -o CANN" ${host_ip} ${host_user} ${host_pwd} | tee ${run_log}
        grep "ERROR\|failed" ${run_log}
        if [ $? -eq 0 ];then
            day=`date -d"yesterday ${day}" +%Y%m%d`
	    count=$((count+1))
	    echo "---------------${day}------------------" | tee -a ${run_log}
            continue
        fi
        rm ${version}
        break
    done
    ./auto_ssh.sh "ls -l ${pkg_path}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
}

function DownloadMsprofPkg()
{
    ./auto_ssh.sh "artget pull \"msprof ${snapshot_version}\" -ru software -ap ${pkg_path} -user ${user} -pwd ${pwd}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    ./auto_ssh.sh "ls -l ${pkg_path}/" ${host_ip} ${host_user} ${host_pwd}
}

function InstallOtherPkg()
{
    export MSPROF_VERSION="master"
    if [ "${snapshot_version}" == "master" ] || [[ "${snapshot_version}" =~ "CANN" ]];then
        ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; echo "Y" | ./*tfplugin*.run --install --quiet --force" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        ./auto_ssh.sh "cd ${pkg_path}; chmod +x *; echo "Y" | ./*toolkit*.run --install --quiet --force" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
	./auto_ssh.sh "cd ${pkg_path}; chmod +x *; echo "Y" | ./*kernels*.run --install" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    else
	local msprof
	if [[ ${msprof_version} == "tr5" ]] || [[ ${msprof_version} == "TR5" ]];then
	    export MSPROF_VERSION=${msprof_version}
	    msprof="6.0"
        else
	    rm -r Manifest; git clone ssh://git@codehub-dg-y.huawei.com:2222/mindstudio/mindstudio_ci/Manifest.git
	    msprof=`grep -rn "version=[0-9].[0-9].[A-Z]" Manifest/dependency/config.ini | awk -F ".B" '{print $1}' | awk -F "=" '{print $2}'`
        fi
	echo "msprof version: " ${MSPROF_VERSION}
	./auto_ssh.sh "cd ${pkg_path}; chmod +x *; ./*msprof_${msprof}*-${os_arch}.run --full --quiet " ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
	./auto_ssh.sh "cd /usr/local/Ascend/ascend-toolkit/latest/bin; ln -sf ../../tools/profiler/bin/msprof msprof " ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    fi
    grep "ERROR\|failed" ${run_log}
    if [ $? -eq 0 ];then
        return 1
    fi
    return 0
}

function CopyLogPROFData()
{
    if [[ "${snapshot_version}" =~ "CANN" ]];then
        local snapshot_version=$(echo ${snapshot_version} | awk -F "CANN-" '{print $2}')
    fi
    mkdir -p ./${form}_${os_arch}_${snapshot_version}/testcase_log
    rm -f ./${form}_${os_arch}_${snapshot_version}/testcase_log/*
    ./auto_ssh.sh "cd /home/; zip -qr result_dir.zip result_dir" ${host_ip} ${host_user} ${host_pwd}
    sshpass -p ${host_pwd} scp -r -q -o StrictHostKeyChecking=no ${host_user}@${host_ip}:/home/result_dir.zip ./${form}_${os_arch}_${snapshot_version}/testcase_log/
    ./auto_ssh.sh "rm /home/result_dir.zip" ${host_ip} ${host_user} ${host_pwd}
    zip -qr ${form}_${os_arch}_${snapshot_version}.zip ./${form}_${os_arch}_${snapshot_version}/*
    rm -r ./${form}_${os_arch}_${snapshot_version}
    ./auto_scp.sh ${form}_${os_arch}_${snapshot_version}.zip 7.212.125.27 "/home/hava/" root Huawei12#$
    rm -f ${form}_${os_arch}_${snapshot_version}.zip
}

function Main()
{
    rm ./${run_log} ./result.txt
    CheckSlaveStatus
    if [ $? -ne 0 ];then
        return 1
    fi
    ClearRunPath
    UninstallRun
    if [ "${snapshot_version}" == "master" ];then
        DownloadPkg
        InstallPkg
        CheckDevStatus
        if [ $? -ne 0 ];then
            return 1
        fi
        DownloadCannPkg
    elif [[ "${snapshot_version}" =~ "CANN" ]];then
        DownloadCannPkg
    else
        DownloadMsprofPkg
    fi
    InstallOtherPkg
    if [ $? -ne 0 ];then
        return 1
    fi
    ./auto_ssh.sh "cd /home/msprof_smoke_test/smoke_test/910_arm/testcase; bash run_all.sh ${command_type} ${msprof_version} ${parsePerformance}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    CopyLogPROFData
    sshpass -p ${host_pwd} scp -r -q -o StrictHostKeyChecking=no ${host_user}@${host_ip}:/home/msprof_smoke_test/smoke_test/910_arm/testcase/result.txt ./

    # 如果更新的不是master,还原到master的代码
    if [[ "${smokeScriptUrl}" == "ssh"* ]];then
        if ! [ "${smokeScriptUrl}" != "ssh://git@szv-y.codehub.huawei.com:2222/ascend_cmd_profiling/smoke_test.git" ];then
            ./auto_ssh.sh "cd /home/msprof_smoke_test; rm -rf smoke_test;" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
            ./auto_ssh.sh "cd /home/msprof_smoke_test; mv smoke_test_back smoke_test" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        fi
    fi

    grep "fail" result.txt
    if [ $? -ne 1 ];then
        return 1
    fi
    return 0
}

function UpdateSmokeScriptBySsh()
{
    # 如果更新的不是master,要先备份代码
    if ! [ "${smokeScriptUrl}" == "ssh://git@szv-y.codehub.huawei.com:2222/ascend_cmd_profiling/smoke_test.git" ];then
        ./auto_ssh.sh "cd /home/msprof_smoke_test; mv smoke_test smoke_test_back" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    fi

    ./auto_ssh.sh "cd /home/msprof_smoke_test; rm -rf smoke_test;" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    ./auto_ssh.sh "cd /home/msprof_smoke_test; git clone -b smoke_test_master ${smokeScriptUrl}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    file_dir=/home/msprof_smoke_test/smoke_test
    rm is_dir_exist.txt
    ./auto_ssh.sh "[ -d "${file_dir}" ] && echo "smokeTestScriptExist" || echo "smokeTestScriptNoneExist"" ${host_ip} ${host_user} ${host_pwd} | tee -a is_dir_exist.txt
    count=$(grep "smokeTestScriptNoneExist" is_dir_exist.txt | wc -l)
    if [ ${count} -eq 2 ]; then
        echo "[ERROR] Failed to download smoking script." | tee -a ${run_log}
            exit 1
    fi
}

function RunSmokeScript()
{
    export MSPROF_VERSION="master"
	if [[ ${msprof_version} == "tr5" ]] || [[ ${msprof_version} == "TR5" ]];then
        export MSPROF_VERSION=${msprof_version}
    fi
    ./auto_ssh.sh "cd /home/msprof_smoke_test/smoke_test/910_arm/testcase; bash run_all.sh ${command_type} ${MSPROF_VERSION} ${parsePerformance}" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    sshpass -p ${host_pwd} scp -r -q -o StrictHostKeyChecking=no ${host_user}@${host_ip}:/home/msprof_smoke_test/smoke_test/910_arm/testcase/result.txt ./

    # 如果更新的不是master,还原到master的代码
    if ! [ "${smokeScriptUrl}" != "ssh://git@szv-y.codehub.huawei.com:2222/ascend_cmd_profiling/smoke_test.git" ];then
        ./auto_ssh.sh "cd /home/msprof_smoke_test; rm -rf smoke_test;" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
        ./auto_ssh.sh "cd /home/msprof_smoke_test; mv smoke_test_back smoke_test" ${host_ip} ${host_user} ${host_pwd} | tee -a ${run_log}
    fi

    grep "fail" result.txt
    if [ $? -ne 1 ];then
        exit 1
    fi
    exit 0
}

if [ $# -lt 2 ];then
    echo "[ERROR] Please input three parameters."
    echo "such as: master or 1.0.0-20220627135132-430 and 5.0.RC3 \n"
    exit 1
else
    # smokeScriptUrl是一个ssh地址的时候就更新脚本
    if [[ "${smokeScriptUrl}" == "ssh"* ]];then
        UpdateSmokeScriptBySsh
        # updateSmokeScript选yes说明不更新cann包和msprof，只跑冒烟
        if [ "${updateSmokeScript}" == "yes" ];then
            RunSmokeScript "$@"
        fi
    fi
    Main "$@"
    if [ $? -ne 0 ];then
        exit 1
    fi
fi
exit 0

