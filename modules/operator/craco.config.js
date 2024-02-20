/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
const HtmlWebpackPlugin = require('html-webpack-plugin')
module.exports = {
  webpack: {
    configure: webpackConfig => webpackConfig,
    alias: {
      react: '@cloudsop/horizon', // 新增
      'react-dom/client': '@cloudsop/horizon', // 兼容react18的用法
      'react-dom': '@cloudsop/horizon', // 新增
      'react-is': '@cloudsop/horizon', // 新增
    },
  },
};
