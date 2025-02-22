/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const HtmlWebpackPlugin = require('html-webpack-plugin');

const path = require('path');

const htmllist = [
  new HtmlWebpackPlugin({
    template: 'public/index.html',
    filename: 'summary.html',
    chunks: ['summary'],
  }),
  new HtmlWebpackPlugin({
    template: 'public/index.html',
    filename: 'communication.html',
    chunks: ['communication'],
  }),
];

const webpackCfg = {
  clusterConfigure: (webpackConfig) => {
    webpackConfig.entry = {
      main: webpackConfig.entry,
      summary: './src/SummaryIndex.ts',
      communication: './src/CommunicationIndex.ts',
    };
    webpackConfig.output.filename = 'static/js/[name].bundle.js';
    webpackConfig.plugins.push(...htmllist);
    return webpackConfig;
  },
  computeConfigure: (webpackConfig) => {
    webpackConfig.entry = {
      main: webpackConfig.entry,
      detail: './src/detailIndex.ts',
      source: './src/sourceIndex.ts',
    };
    webpackConfig.output.filename = 'static/js/[name].bundle.js';
    webpackConfig.plugins.push(
      new HtmlWebpackPlugin({
        template: 'public/index.html',
        filename: 'detail.html',
        chunks: ['detail'],
      }),
      new HtmlWebpackPlugin({
        template: 'public/index.html',
        filename: 'source.html',
        chunks: ['source'],
      }),
    );
    return webpackConfig;
  },
  alias: {
    '@': path.resolve('src'),
    react: '@cloudsop/horizon', // 新增
    'react-dom/client': '@cloudsop/horizon', // 兼容react18的用法
    'react-dom': '@cloudsop/horizon', // 新增
    'react-is': '@cloudsop/horizon', // 新增
  },
};

module.exports = {
  webpackCfg,
};
