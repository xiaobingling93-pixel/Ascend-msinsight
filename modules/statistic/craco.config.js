/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
const {webpackCfg} = require('../build-config');

module.exports = {
  devServer: {
    port: 3006,
    open: false,
  },
  webpack: {
    alias: webpackCfg.alias,
  },
};
