/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Tabs, Form, InputNumber, Button, message, Select } from 'lib/components';
import _ from 'lodash';
import eventBus, { useEventBus } from '../../utils/eventBus';
import {
    type communicator,
    type communicatorContainerData,
    generateCommunicatorData,
    type partitionMode,
    type tabData, titleMap,
} from './ContainerUtils';
import styled from '@emotion/styled';

const RankItem = styled.div`
    text-align: center;
    .rank_icon{
        margin: auto;
    }
`;

const RankGroupItem = styled.div`
    display: flex;
    cursor: pointer;
`;

const RankGroupContainer = styled.div`
    display: flex;
    flex-wrap: wrap;
    gap: 30px;
    padding: 16px;
    max-height: 240px;
    overflow: auto;
    background: ${(p): string => p.theme.bgColorLight}
`;

export const CommunicatorContainer = observer(({ session }: { session: Session }) => {
    const [activeTab, setActiveTab] = useState<string>('pp');
    const [unitCount, setUnitCount] = useState<number>(0);
    const { t } = useTranslation('summary');
    useEffect(() => {
        if (!session.clusterCompleted) {
            session.communicatorData = { partitionModes: [], defaultPPSize: 0 };
            return;
        }
        getDefaultCommunicatorData(setUnitCount).then(value => {
            session.communicatorData = value;
        });
    }, [session.renderId]);
    useEffect(() => {
        if (!session.clusterCompleted) {
            return;
        }
        if (session.communicatorData.partitionModes.length === 0) {
            getDefaultCommunicatorData(setUnitCount).then(value => {
                session.communicatorData = value;
            });
        }
    }, [session.communicatorData]);
    const items = useMemo(() => {
        return _.map(session.communicatorData.partitionModes, (value: partitionMode): tabData => {
            return {
                label: t(titleMap.get(value.mode) as string),
                key: value.mode,
                children: <CommunicatorContent session={session} partitionData={value}/>,
            };
        });
    }, [session.communicatorData, t]);
    return (
        <div style={{ marginBottom: 24 }}>
            {<CommunicatorHeader session={session} defaultPPSize={session.communicatorData.defaultPPSize} unitCount={unitCount}></CommunicatorHeader>}
            <Tabs activeKey={activeTab}
                onTabClick={(key: string): void => { eventBus.emit('setActiveTab', key); setActiveTab(key); }}
                items={items}
            ></Tabs>
        </div>
    );
});

const CommunicatorContent = observer(({ session, partitionData }: { session: Session; partitionData: partitionMode }) => {
    return (
        <RankGroupContainer>
            {
                _.map(partitionData.communicators, (item) => (
                    <RankGroup key={item.name} rankGroup={item} session={session}></RankGroup>
                ))
            }
        </RankGroupContainer>
    );
});

const RankGroup = ({ rankGroup, session }: { rankGroup: communicator; session: Session }): JSX.Element => {
    const [active, setActive] = useState('');
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

    return (
        <RankGroupItem className={active === rankGroup.name ? 'activeRank' : active}>
            {
                _.map(rankGroup.ranks, (value) => (
                    <RankId key={value} id={value} onClick={(): void => { selectRankGroup(rankGroup); }}></RankId>
                ))
            }
        </RankGroupItem>
    );
};

const CommunicatorHeader = observer(({ session, defaultPPSize, unitCount }: { session: Session; defaultPPSize: number; unitCount: number }) => {
    const [form] = Form.useForm();
    const { t } = useTranslation('summary');
    const onClick = (size: number) => (): void => {
        const values: {ppSize: number; tpSize: number; dpSize: number} = form.getFieldsValue();
        if (values.dpSize * values.tpSize * values.ppSize !== unitCount) {
            message.error('The parameter is incorrect.');
            return;
        }
        session.communicatorData = generateCommunicatorData(values, size, unitCount);
        eventBus.emit('activeCommunicator', undefined);
    };
    return (
        <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
            <Form.Item name={'algorithm'} label={t('Algorithm')} style={{ margin: '10px 24px 10px 0' }}>
                <Select defaultValue="Megatron" disabled={true} style={{ width: '120px' }} options={[
                    {
                        value: 'Megatron',
                        label: 'Megatron',
                    }]}/>
            </Form.Item>
            <Form.Item name={'ppSize'} label={t('PPSize')} style={{ margin: '10px 24px 10px 0' }}>
                <InputNumber min={0} max={unitCount} style={{ width: '120px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Form.Item name={'tpSize'} label={t('TPSize')} style={{ margin: '10px 24px 10px 0' }}>
                <InputNumber min={0} max={unitCount} style={{ width: '120px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Form.Item name={'dpSize'} label={t('DPSize')} style={{ margin: '10px 24px 10px 0' }}>
                <InputNumber min={0} max={unitCount} style={{ width: '120px' }} maxLength={200}></InputNumber>
            </Form.Item>
            <Button type="primary" style={{ margin: '10px 32px 10px 0' }} onClick={onClick(defaultPPSize)}>{t('Generate')}</Button>
        </Form>
    );
});

const RankId = ({ id, onClick }: { id: number; onClick: () => void }): JSX.Element => {
    return (
        <RankItem onClick={onClick}>
            <div className={'rank_icon'}></div>
            <div>rank {id}</div>
        </RankItem>
    );
};

export async function getDefaultCommunicatorData(setUnitCount: React.Dispatch<React.SetStateAction<number>>): Promise<communicatorContainerData> {
    const result = {
        partitionModes: [
            {
                mode: 'pp',
                communicators: [] as communicator[],
            },
            {
                mode: 'tpOrDp',
                communicators: [] as communicator[],
            },
        ],
        defaultPPSize: 0,
    };
    const data = await window.requestData('communication/communicator', {}, 'communication');
    if (data?.ppGroups !== undefined && data?.tpOrDpGroups !== undefined && data?.defaultPPSize !== undefined) {
        result.partitionModes = [
            {
                mode: 'pp',
                communicators: _.map(data.ppGroups, (group, key) => {
                    return {
                        name: `stage${key}`,
                        ranks: group,
                        value: `(${_.join(group, ', ')}${(group.length > 1 ? ')' : ',)')}`,
                    } as communicator;
                }),
            },
            {
                mode: 'tpOrDp',
                communicators: _.map(data.tpOrDpGroups, (group, key) => {
                    return {
                        name: `tpOrDpStage${key}`,
                        ranks: group,
                        value: `(${_.join(group, ', ')}${(group.length > 1 ? ')' : ',)')}`,
                    } as communicator;
                }),
            },
        ];
        result.defaultPPSize = data.defaultPPSize;
        if (result.partitionModes.length > 0 && result.partitionModes[0].communicators.length > 0) {
            setUnitCount(data.defaultPPSize * result.partitionModes[0].communicators[0].ranks.length);
        }
    }
    return result;
}

function selectRankGroup(rankGroup: communicator): void {
    eventBus.emit('activeCommunicator', rankGroup);
}
