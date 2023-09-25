/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
const HtmlWebpackPlugin = require('html-webpack-plugin')

const htmllist = [
  new HtmlWebpackPlugin({
    template: 'public/index.html',
    filename: 'memory.html',
    chunks: ['memory'],
  }),
];

module.exports = {
    webpack: {
        configure: (webpackConfig) => {
            // Because CEF has issues with loading source maps properly atm,
            // lets use the best we can get in line with `inline-source-map`
            webpackConfig.entry = {
                main: webpackConfig.entry,
                index: './src/index.tsx',
            };
            webpackConfig.output.filename = 'static/js/[name].bundle.js';
            webpackConfig.devtool = 'inline-source-map';
            return webpackConfig;
        },
    },
};
