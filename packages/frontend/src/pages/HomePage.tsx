import { Tabs } from 'antd';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { Session } from '../entity/session';
import { SessionPage } from './SessionPage';
import CommunicationAnalysis from './CommunicationAnalysis';
import BandwidthAnalysis from '../components/communicationAnalysis/BandwidthAnalysis';

const HomePage = observer(function ({ session, theme }: { session: Session;theme: string }) {
    const items = [
        {
            tab: 'Timeline View',
            key: 'timeline',
            content: <SessionPage session={session}/>,
        },
        {
            tab: 'Communication Analysis',
            key: 'CommunicationAnalysis',
            content: <CommunicationAnalysis session={session}/>,
        },
        {
            tab: 'Total HCCL Operators',
            key: 'Operators',
            content: <BandwidthAnalysis session={session}/>,
        },
    ];
    return (
        <div style={{ height: '100%', width: '100%' }} className={'theme_' + theme}>
            <Tabs>
                {
                    items.map(item => (
                        <Tabs.TabPane tab={item.tab} key={item.key}>
                            <div style={{ height: 'calc(100% - 62px)', width: '100%', position: 'absolute', display: 'flex' }}>
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
