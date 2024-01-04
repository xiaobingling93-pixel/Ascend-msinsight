/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { Session } from '../../entity/session';
import React, { useEffect, useMemo, useState } from 'react';
import { Tabs, Form, InputNumber, Row, Button, message, Select } from 'antd';
import _ from 'lodash';
import eventBus, { useEventBus } from '../../utils/eventBus';
import { queryTopSummary } from '../../utils/RequestUtils';
import {
    communicator,
    communicatorContainerData,
    generateCommunicatorData,
    partitionMode,
    tabData, titleMap,
} from './ContainerUtils';

export const CommunicatorContainer = observer(({ session }: { session: Session }) => {
    const [ activeTab, setActiveTab ] = useState<string>('pp');
    useEffect(() => {
        setActiveTab('pp');
        if (session.communicatorData.partitionModes.length === 0) {
            queryTopSummary({ step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 }).then((result) => {
                session.rankCount = result.rankCount;
                getDefaultCommunicatorData(result.filePath).then(value => {
                    session.communicatorData = value;
                });
            });
        }
    }, [session.communicatorData]);
    const items = useMemo(() => {
        return _.map(session.communicatorData.partitionModes, (value: partitionMode): tabData => {
            return {
                tab: titleMap.get(value.mode) as string,
                key: value.mode,
                content: <CommunicatorContent session={session} partitionData={value}/>,
            };
        });
    }, [session.communicatorData]);
    return (
        <div style={{ height: '300px', width: '100%', margin: '10px 0' }} className={'CommunicatorContainer'}>
            {<CommunicatorHeader session={session} defaultPPSize={session.communicatorData.defaultPPSize}></CommunicatorHeader>}
            <Tabs activeKey={activeTab} onTabClick={(key) => { eventBus.emit('setActiveTab', key); setActiveTab(key); }} style={{ height: '240px' }}>
                {
                    items.map(item => (
                        <Tabs.TabPane tab={item.tab} key={item.key} style={{ height: '140px' }}>
                            <div style={{ width: '100%', height: '100%', overflow: 'auto' }} className={'common-tabcontent'}>
                                {item.content}
                            </div>
                        </Tabs.TabPane>
                    ))
                }
            </Tabs>
        </div>
    );
});

const CommunicatorContent = observer(({ session, partitionData }: { session: Session; partitionData: partitionMode }) => {
    return (
        <Row>
            {
                _.map(partitionData.communicators, (communicator) => (
                    <RankGroup key={communicator.name} rankGroup={communicator} session={session}></RankGroup>
                ))
            }
        </Row>
    );
});

const RankGroup = ({ rankGroup, session }: { rankGroup: communicator; session: Session }): JSX.Element => {
    const [ active, setActive ] = useState('');
    useEventBus('activeCommunicator', (data) => {
        if (data === undefined) {
            setActive('');
            session.activeCommunicator = data;
        } else {
            const selectCommunicator = data as communicator;
            setActive(selectCommunicator.name);
            session.activeCommunicator = selectCommunicator;
        }
    });
    const width = (rankGroup.ranks.length * 85).toString().concat('px');
    return (
        <div style={{ width, margin: '0 10px', cursor: 'pointer' }}>
            <Row className={active === rankGroup.name ? 'activeRank' : active} wrap={false} >
                {
                    _.map(rankGroup.ranks, (value) => (
                        <RankId key={value} id={value} onClick={() => { selectRankGroup(rankGroup); }}></RankId>
                    ))
                }
            </Row>
        </div>
    );
};

const CommunicatorHeader = observer(({ session, defaultPPSize }: { session: Session; defaultPPSize: number }) => {
    const [form] = Form.useForm();
    const onClick = (size: number) => () => {
        const values: {ppSize: number; tpSize: number; dpSize: number} = form.getFieldsValue();
        if (values.dpSize * values.tpSize * values.ppSize !== session.rankCount) {
            message.error('The parameter is incorrect.');
            return;
        }
        session.communicatorData = generateCommunicatorData(values, size, session.rankCount);
        eventBus.emit('activeCommunicator', undefined);
    };
    return (
        <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
            <Form.Item name={'algorithm'} label={'Algorithm'} style={{ margin: '10px 10px 10px 0' }}>
                <Select defaultValue="Megatron" disabled={true} style={{ width: '120px' }} options={[
                    {
                        value: 'Megatron',
                        label: 'Megatron',
                    }]}/>
            </Form.Item>
            <Form.Item name={'ppSize'} label={'PP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.rankCount} style={{ width: '120px', margin: '0 0 0 10px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Form.Item name={'tpSize'} label={'TP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.rankCount} style={{ width: '120px', margin: '0 0 0 10px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Form.Item name={'dpSize'} label={'DP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.rankCount} style={{ width: '120px', margin: '0 0 0 10px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Button style={{ margin: '10px 10px 10px 0' }} onClick={onClick(defaultPPSize)}>Generate</Button>
        </Form>
    );
});

const RankId = ({ id, onClick }: { id: number; onClick: () => void }): JSX.Element => {
    return (
        <div style={{ height: '75px', width: '65px' }}>
            <Row justify={'center'} onClick={onClick}>
                <div className={'rank_icon'}></div>
            </Row>
            <Row justify={'center'} onClick={onClick}>
                <span>rank {id}</span>
            </Row>
        </div>
    );
};

export async function getDefaultCommunicatorData(filePath: string): Promise<communicatorContainerData> {
    const result = {
        partitionModes: [
            {
                mode: 'pp',
                communicators: [],
            },
            {
                mode: 'tpOrDp',
                communicators: [],
            },
        ],
        defaultPPSize: 0,
    };
    const data = await window.requestData('communicator/parse', { filePath }, 'communication');
    if (data?.ppGroups !== undefined && data?.tpOrDpGroups !== undefined && data?.defaultPPSize !== undefined) {
        result.partitionModes = [
            {
                mode: 'pp',
                communicators: data.ppGroups,
            },
            {
                mode: 'tpOrDp',
                communicators: data.tpOrDpGroups,
            },
        ];
        result.defaultPPSize = data.defaultPPSize;
    }
    return result;
}

function selectRankGroup(rankGroup: communicator): void {
    eventBus.emit('activeCommunicator', rankGroup);
}
