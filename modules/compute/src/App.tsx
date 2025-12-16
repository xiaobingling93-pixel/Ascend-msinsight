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
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { SharedConfigProvider } from '@insight/lib';
import { useRootStore } from './context/context';
import connector from './connection';
import HotMethod from './components/hotMethod/HotMethod';
import Detail from './components/detail/Index';
import { GlobalStyles, themeInstance } from '@insight/lib/theme';
import CacheKit from './components/cacheKit/Index';

const app = observer(({ page }: {page?: string}) => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');

    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        connector.send({ event: 'getParseStatus', body: { } });
        getLanguage();
    }, []);

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

    let dom;
    if (session === undefined) {
        dom = <></>;
    } else if (page === 'detail') {
        dom = <Detail session={session}/>;
    } else if (page === 'cacheKit') {
        dom = <CacheKit session={session}/>;
    } else {
        dom = <HotMethod session={session} />;
    }
    return <ThemeProvider theme={themeInstance.getThemeType()}>
        <GlobalStyles />
        <SharedConfigProvider locale={locale}>
            {dom}
        </SharedConfigProvider>
    </ThemeProvider>;
});

export default app;
