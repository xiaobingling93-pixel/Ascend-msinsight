import { ThemeProvider } from '@emotion/react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { AppErrorBoundary } from './components/error/AppErrorBoundary';
import { SessionPageErrorBoundary } from './components/error/SessionPageErrorBoundary';
import { useRootStore } from './context/context';
import i18n from './i18n';
import { SessionPage } from './pages/SessionPage';
import { platform } from './platforms';
import { themeInstance, ThemeItem } from './theme/theme';
import { getSearchParams } from './utils/localUrl';
import eventBus, { EventType } from './utils/eventBus';

const Window = styled.div`
    background-color: ${props => props.theme.backgroundColor};
    text-align: center;
    height: 100vh;
    overflow: hidden;
    user-select: none;
    display: flex;
    width: 100vw;
    color: ${props => props.theme.fontColor};
    font-family: Microsoft YaHei;
`;

// 全局新增监听搜索快捷键输入
document.addEventListener('keydown', (e) => {
    if (e.ctrlKey && (e.key === 'f' || e.key === 'F')) {
        eventBus.emit(EventType.GLOBALSEARCH);
    }
});

const forbidDefaultEvent = (e: MouseEvent): void => {
    e.preventDefault();
};
window.addEventListener('drop', forbidDefaultEvent);
window.addEventListener('dragover', forbidDefaultEvent);

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
        background-color: ${props => props.theme.maskColor};
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
    const lang = getSearchParams('language');
    useEffect(() => {
        insightStore.loadTemplates().then(() => {
            session = sessionStore.activeSession;
        });
        i18n.changeLanguage(lang === 'zh' ? 'zh' : 'en');
        platform.initTheme().then((res: ThemeItem) => {
            window.setTheme(res === 'dark');
        });
    }, []);
    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            <Window>
                <AppErrorBoundary>
                    <SessionPageErrorBoundary>
                        {session !== undefined ? <SessionPage session={session} /> : <StatePopover/>}
                    </SessionPageErrorBoundary>
                </AppErrorBoundary>
            </Window>
        </ThemeProvider>
    );
});
