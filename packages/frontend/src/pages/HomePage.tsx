/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useState, useEffect } from 'react';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react-lite';
import { Tabs, Switch, Spin } from 'antd';
import { LoadingOutlined } from '@ant-design/icons';
import { Session } from '../entity/session';
import { SessionPage } from './SessionPage';
import CommunicationAnalysis from '../components/communicationAnalysis/CommunicationAnalysis';
import AnalysisSummary from './AnalysisSummary';
import { DragFileInit } from '../components/dragFile/DragFile';
import { CardUnit } from '../insight/units/AscendUnit';
import { queryTopSummary } from '../utils/RequestUtils';
import { getDefaultCommunicatorData } from '../components/communicatorContainer/CommunicatorContainer';

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

function init(session: Session): void {
    DragFileInit('home', (result: any) => {
        runInAction(() => {
            session.phase = 'download';
            session.endTimeAll = 1000000000;
            result.cards.forEach((item: any) => {
                const unit = new CardUnit({ cardId: item.rankId, cardName: item.cardName });
                if (item.result as boolean) {
                    unit.phase = 'analyzing';
                } else {
                    unit.phase = 'error';
                }
                session.units.push(unit);
            });
            session.allRankIds = result.cards.map((item: any) => item.rankId);
        });
    });
}

// eslint-disable-next-line max-lines-per-function
const HomePage = observer(function ({ session }: { session: Session }) {
    const parsing = isParing(session);
    const [ activeTab, setActiveTab ] = useState('timeline');
    const items = [
        {
            label: 'Timeline View',
            key: 'timeline',
            children: <div style={{ display: 'flex', height: '100%' }} id={'home'}><SessionPage session={session}/></div>,
        },
        {
            label: <div>Analysis Summary {parsing && <Spin indicator={antIcon}/>}</div>,
            key: 'AnalysisSummary',
            children: <AnalysisSummary session={session} active={activeTab === 'AnalysisSummary'}/>,
            display: session.units.length > 1,
            disabled: parsing,
        },
        {
            label: <div>Communication Analysis {parsing && <Spin indicator={antIcon}/>}</div>,
            key: 'CommunicationAnalysis',
            children: <CommunicationAnalysis session={session} active={activeTab === 'CommunicationAnalysis'}/>,
            display: session.units.length > 1,
            disabled: parsing,
        },
        {
            label: 'Memory Analysis',
            key: 'MemoryAnalysis',
            content: <div style={{ display: 'flex', height: '100%' }} id={'home'} />,
        },
    ];
    items.forEach(item => {
        let style = {};
        if (item.key === 'timeline') {
            style = { padding: 0 };
        }
        item.children = (<div className={'home-tab-children'} style={style}>{item.children}</div>);
    });
    const displayItems = items.filter(item => item.display !== false);
    const handleTabsChange = (activeKey: string): void => {
        setActiveTab(activeKey);
    };
    useEffect(() => {
        init(session);
    }, []);
    useEffect(() => {
        if (!parsing && session.units.length > 0) {
            queryTopSummary({ step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 }).then(({ result }) => {
                getDefaultCommunicatorData(result.filePath).then(value => {
                    session.communicatorData = value;
                });
            });
        }
    }, [ session.units, parsing ]);
    return (
        <div style={{ height: '100%', width: '100%' }}>
            <Switch checkedChildren="dark" unCheckedChildren="light" defaultChecked onChange={onChange}
                style={{ position: 'absolute', top: '10px', right: '50px', zIndex: 1000 }}/>
            <Tabs onChange={handleTabsChange} items={displayItems}/>
        </div>
    );
});

export default HomePage;
