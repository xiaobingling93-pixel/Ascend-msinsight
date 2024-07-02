/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');

module.exports = {
  devServer: {
    port: 3001,
    open: false,
  },
  webpack: {
    configure: (webpackConfig) => webpackCfg.memoryConfigure(webpackConfig),
    alias: webpackCfg.alias,
  },
};
