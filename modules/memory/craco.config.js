/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
const {webpackCfg, configureConfig } = require('../build-config');

const path = require('path');

const libPath = path.resolve(__dirname, '../lib/src');
const echartsPath = require.resolve('echarts');

module.exports = {
    devServer: {
        port: 3001,
        open: false,
        client: {
            overlay: {
                runtimeErrors: (error) => {
                    // 禁止界面展示错误：ResizeObserver loop completed with undelivered notifications
                    return !(error?.message.includes('ResizeObserver'));
                },
            },
        },
    },
    webpack: {
        alias: webpackCfg.alias,
        configure: (webpackConfig) => {
            return configureConfig(webpackConfig, [libPath, echartsPath]);
        }
    },
};
