/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { useRootStore } from './context/context';
import Operator from './components/operator/Operator';
import { runInAction } from 'mobx';

const App = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    useEffect(() => {
        session = sessionStore.activeSession;
        window.setTheme = (isDark: boolean) => {
            document.body.className = isDark ? 'theme_dark' : 'theme_light';
            runInAction(() => {
                if (session === undefined) {
                    return;
                }
                session.isDark = isDark;
            });
        };
        window.setTheme(true);
    }, []);
    return session !== undefined ? <Operator session={session} /> : <></>;
});

export default App;
