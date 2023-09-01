/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

module.exports = {
    webpack: {
        configure: (webpackConfig) => {
            // Because CEF has issues with loading source maps properly atm,
            // lets use the best we can get in line with `inline-source-map`
            webpackConfig.devtool = 'inline-source-map'
            return webpackConfig
        },
    },
};
