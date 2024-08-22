/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const HTMLInlineCSSWebpackPlugin = require('html-inline-css-webpack-plugin').default;
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const fs = require('fs');
const path = require('path');

class ScriptTypePlugin {
  apply(compiler) {
    compiler.hooks.compilation.tap('ScriptTypePlugin', (compilation) => {
      HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tapAsync(
        'ScriptTypePlugin',
        (data, cb) => {
          data.html = data.html.replace(
            /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi,
            (match) => match.replace('<script', '<script type="module"'),
          );
          cb(null, data);
        },
      );
    });
  }
}

class BackgroundSvgInlinePlugin {
  constructor(module) {
    this.moduleName = module;
  }

  apply(compiler) {
    compiler.hooks.compilation.tap('BackgroundSvgInlinePlugin', (compilation) => {
      HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tapAsync(
        'BackgroundSvgInlinePlugin',
        (data, cb) => {
          data.html = data.html.replace(
            /(?<svgPath>\.\.\/\.\.\/static\/media\/)(?<svgName>\w+)(?<svgCode>\.\w+\.svg)/g,
            (m, svgPath, svgName) => {
              const svgContent = fs.readFileSync(path.join(__dirname, `${this.moduleName}/src/assets/images`, `${svgName}.svg`));
              return `'data:image/svg+xml;base64,${svgContent.toString('base64')}'`;
            },
          );
          cb(null, data);
        },
      );
    });
  }
}

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
  timelineConfigure: (webpackConfig) => {
    // Because CEF has issues with loading source maps properly atm,
    // lets use the best we can get in line with `inline-source-map`
    webpackConfig.devtool = 'inline-source-map';
    return webpackConfig;
  },
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
  memoryConfigure: (webpackConfig) => {
    return webpackConfig;
  },
  operatorConfigure: (webpackConfig) => {
    return webpackConfig;
  },
  jupyterConfigure: (webpackConfig) => {
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
    '@': path.resolve(__dirname, 'src'),
    react: '@cloudsop/horizon', // 新增
    'react-dom/client': '@cloudsop/horizon', // 兼容react18的用法
    'react-dom': '@cloudsop/horizon', // 新增
    'react-is': '@cloudsop/horizon', // 新增
  },
};

module.exports = {
  ScriptTypePlugin,
  BackgroundSvgInlinePlugin,
  HTMLInlineCSSWebpackPlugin,
  HtmlInlineScriptPlugin,
  webpackCfg,
};
