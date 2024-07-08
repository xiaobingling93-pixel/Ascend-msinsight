/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const {ModuleFederationPlugin} = require('webpack').container;
module.exports = {
  devServer: {
    port: 8000,
    open: false,
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
          './style/index': './src/style/index.css',
          './CommonUtils': './src/utils/Common.tsx',
          './Connector': './src/connection/index',
          './useDraggableContainer': './src/useDraggableContainer/useDraggableContainer.tsx',
          './i18n': './src/i18n/index.ts',
          './SharedConfigProvider': './src/SharedConfigProvider/index.tsx',
          './ColumnFilter': './src/resize/ColumnFilter.tsx',
          './ThemeStore': './src/utils/ThemeStore.ts',
          './theme': './src/theme/index.ts',
          './Icon': './src/icon/Icon.tsx',
        },
        shared: {
          react: {singleton: true},
          'react-dom': {singleton: true},
          '@cloudsop/horizon': {singleton: true},
          i18next: {singleton: true},
          'react-i18next': {singleton: true},
          antd: {singleton: true},
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
