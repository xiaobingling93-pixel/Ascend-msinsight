/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { useRootStore } from './context/context';
import connector from './connection';
import HotMethod from './components/hotMethod/HotMethod';
import Detail from './components/detail/Index';
import { themeInstance } from './theme/theme';
import { GlobalStyles } from 'ascend-theme';

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
