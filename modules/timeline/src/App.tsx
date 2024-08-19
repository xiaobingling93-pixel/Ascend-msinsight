/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { ThemeProvider } from '@emotion/react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { AppErrorBoundary } from './components/error/AppErrorBoundary';
import { SessionPageErrorBoundary } from './components/error/SessionPageErrorBoundary';
import { useRootStore } from './context/context';
import { SessionPage } from './pages/SessionPage';
import { platform } from './platforms';
import { themeInstance, GlobalStyles } from 'ascend-theme';
import type { ThemeItem } from 'ascend-theme';
import eventBus, { EventType } from './utils/eventBus';
import connector from './connection';

const Window = styled.div`
    height: 100vh;
    overflow: hidden;
    display: flex;
    width: 100vw;
    color: ${(props): string => props.theme.fontColor};
`;

// 全局新增监听搜索快捷键输入
document.addEventListener('keydown', (e) => {
    if (e.ctrlKey && (e.key === 'f' || e.key === 'F')) {
        eventBus.emit(EventType.GLOBALSEARCH);
    }
});

const ImgWithFallback = ({
    className = '',
}): JSX.Element => {
    const PictureContainer = styled.picture`
        img {
            height: 48px;
            width: 48px;
        }
    `;
    return (
        <PictureContainer>
            <div className={className}></div>
        </PictureContainer>
    );
};

const StatePopover = observer(() => {
    const Mask = styled.div`
        position: absolute;
        display: flex;
        z-index: 4;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        width: 100%;
        height: 100%;
        justify-content: center;
        align-items: center;
        background-color: ${(props): string => props.theme.maskColor};
    `;
    const Info = styled.div`
        border-radius: 4px;
        position: relative;
        display: flex;
        align-items: center;
        width: 16rem;
        height: 3rem;
        font-size: 1.12rem;
        .img {
            margin-right: 10px;
        }
    `;
    return <Mask>
        <Info className={'info'}><ImgWithFallback className={'loading'}/>waiting...</Info>
    </Mask>;
});

export const App = observer(() => {
    const { insightStore, sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');
    const theme = themeInstance.getThemeType();

    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        insightStore.loadTemplates().then(() => {
            session = sessionStore.activeSession;
        });
        platform.initTheme().then((res: ThemeItem) => {
            window.setTheme(res === 'dark');
        });
        getLanguage();
    }, []);

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

    return (
        <ThemeProvider theme={theme}>
            <GlobalStyles />
            <Window>
                <AppErrorBoundary>
                    <SessionPageErrorBoundary>
                        <SharedConfigProvider locale={locale}>
                            {session !== undefined ? <SessionPage session={session} /> : <StatePopover/>}
                        </SharedConfigProvider>
                    </SessionPageErrorBoundary>
                </AppErrorBoundary>
            </Window>
        </ThemeProvider>
    );
});
