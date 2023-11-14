/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
const HtmlWebpackPlugin = require('html-webpack-plugin')
const HTMLInlineCSSWebpackPlugin = require('html-inline-css-webpack-plugin').default;
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const fs= require('fs');
const path= require('path');

class ScriptTypePlugin {
  apply(compiler) {
    compiler.hooks.compilation.tap('ScriptTypePlugin', (compilation) => {
      HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tapAsync(
        'ScriptTypePlugin',
        (data, cb) => {
          data.html = data.html.replace(
            /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi,
            (match) => match.replace('<script', '<script type="module"')
          );
          cb(null, data);
        }
      );
    });
  }
}

class BackgroundSvgInlinePlugin {
  apply(compiler) {
    compiler.hooks.compilation.tap('BackgroundSvgInlinePlugin', (compilation) => {
      HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tapAsync(
        'BackgroundSvgInlinePlugin',
        (data, cb) => {
          data.html = data.html.replace(
            /(\.\.\/\.\.\/static\/media\/)(\w+)(\.\w+\.svg)/g,
            (m, $1, $2) => {
              const svgContent = fs.readFileSync(path.join(__dirname, 'src/assets/images', $2+'.svg'));
              return "'data:image/svg+xml;base64,"+svgContent.toString('base64')+"'";
            }
          );
          cb(null, data);
        }
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
  }), new HTMLInlineCSSWebpackPlugin(), new HtmlInlineScriptPlugin(), new ScriptTypePlugin(), new BackgroundSvgInlinePlugin(),
];

module.exports = {
  webpack: {
    configure: (webpackConfig) => {
      // Because CEF has issues with loading source maps properly atm,
      // lets use the best we can get in line with `inline-source-map`
      webpackConfig.entry = {
        main: webpackConfig.entry,
        index: './src/index.tsx',
        summary: './src/SummaryIndex.tsx',
        communication: './src/CommunicationIndex.tsx',
      };
      webpackConfig.output.filename = 'static/js/[name].bundle.js';
      webpackConfig.plugins.push(...htmllist);
      webpackConfig.devtool = 'inline-source-map';
      return webpackConfig;
    },
  },
};
