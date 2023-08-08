#!/bin/bash
#Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
# ================================================================================
set -e

script=$(readlink -f "$0")
route=$(dirname "$script")

function build() {
  export npm_config_build_from_source=true
  os=$(uname -s)
  if [ "${os:0:5}" == 'MINGW' ]; then
    echo 'start buildWin'
    npm run buildWin || exit 2
    echo 'buildWin end'
  elif [ "${os:0:5}" == 'Linux' ]; then
    echo 'start buildLinux'
    npm run buildLinux || exit 2
    echo 'buildLinux end'
  else
    echo "${os:0:5}"':os system is wrong' && exit 1
  fi
  rm -rf "${route}"/out
  mkdir "${route}"/out
  cp "${route}"/packages/extension/ascend-insight-extension-*.vsix "${route}"/out
}

function main() {
  build
}

main
