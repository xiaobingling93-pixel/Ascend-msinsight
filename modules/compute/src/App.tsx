/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { useRootStore } from './context/context';
import HotMethod from './components/hotMethod/HotMethod';

const App = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    useEffect(() => {
        session = sessionStore.activeSession;
    }, []);
    return session !== undefined ? <HotMethod session={session} /> : <></>;
});

export default App;
