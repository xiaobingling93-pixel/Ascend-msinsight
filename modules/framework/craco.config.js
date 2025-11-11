/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const {webpackCfg, configureConfig} = require('../build-config');
const path = require("path");

const libPath = path.resolve(__dirname, '../lib/src');
const echartsPath = require.resolve('echarts');

module.exports = {
  devServer: {
    port: 5174,
    open: false,
    historyApiFallback: false,
  },
  webpack: {
    configure: webpackConfig => {
      return configureConfig(webpackConfig, [libPath, echartsPath]);
    },
    alias: webpackCfg.alias,
  },
};
