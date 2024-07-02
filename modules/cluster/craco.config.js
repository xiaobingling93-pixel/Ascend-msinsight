/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');

module.exports = {
  devServer: {
    port: 3003,
    open: false,
  },
  webpack: {
    configure: (webpackConfig) => webpackCfg.clusterConfigure(webpackConfig),
    alias: webpackCfg.alias,
  },
};
