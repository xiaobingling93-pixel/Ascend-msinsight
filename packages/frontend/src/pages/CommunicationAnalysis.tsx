import { observer } from 'mobx-react-lite';
import React, { useState } from 'react';
import { Session } from '../entity/session';
import Help from '../components/communicationAnalysis/Help';
import Filter from '../components/communicationAnalysis/Filter';
import CommunicationTimeTable from '../components/communicationAnalysis/CommunicationTimeTable';
import CommunicationTimeChart from '../components/communicationAnalysis/CommunicationTimeChart';
import CommunicationMatrix from '../components/communicationAnalysis/CommunicationMatrix';
import BandwidthAnalysis from '../components/communicationAnalysis/BandwidthAnalysis';
import { ArrowLeftOutlined } from '@ant-design/icons';
import { Breadcrumb } from 'antd';
import { Space } from '../components/communicationAnalysis/Common';

const CommunicationAnalysis = observer(function ({ session }: { session: Session }) {
    const [ showWindow, setShowWindow ] = useState('CommunicationDurationAnalysis');
    const [ rankId, setRankId ] = useState('');
    const switchPage = (page: string): void => {
        setShowWindow(page);
    };

    const showOperator = (rankId: string): void => {
        setRankId(rankId);
    };

    const returnHome = (): void => {
        setRankId('');
    };

    const handleFilterChange = (conditions: any): void => {
    };
    return (
        <div style={{ textAlign: 'left', padding: '0 10px' }}>
            <div>
                <Help style={{ display: 'none' }}/>
                {/* 筛选条件 */}
                <Filter switchPage={switchPage} handleFilterChange={handleFilterChange} showWindow={showWindow}/>
                {/* 通信用时分析 */}
                <div style={{ display: showWindow === 'CommunicationDurationAnalysis' ? 'block' : 'none' }}>
                    <CommunicationTimeChart showWindow={showWindow}/>
                    <CommunicationTimeTable showOperator={showOperator}/>
                </div>
                {/* 通信矩阵 */}
                <CommunicationMatrix isShow={showWindow === 'CommunicationMatrix'}/>
            </div>
            {
                rankId !== '' && (
                    <div className={'fullbox'} style={{ padding: '0 20px', background: 'white' }}>
                        <Breadcrumb>
                            <Breadcrumb.Item onClick={returnHome }>
                                <a><ArrowLeftOutlined /><Space length={10}/><span>Communication Duration Analysis</span></a>
                            </Breadcrumb.Item>
                            <Breadcrumb.Item>Total HCCL Operators(RankId {rankId})</Breadcrumb.Item>
                        </Breadcrumb>
                        <BandwidthAnalysis session={session} />
                    </div>
                )
            }
        </div>
    );
});

export default CommunicationAnalysis;
