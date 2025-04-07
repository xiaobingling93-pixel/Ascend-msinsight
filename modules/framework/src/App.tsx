/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import { GlobalStyles, themeInstance } from 'ascend-theme';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { DragDirection, SizeMethod, useDraggableContainer } from 'ascend-use-draggable-container';
import { useRootStore } from './context/context';
import RemoteManager from './components/RemoteManager/Index';
import Main from './components/Main';
import { Session } from '@/entity/session';
import { connectRemote } from '@/centralServer/server';
import { LOCAL_HOST, PORT, JUPYTERLABPROXY } from '@/centralServer/websocket/defs';
import './index.css';
import { registerEventListeners } from '@/connection';
import { registerDragAndDropFile } from '@/utils';
import { Spin } from 'antd';
import { LoadingOutlined } from '@ant-design/icons';
import i18n from 'ascend-i18n';
import { runInAction } from 'mobx';

const init = async(session: Session): Promise<void> => {
    // 注册文件拖拽
    registerDragAndDropFile();
    // 连接与模块的通信
    registerEventListeners();

    // 连接ws（启动后第一次）
    const isSuccess = await connectRemote({ remote: LOCAL_HOST, port: PORT, jupyterlabProxy: JUPYTERLABPROXY, projectName: '', dataPath: [] });
    if (isSuccess) {
        runInAction(() => {
            session.defaultConnected = true;
        });
    }
};

const LEFT_WIDTH = 300;
const App = observer(() => {
    const session = useRootStore().sessionStore.activeSession;
    const [locale] = useState<'zhCN' | 'enUS'>('zhCN');

    const [view] = useDraggableContainer({
        draggableWH: LEFT_WIDTH,
        foldWH: 0,
        dragDirection: DragDirection.LEFT,
        open: true,
        sizeMethod: SizeMethod.NUMBER,
    });

    useEffect(() => {
        // 启动
        init(session);
    }, []);

    return session !== undefined
        ? <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                <Spin
                    wrapperClassName="spin-global"
                    spinning={session.loading}
                    tip={i18n.t('Loading', { ns: 'framework' })}
                    indicator={<LoadingOutlined style={{ fontSize: 24 }} spin />}>
                    {
                        view({
                            mainContainer: <Main session={session} />,
                            draggableContainer: <RemoteManager session={session} />,
                            id: 'framework',
                            padding: 16,
                        })
                    }
                </Spin>
            </SharedConfigProvider>
        </ThemeProvider>
        : <></>;
});

export default App;
