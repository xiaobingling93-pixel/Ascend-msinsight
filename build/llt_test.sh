#!/bin/bash
#Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
# ================================================================================
set -e

script=$(readlink -f "$0")
route=$(dirname "$script")

function main() {
  export npm_config_build_from_source=true
  export npm_config_audit=false
  export npm_config_strict_ssl=false
  export npm_config_disturl=http://mirrors.tools.huawei.com/nodejs
  export npm_config_registry=https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/
  npm config set @cloudsop:registry=https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/product_npm
  cd "${route}"/..
  npm run test
}

main
