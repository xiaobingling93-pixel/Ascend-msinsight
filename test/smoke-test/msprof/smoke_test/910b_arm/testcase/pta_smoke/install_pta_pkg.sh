user="open_mock"
pwd="open_mock"
# 运行机器是UTC时区，时间可能滞后一天
day=$(date -d "tomorrow" +%Y%m%d)
pkg_path=/home/temp
os=aarch64
version=$1
pta_path=/home/temp/pta_${version}

function downloadPta() {
  mkdir pta_path
  count=1
  while [ $count -le 10 ]
  do
    echo "try to get PTA ${day}"
    artget pull "FrameworkPTAdapter 7.3.0" -ru software -user ${user} -pwd ${pwd} -rp "${day}-07/Pytorch/pytorch_v${version}_py310.tar.gz" -ap "${pta_path}"
    if [ $? -ne 0 ]; then
      day=`date -d"yesterday ${day}" +%Y%m%d`
      count=$((count+1))
      continue
    fi
    break
  done
}

function installPta() {
  cd ${pta_path}
  tar -xvf ${pta_path}/pytorch_v${version}_py310.tar.gz
  source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta
  pip install --force-reinstall torch_npu-*${os}.whl
}

downloadPta
installPta
