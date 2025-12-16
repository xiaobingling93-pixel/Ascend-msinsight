#!/bin/bash
# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------
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
