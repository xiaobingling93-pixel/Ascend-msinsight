/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { type ReactNode } from 'react';
import { ConfigProvider } from 'antd';
import zhCN from 'antd/es/locale/zh_CN';
import enUS from 'antd/es/locale/en_US';
import { light } from '../theme/light';

interface SharedConfigProviderProps {
    locale: 'zhCN' | 'enUS';
    children: ReactNode;
}

const locales = {
    zhCN,
    enUS,
};

ConfigProvider.config({
    theme: {
        primaryColor: light.primaryColor,
        errorColor: light.dangerColor,
        warningColor: light.warningColor,
        successColor: light.successColor,
        infoColor: light.infoColor,
    },
});
export const SharedConfigProvider: React.FC<SharedConfigProviderProps> = ({ locale, children }) => (
    <ConfigProvider locale={locales[locale]}>
        {children}
    </ConfigProvider>
);
