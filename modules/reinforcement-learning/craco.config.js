/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
const {webpackCfg, configureConfig} = require('../build-config');
const path = require('path');

const libPath = path.resolve(__dirname, '../lib/src');
const echartsPath = require.resolve('echarts');

module.exports = {
  devServer: {
    port: 3008,
    open: false,
  },
  webpack: {
      alias: webpackCfg.alias,
      configure: (webpackConfig) => {
          return configureConfig(webpackConfig, [libPath, echartsPath]);
      }
  },
};
