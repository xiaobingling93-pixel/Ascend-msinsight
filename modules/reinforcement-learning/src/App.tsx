/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { Layout } from '@insight/lib/components';
import { Home } from '@/pages/Home';
import { connector } from '@/connection';

const App: React.FC = observer(() => {
    useEffect(() => {
        connector.send({
            event: 'getParseStatus',
            body: {
                from: 'RL',
                requests: ['language', 'theme', 'parseCompleted'],
            },
        });
    }, []);

    return (
        <Layout>
            <Home />
        </Layout>
    );
});

export default App;
