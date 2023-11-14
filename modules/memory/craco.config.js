/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
const HTMLInlineCSSWebpackPlugin = require('html-inline-css-webpack-plugin').default;
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');

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

module.exports = {
  webpack: {
    configure: (webpackConfig) => {
      // Because CEF has issues with loading source maps properly atm,
      // lets use the best we can get in line with `inline-source-map`
      webpackConfig.devtool = 'inline-source-map';
      webpackConfig.plugins.push(...[ new HTMLInlineCSSWebpackPlugin(), new HtmlInlineScriptPlugin(), new ScriptTypePlugin() ]);
      return webpackConfig
    },
  },
};
