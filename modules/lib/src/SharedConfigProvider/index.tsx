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
