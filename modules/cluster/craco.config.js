/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');

const path = require('path');

const libPath = path.resolve(__dirname, '../lib/src');
const echartsPath = require.resolve('echarts');

module.exports = {
  devServer: {
    port: 3003,
    open: false,
  },
  webpack: {
    configure: (webpackConfig) => webpackCfg.clusterConfigure(webpackConfig, [libPath, echartsPath]),
    alias: webpackCfg.alias,
  },
};
