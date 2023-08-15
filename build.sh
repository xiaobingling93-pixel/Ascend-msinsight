#!/bin/bash
#Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
# ================================================================================
set -e

script=$(readlink -f "$0")
route=$(dirname "$script")

function buildInit() {
  vscodeVersion=6.0.3
  ideaVersion=6.0.RC3
  os=$(uname -s)
  rm -rf "${route}"/out
  mkdir "${route}"/out
}

function buildVscodePlugin() {
  export npm_config_build_from_source=true
  if [ "${os:0:5}" == 'MINGW' ]; then
    osName=win
    echo 'start buildWin'
    npm run buildWin || exit 2
    echo 'buildWin end'
  elif [ "${os:0:5}" == 'Linux' ]; then
    osName=linux-$(uname -m)
    echo 'start buildLinux'
    npm run buildLinux || exit 2
    echo 'buildLinux end'
  else
    echo "${os:0:5}"':os system is wrong' && exit 1
  fi
  cp "${route}"/packages/extension/ascend-insight-extension-*.vsix "${route}/out/ascend-insight-extension_${vscodeVersion}_${osName}.vsix"
}

function buildIdeaPlugin() {
  cd "${route}"/plugins
  echo "build plugin insight $(date +%Y-%m-%d_%H:%M:%S) task:buildPlugin"
  if [ 0"$GRADLE_URL" = "0" ]; then
    gradle wrapper
  else
    gradle wrapper --gradle-distribution-url "$GRADLE_URL"
  fi
  chmod a+x gradlew
  ./gradlew clean || {
    echo "gradlew clean ascend insight failed" && exit 2
  }
  ./gradlew buildPlugin || {
    echo "build ascend insight failed" && exit 3
  }
  cp "${route}"/plugins/build/distributions/ascend-insight-*.zip "${route}"/out/ascend-insight-plugin_"${ideaVersion}"_"${osName}".zip
}

function main() {
  buildInit
  buildVscodePlugin
  buildIdeaPlugin
}

main
