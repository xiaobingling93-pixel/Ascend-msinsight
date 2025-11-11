/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const {webpackCfg} = require('../build-config');
const path = require('path');

const libPath = path.resolve(__dirname, '../lib/src');
const echartsPath = require.resolve('echarts');

module.exports = {
    devServer: {
        port: 3004,
        open: false,
    },
    webpack: {
        configure: webpackConfig => webpackCfg.computeConfigure(webpackConfig, [libPath, echartsPath]),
        alias: webpackCfg.alias,
    },
};
