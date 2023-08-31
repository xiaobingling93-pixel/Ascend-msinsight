#!/bin/bash
#Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
# ================================================================================
set -e

script=$(readlink -f "$0")
route=$(dirname "$script")

function main() {
  npm config set strict-ssl false
  npm config set audit false
  npm config set disturl http://mirrors.tools.huawei.com/nodejs
  npm config set registry https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/
  cd "${route}"/..
  npm run test
}

main
