/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { Tabs, Switch, Spin } from 'antd';
import { LoadingOutlined } from '@ant-design/icons';
import { Session } from '../entity/session';
import { SessionPage } from './SessionPage';
import CommunicationAnalysis from '../components/communicationAnalysis/CommunicationAnalysis';
import AnalysisSummary from './AnalysisSummary';
import { DragFileInit } from '../components/dragFile/DragFile';

const antIcon = <LoadingOutlined style={{ fontSize: 24 }} spin />;

const onChange = async (checked: boolean): Promise<void> => {
    window.setTheme(checked);
};

const isParing = (session: Session): boolean => {
    let parsing = false;
    session.units.forEach(unit => {
        if (unit.phase === 'analyzing') {
            parsing = true;
        }
    });
    return parsing;
};

const HomePage = observer(function ({ session }: { session: Session }) {
    const parsing = isParing(session);
    const [ activeTab, setActiveTab ] = useState('timeline');
    const items = [
        {
            tab: 'Timeline View',
            key: 'timeline',
            content: <div style={{ display: 'flex', height: '100%' }} id={'home'}><SessionPage session={session}/></div>,
        },
        {
            tab: <div>Analysis Summary {parsing && <Spin indicator={antIcon}/>}</div>,
            key: 'AnalysisSummary',
            content: <AnalysisSummary session={session} active={activeTab === 'AnalysisSummary'}/>,
            display: session.units.length > 1,
        },
        {
            tab: <div>Communication Analysis {parsing && <Spin indicator={antIcon}/>}</div>,
            key: 'CommunicationAnalysis',
            content: <CommunicationAnalysis session={session} active={activeTab === 'CommunicationAnalysis'}/>,
            display: session.units.length > 1,
        },
    ];
    const displayItems = items.filter(item => item.display !== false);
    const handleTabsChange = (activeKey: string): void => {
        setActiveTab(activeKey);
    };
    useEffect(() => {
        DragFileInit('home');
    }, []);
    return (
        <div style={{ height: '100%', width: '100%' }}>
            <Switch checkedChildren="dark" unCheckedChildren="light" defaultChecked onChange={onChange}
                style={{ position: 'absolute', top: '10px', right: '50px', zIndex: 1000 }}/>
            <Tabs onChange={handleTabsChange}>
                {
                    displayItems.map(item => (
                        <Tabs.TabPane tab={item.tab} key={item.key}>
                            <div style={{ width: '100%', position: 'fixed', height: 'calc(100% - 62px)' }} className={'common-tabcontent'}>
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
