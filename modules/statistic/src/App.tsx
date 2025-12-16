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
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from '@insight/lib';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import CurvePage from './pages/CurvePage';
import { GlobalStyles } from '@insight/lib/theme';
import { getInitStatus, registerEventHandlers } from './index';

export const App = observer(() => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');
    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        registerEventHandlers();
        getInitStatus();
    }, []);

    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                {session !== undefined ? <CurvePage session={session} isDark={themeInstance.getCurrentTheme() === 'dark'} /> : <></>}
            </SharedConfigProvider>
        </ThemeProvider>
    );
});
