/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const HtmlWebpackPlugin = require('html-webpack-plugin');

const path = require('path');

const configureConfig = (webpackConfig, paths) => {
    const oneOfRule = webpackConfig.module.rules.find((r) => r.oneOf);
    if (oneOfRule) {
        const loaderRule = oneOfRule.oneOf.find(
            (r) =>
                r.test &&
                r.test.toString().includes('tsx') &&
                r.loader &&
                r.loader.includes('babel-loader')
        );
        if (loaderRule) {
            if (!Array.isArray(loaderRule.include)) loaderRule.include = [loaderRule.include];
            loaderRule.include.push(...paths, path.resolve(__dirname, '../lib/src'));
        }
    }
    return webpackConfig;
};

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
  clusterConfigure: (webpackConfig, paths) => {
    webpackConfig.entry = {
      main: webpackConfig.entry,
      summary: './src/SummaryIndex.tsx',
      communication: './src/CommunicationIndex.tsx',
    };
    webpackConfig.output.filename = 'static/js/[name].bundle.js';
    webpackConfig.plugins.push(...htmllist);
    return configureConfig(webpackConfig, paths);
  },
  computeConfigure: (webpackConfig, paths) => {
    webpackConfig.entry = {
      main: webpackConfig.entry,
      detail: './src/detailIndex.ts',
      source: './src/sourceIndex.ts',
      cache: './src/cacheKitIndex.ts',
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
      new HtmlWebpackPlugin({
        template: 'public/index.html',
        filename: 'cache.html',
        chunks: ['cache'],
      }),
    );
      return configureConfig(webpackConfig, paths);
  },
  alias: {
    '@': path.resolve('src'),
    react: '@cloudsop/horizon', // 新增
    'react-dom/client': '@cloudsop/horizon', // 兼容react18的用法
    'react-dom': '@cloudsop/horizon', // 新增
    'react-is': '@cloudsop/horizon', // 新增
  },
};

module.exports = { webpackCfg, configureConfig };
