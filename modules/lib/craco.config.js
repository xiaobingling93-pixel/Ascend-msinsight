/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const {ModuleFederationPlugin} = require('webpack').container;
module.exports = {
  devServer: {
    port: 8000,
  },
  webpack: {
    configure: webpackConfig => {
      webpackConfig.plugins.push(new ModuleFederationPlugin({
        filename: 'remoteEntry.js',
        name: 'lib',
        exposes: {
          './ResizeTable': './src/resize/ResizeTable.tsx',
          './Simple': './src/Simple',
          './Resizor': './src/resize/Resizor.tsx',
          './style/color': './src/style/color.css',
          './CommonUtils': './src/utils/Common.tsx',
          './Connector': './src/connection/index',
        },
        shared: {
          react: {singleton: true},
          'react-dom': {singleton: true},
          '@cloudsop/horizon': {singleton: true},
        },
      }));
      webpackConfig.output.publicPath = 'auto';
      return webpackConfig;
    },
    alias: {
      react: '@cloudsop/horizon', // 新增
      'react-dom/client': '@cloudsop/horizon', // 兼容react18的用法
      'react-dom': '@cloudsop/horizon', // 新增
      'react-is': '@cloudsop/horizon', // 新增
    },
  },
};
