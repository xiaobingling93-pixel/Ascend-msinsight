/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
// @ts-check

'use strict';

const path = require('path');
const CopyWebpackPlugin = require('copy-webpack-plugin');

// @ts-check
/** @typedef {import('webpack').Configuration} WebpackConfig **/

/** @type WebpackConfig */
const extensionConfig = {
  // vscode extensions run in a Node.js-context 📖 -> https://webpack.js.org/configuration/node/
  target: 'node',
  // this leaves the source code as close as possible to the original (when packaging we set this to 'production')
  mode: 'none',
  // the entry point of this extension, 📖 -> https://webpack.js.org/configuration/entry-context/
  entry: './src/extension.ts',
  output: {
    // the bundle is stored in the 'dist' folder (check package.json), 📖 -> https://webpack.js.org/configuration/output/
    path: path.resolve(__dirname, 'dist'),
    filename: 'extension.js',
    libraryTarget: 'commonjs2'
  },
  externals: {
    // the vscodnpm install vsce -g --save vscee-module is created on-the-fly and must be excluded. Add other modules that cannot be webpack'ed, 📖 -> https://webpack.js.org/configuration/externals/
    vscode: 'commonjs vscode'
  },
  resolve: {
    // support reading TypeScript and JavaScript files, 📖 -> https://github.com/TypeStrong/ts-loader
    extensions: ['.ts', '.js']
  },
  module: {
    rules: [
      {
        test: /\.ts$/,
        exclude: /node_modules/, // exclude npm package
        use: [
          {
            loader: 'ts-loader'
          }
        ]
      }
    ]
  },
  devtool: 'nosources-source-map',
  infrastructureLogging: {
    level: "log",
  },
  plugins: [
    new CopyWebpackPlugin({
      patterns: [
        { from: '../../server/output/build', to: 'profiler' },
        { from: '../../modules/framework/build', to: 'profiler' },
      ],
    }),
  ],
};
module.exports = [extensionConfig];