/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
          './CollapsiblePanel': './src/CollapsiblePanel/index.tsx',
          './Layout': './src/components/Layout/index.tsx',
          './components': './src/components/index.ts',
        },
        shared: {
          react: {singleton: true},
          'react-dom': {singleton: true},
          '@cloudsop/horizon': {singleton: true},
          i18next: {singleton: true},
          'react-i18next': {singleton: true},
          '@emotion/react': {singleton: true},
          '@emotion/styled': {singleton: true},
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
