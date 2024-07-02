/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');
module.exports = {
  devServer: {
    port: 3004,
    open: false,
  },
  webpack: {
    configure: webpackConfig => webpackCfg.computeConfigure(webpackConfig),
    alias: webpackCfg.alias,
  },
};
