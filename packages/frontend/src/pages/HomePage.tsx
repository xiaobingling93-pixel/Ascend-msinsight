/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Tabs, Switch } from 'antd';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { Session } from '../entity/session';
import { SessionPage } from './SessionPage';
import CommunicationAnalysis from './CommunicationAnalysis';
import AnalysisSummary from './AnalysisSummary';
import { notNull } from '../components/communicationAnalysis/Common';

const onChange = async (checked: boolean): Promise<void> => {
    window.setTheme(checked);
};

const HomePage = observer(function ({ session }: { session: Session }) {
    const items = [
        {
            tab: 'Timeline View',
            key: 'timeline',
            content: <div style={{ display: 'flex', height: '100%' }}><SessionPage session={session}/></div>,
        },
        {
            tab: 'Analysis Summary',
            key: 'AnalysisSummary',
            content: <AnalysisSummary session={session}/>,
            display: notNull(session.allRankIds) && session.allRankIds.length > 0,
        },
        {
            tab: 'Communication Analysis',
            key: 'CommunicationAnalysis',
            content: <CommunicationAnalysis session={session}/>,
            display: notNull(session.allRankIds) && session.allRankIds.length > 0,
        },
    ];
    return (
        <div style={{ height: '100%', width: '100%' }}>
            <Switch checkedChildren="dark" unCheckedChildren="light" defaultChecked onChange={onChange}
                style={{ position: 'absolute', top: '10px', right: '50px', zIndex: 1000 }}/>
            <Tabs >
                {
                    items.map(item => (
                        <Tabs.TabPane tab={item.tab} key={item.key}>
                            <div style={{ width: '100%', position: 'fixed', height: 'calc(100% - 62px)' }}>
                                {item.content}
                            </div>
                        </Tabs.TabPane>
                    ))
                }
            </Tabs>
        </div>
    );
});

export default HomePage;
