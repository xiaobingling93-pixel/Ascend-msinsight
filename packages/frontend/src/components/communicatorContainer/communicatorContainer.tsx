/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { Session } from '../../entity/session';
import React, { useMemo, useState } from 'react';
import { Tabs, Form, InputNumber, Row, Button, message, Select, Tooltip } from 'antd';
import { ReactComponent as Rank } from '../../assets/images/rank_id.svg';
import _ from 'lodash';
import eventBus, { useEventBus } from '../../utils/eventBus';
import { QuestionCircleFilled } from '@ant-design/icons';

type communicatorContainerData = {
    partitionModes: partitionMode[];
};

type partitionMode = {
    mode: string;
    communicators: communicator[];
};

type communicator = {
    name: string;
    ranks: number[];
};

type tabData = {
    tab: string;
    key: string;
    content: JSX.Element;
};

const titleMap = new Map([
    [ 'pp', 'Pipeline Parallel' ],
    [ 'tp', 'Tensor Parallel' ],
    [ 'dp', 'Data Parallel' ],
    [ 'tpOrDp', 'Tensor/Data Parallel' ],
]);

export const CommunicatorContainer = observer(({ session }: { session: Session }) => {
    const [ communicator, setCommunicator ] = useState(getDefaultCommunicatorData());
    const [ activeTab, setActiveTab ] = useState<string>('pp');
    useEventBus('setCommunicator', (data) => {
        setCommunicator(data as communicatorContainerData);
    });
    const items = useMemo(() => {
        return _.map(communicator.partitionModes, (value: partitionMode): tabData => {
            return {
                tab: titleMap.get(value.mode) as string,
                key: value.mode,
                content: <CommunicatorContent session={session} partitionData={value}/>,
            };
        });
    }, [communicator]);
    return (
        <div style={{ height: '300px', width: '100%', margin: '10px 0' }} className={'CommunicatorContainer'}>
            {<CommunicatorHeader session={session}></CommunicatorHeader>}
            <Tabs activeKey={activeTab} onTabClick={(key) => setActiveTab(key)} style={{ height: '240px' }}>
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
                    <RankGroup key={communicator.name} rankGroup={communicator}></RankGroup>
                ))
            }
        </Row>
    );
});

const RankGroup = ({ rankGroup }: {rankGroup: communicator}): JSX.Element => {
    const [ active, setActive ] = useState('');
    useEventBus('activeCommunicator', (data) => {
        if (data === undefined) {
            setActive('');
        } else {
            const selectCommunicator = data as communicator;
            setActive(selectCommunicator.name);
        }
    });
    const width = (rankGroup.ranks.length * 85).toString().concat('px');
    return (
        <div style={{ width }}>
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

const CommunicatorHeader = observer(({ session }: { session: Session }) => {
    const [form] = Form.useForm();
    const onClick = (): void => {
        const values: {ppSize: number; tpSize: number; dpSize: number} = form.getFieldsValue();
        if (values.dpSize * values.tpSize * values.dpSize !== session.units.length) {
            message.error('The parameter is incorrect.');
            return;
        }
        eventBus.emit('setCommunicator', generateCommunicatorData(values, session.units.length));
    };
    return (
        <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
            <Form.Item name={'ppSize'} label={'PP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.units.length} style={{ width: '120px', margin: '0 0 0 10px' }}></InputNumber>
            </Form.Item>
            <Form.Item name={'tpSize'} label={'TP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.units.length} style={{ width: '120px', margin: '0 0 0 10px' }}></InputNumber>
            </Form.Item>
            <Form.Item name={'dpSize'} label={'DP Size'} style={{ margin: '10px 10px 10px 0' }}>
                <InputNumber min={0} max={session.units.length} style={{ width: '120px', margin: '0 0 0 10px' }}></InputNumber>
            </Form.Item>
            <Form.Item name={'algorithm'} label={'Algorithm'} style={{ margin: '10px 10px 10px 0' }}>
                <Select defaultValue="Megatron" disabled={true} style={{ width: '120px' }} options={[
                    {
                        value: 'Megatron',
                        label: <div>Megatron <Tooltip title={
                            (
                                <div style={{ background: 'var(--grey100)', padding: '1rem' }}>
                                    <div>RankCounts = PP Size * TP Size * DP Size</div>
                                </div>
                            )
                        }>
                            <QuestionCircleFilled style={{ cursor: 'pointer' }}/>
                        </Tooltip></div>,
                    }]}/>
            </Form.Item>
            <Button style={{ margin: '10px 10px 10px 0' }} onClick={onClick}>Generate</Button>
        </Form>
    );
});

const RankId = ({ id, onClick }: { id: number; onClick: () => void }): JSX.Element => {
    return (
        <div style={{ height: '75px', width: '65px' }}>
            <Row justify={'center'} onClick={onClick}>
                <Rank></Rank>
            </Row>
            <Row justify={'center'} onClick={onClick}>
                <span>rank {id}</span>
            </Row>
        </div>
    );
};

function getDefaultCommunicatorData(): communicatorContainerData {
    // 先用模拟数据代替
    const partitionModes: partitionMode[] = [
        {
            mode: 'pp',
            communicators: [ { name: 'stage1', ranks: [ 0, 1, 2, 3 ] }, { name: 'stage2', ranks: [ 4, 5, 6, 7 ] } ],
        },
        {
            mode: 'tpOrDp',
            communicators: [ { name: 'modelOrData1', ranks: [ 0, 1 ] }, { name: 'modelOrData2', ranks: [ 2, 3 ] },
                { name: 'modelOrData3', ranks: [ 4, 5 ] }, { name: 'modelOrData4', ranks: [ 6, 7 ] } ],
        },
    ];
    return {
        partitionModes,
    };
}

function generateCommunicatorData(values: {ppSize: number; tpSize: number; dpSize: number}, rankNum: number): communicatorContainerData {
    const partitionModes: partitionMode[] = [
        { mode: 'pp', communicators: [] },
        { mode: 'tp', communicators: [] },
        { mode: 'dp', communicators: [] },
    ];
    const pipelineCount = rankNum / values.ppSize;
    const modelCount = rankNum / values.tpSize;
    for (let i = 0; i < pipelineCount; i++) {
        partitionModes[0].communicators.push({
            ranks: _.range(i * values.ppSize, (i + 1) * values.ppSize), name: 'stage' + i.toString(),
        });
        for (let j = 0; j < values.tpSize; j++) {
            partitionModes[2].communicators.push({
                ranks: _.range(i * values.ppSize + j, (i + 1) * values.ppSize + j, values.tpSize),
                name: 'data' + (i * values.tpSize + j).toString(),
            });
        }
    }
    for (let i = 0; i < modelCount; i++) {
        partitionModes[1].communicators.push({
            ranks: _.range(i * values.tpSize, (i + 1) * values.tpSize), name: 'model' + i.toString(),
        });
    }
    eventBus.emit('activeCommunicator', undefined);
    return { partitionModes };
}

function selectRankGroup(rankGroup: communicator): void {
    eventBus.emit('activeCommunicator', rankGroup);
}
