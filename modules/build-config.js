/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
const HTMLInlineCSSWebpackPlugin = require('html-inline-css-webpack-plugin').default;
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const fs = require('fs');
const path = require('path');
const {ModuleFederationPlugin} = require('webpack').container;

class ScriptTypePlugin {
  apply(compiler) {
    compiler.hooks.compilation.tap('ScriptTypePlugin', (compilation) => {
      HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tapAsync(
        'ScriptTypePlugin',
        (data, cb) => {
          data.html = data.html.replace(
            /<script\b[^<]*<\/script>/gi,
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

const federationConfig = {
  filename: 'remoteEntry.js',
  name: 'host',
  remotes: {lib: process.env.NODE_ENV === 'development' ? 'lib@http://localhost:8000/remoteEntry.js' : 'lib@../lib/remoteEntry.js'},
  shared: {
    react: {singleton: true, eager: true},
    'react-dom': {singleton: true, eager: true},
    '@cloudsop/horizon': {singleton: true, eager: true},
    i18next: {singleton: true},
    'react-i18next': {singleton: true},
    antd: {singleton: true},
    '@emotion/react': {singleton: true},
    '@emotion/styled': {singleton: true},
  },
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
  }), new HtmlInlineScriptPlugin(), new ScriptTypePlugin(), new BackgroundSvgInlinePlugin('cluster'),
  new ModuleFederationPlugin(federationConfig),
];

const webpackCfg = {
  timelineConfigure: (webpackConfig) => {
    // Because CEF has issues with loading source maps properly atm,
    // lets use the best we can get in line with `inline-source-map`
    webpackConfig.devtool = 'inline-source-map';
    webpackConfig.plugins.push(...[new HtmlInlineScriptPlugin(), new ScriptTypePlugin(), new BackgroundSvgInlinePlugin('timeline'),
      new ModuleFederationPlugin(federationConfig)]);
    return webpackConfig;
  },
  clusterConfigure: (webpackConfig) => {
    webpackConfig.entry = {
      main: webpackConfig.entry,
      summary: './src/SummaryIndex.ts',
      communication: './src/CommunicationIndex.ts',
    };
    webpackConfig.output.filename = 'static/js/[name].bundle.js';
    webpackConfig.plugins.splice(0, 1);
    webpackConfig.plugins.push(...htmllist);
    return webpackConfig;
  },
  memoryConfigure: (webpackConfig) => {
    webpackConfig.plugins.push(...[new HtmlInlineScriptPlugin(), new ScriptTypePlugin(), new ModuleFederationPlugin(federationConfig)]);
    return webpackConfig;
  },
  operatorConfigure: (webpackConfig) => {
    webpackConfig.plugins.push(new ModuleFederationPlugin(federationConfig));
    return webpackConfig;
  },
  jupyterConfigure: (webpackConfig) => {
    webpackConfig.plugins.push(new ModuleFederationPlugin(federationConfig));
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
    webpackConfig.plugins.push(new ModuleFederationPlugin(federationConfig));
    return webpackConfig;
  },
  alias: {
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
