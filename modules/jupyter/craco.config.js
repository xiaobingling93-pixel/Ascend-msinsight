/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const {webpackCfg} = require('../build-config');

module.exports = {
  webpack: {
    configure: (webpackConfig) => webpackCfg.jupyterConfigure(webpackConfig),
    alias: webpackCfg.alias,
  },
};
