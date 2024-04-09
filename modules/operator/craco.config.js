/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');
module.exports = {
  webpack: {
    configure: webpackConfig => webpackCfg.operatorConfigure(webpackConfig),
    alias: webpackCfg.alias,
  },
};
