/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Form, InputNumber, Button, Select, Checkbox, Tooltip } from 'ascend-components';
import { message } from 'antd';
import _ from 'lodash';
import eventBus, { useEventBus } from '../../utils/eventBus';
import {
    type communicator,
    type communicatorContainerData,
    generateCommunicatorData,
    getRankData,
    type ppData,
    type dpData,
    type tpData,
    type rankItem,
} from './ContainerUtils';
import connector from '../../connection';
import styled from '@emotion/styled';
import { displayRect, drawLineSVG, hideLine, transformLine } from './draw';
import { select } from 'd3';
import { CheckboxChangeEvent } from 'antd/lib/checkbox';
import { FormInstance } from 'antd/lib/form';

const RankContainer = styled.div`
    position: relative;
    max-height: 700px;
    overflow: auto;
`;

const RankItem = styled.div`
    text-align: center;
    .rank{
        height: 35px;
        width: 35px;
        margin: 0 20px;
    }
`;

const PpContainer = styled.div`
    display: flex;
    text-align: center;
    padding: 16px;
`;

const DpContainer = styled.div`
    position: relative;
    display: flex;
    text-align: center;
    padding: 0 16px;
`;

const TpContainer = styled.div`
    display: flex;
    text-align: center;
    padding: 16px;
    margin: 0 10px;
    background-color: ${(props): string => props.theme.rankBackgroudColor};
    border-radius: 2px;
    box-shadow: 0px 2px 4px 0px;
`;

interface parallelStrategyType {
    algorithm: string;
    dpSize: number;
    level: string;
    ppSize: number;
    tpSize: number;
};

export const CommunicatorContainer = observer(({ session }: { session: Session }) => {
    const [unitCount, setUnitCount] = useState<number>(0);
    getDefaultCommunicatorData(setUnitCount);
    return (
        <div style={{ marginBottom: 24 }}>
            {<CommunicatorHeader session={session} defaultPPSize={session.communicatorData.defaultPPSize} unitCount={unitCount}></CommunicatorHeader>}
            {<CommunicatorContent session={session} ranksData={session.ranksData}/>}
        </div>
    );
});

const CommunicatorHeader = observer(({ session, defaultPPSize, unitCount }: { session: Session; defaultPPSize: number; unitCount: number }) => {
    const [form, setForm] = useState({} as FormInstance<any>);
    const [disabled, setDisabled] = useState(false);
    const init = async (): Promise<void> => {
        const { dpSize, tpSize, ppSize, level } = (await getParallelStrategy()) as unknown as parallelStrategyType;
        form.setFieldsValue({ dpSize, tpSize, ppSize });
        if (level === 'collected') {
            setDisabled(true);
        } else {
            setDisabled(false);
        }
        session.communicatorData = generateCommunicatorData({ dpSize, tpSize, ppSize }, defaultPPSize);
        session.ranksData = getRankData({ dpSize, tpSize, ppSize });
        connector.send({ event: 'updateCommunicatorData', body: session.communicatorData, to: 4 });
        eventBus.emit('activeCommunicator', undefined);
    };
    useEffect(() => {
        init();
    }, [session.renderId]);
    const onClick = (size: number) => (): void => {
        const values: { ppSize: number; tpSize: number; dpSize: number; algorithm: string } = form.getFieldsValue();
        if (values.dpSize * values.tpSize * values.ppSize !== unitCount) {
            message.error('The parameter is incorrect.');
            return;
        }
        setParallelStrategy({ ...values });
        session.communicatorData = generateCommunicatorData(values, size);
        session.ranksData = getRankData(values);
        // 只有用户手动修改并行策略，才会同步策略信息给通信页面，此处4为通信页面页签的序号
        connector.send({ event: 'updateCommunicatorData', body: session.communicatorData, to: 4 });
        eventBus.emit('activeCommunicator', undefined);
    };
    const getForm = (dom: FormInstance<any>): void => {
        setForm(dom);
    };
    return (
        <FormDom unitCount={unitCount} disabled={disabled} defaultPPSize={defaultPPSize} onClick={onClick} getForm={getForm}/>
    );
});

const FormDom = ({ unitCount, disabled, defaultPPSize, onClick, getForm }:
{ unitCount: number; disabled: boolean; defaultPPSize: number; onClick: (size: number) => () => void;
    getForm: (dom: FormInstance<any>) => void; }): JSX.Element => {
    const { t } = useTranslation('summary');
    const [form] = Form.useForm();
    useEffect(() => {
        getForm(form);
    }, []);
    return (
        <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
            <Form.Item name={'algorithm'} label={t('Algorithm')} style={{ margin: '10px 24px 10px 0' }} initialValue={'Megatron-LM(tp-dp-pp)'}>
                <Select defaultValue="Megatron-LM(tp-dp-pp)" style={{ width: '120px' }} options={[
                    { value: 'Megatron-LM(tp-dp-pp)', label: 'Megatron-LM(tp-dp-pp)' }]}/>
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
            <Tooltip placement="right" title={disabled ? t('ProhibitConfiguration') : ''}>
                <div style={{ width: '70px' }}>
                    <Button type="primary" style={{ margin: '10px 32px 10px 0' }} disabled={disabled} onClick={onClick(defaultPPSize)}>{t('Generate')}</Button>
                </div>
            </Tooltip>
        </Form>
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
                communicators: [],
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
        if (result.partitionModes.length > 0 && data.ppGroups.length > 0) {
            setUnitCount(data.defaultPPSize * data.ppGroups[0].length);
        }
    }
    return result;
}

const CommunicatorContent = observer(({ session, ranksData }: { session: Session; ranksData: ppData[] }) => {
    const svg = select('#parallelDrawLineSVG');
    drawLineSVG(svg, ranksData, session.communicatorData.partitionModes);
    const onClick = (e: React.MouseEvent<any>): void => {
        e.stopPropagation();
        const target = e.target as HTMLElement;
        if (target.className === 'rank') {
            const site = target.getAttribute('site')?.split(' ').map(Number);
            if (site !== undefined) {
                transformLine(svg, site);
            }
        } else {
            if (target.nodeName !== 'polyline') {
                hideLine(svg);
            }
        }
    };
    const onChange = (name: string, checked: boolean): void => {
        displayRect(svg, name, checked);
    };
    useEventBus('activeCommunicator', (data) => {
        if (data === undefined) {
            session.activeCommunicator = data;
        } else {
            const selectCommunicator = data as communicator;
            session.activeCommunicator = selectCommunicator;
        }
    });
    return (
        <>
            { ranksData.length > 0 && <ParallelSwitch session={session} onChange={onChange}/> }
            <RankContainer onClick={(e): void => onClick(e)} >
                <svg id='parallelDrawLineSVG' style={{ position: 'absolute', pointerEvents: 'none', zIndex: 10 }}></svg>
                <div style={{ paddingBottom: '40px' }}>
                    {
                        ranksData.map(item => (
                            <Pp key={item.key} ppData={item.values} session={session}></Pp>
                        ))
                    }
                </div>
            </RankContainer>
        </>
    );
});

const ParallelSwitch = ({ onChange, session }: { session: Session; onChange: (name: string, checked: boolean) => void }): JSX.Element => {
    const { t } = useTranslation('summary');
    const [form] = Form.useForm();
    const options = [
        {
            name: 'dataParallel',
            title: 'Data Parallel',
        },
        {
            name: 'tensorParallel',
            title: 'Tensor Parallel',
        },
        {
            name: 'pipelineParallel',
            title: 'Pipeline Parallel',
        },
    ];
    useEffect(() => {
        form.setFieldsValue({
            dataParallel: false,
            tensorParallel: false,
            pipelineParallel: false,
        });
    }, [session.ranksData]);
    return (
        <Form form={form} layout="inline">
            {
                options.map(item => (
                    <Form.Item name={item.name} label={t(item.title)} key={item.name} valuePropName='checked'>
                        <Checkbox onChange={(e: CheckboxChangeEvent): void => { onChange(item.title, e.target.checked); }}></Checkbox>
                    </Form.Item>
                ))
            }
        </Form>
    );
};

const Pp = ({ session, ppData }: { session: Session; ppData: dpData[] }): JSX.Element => {
    return (
        <PpContainer id={'PpContainer'}>
            {
                ppData.map(item => (
                    <Dp key={item.key} dpData={item.values} session={session}></Dp>
                ))
            }
        </PpContainer>
    );
};

const Dp = ({ session, dpData }: { session: Session; dpData: tpData[] }): JSX.Element => {
    return (
        <DpContainer>
            {
                dpData.map(item => (
                    <Tp key={item.key} tpData={item.values} session={session}></Tp>
                ))
            }
        </DpContainer>
    );
};

const Tp = ({ session, tpData }: { session: Session; tpData: rankItem[] }): JSX.Element => {
    return (
        <TpContainer style={{ border: '2px white solid' }}>
            {
                tpData.map(item => (
                    <Rank key={item.value} rank={item} session={session}></Rank>
                ))
            }
        </TpContainer>
    );
};

const Rank = ({ session, rank }: { session: Session; rank: rankItem }): JSX.Element => {
    const ref = useRef(null);
    useEffect(() => {
        if (ref.current !== null) {
            (ref.current as HTMLElement).setAttribute('site', rank.site.join(' '));
        }
    }, [session.ranksData]);
    return (
        <RankItem>
            <div className={'rank'} ref={ref}></div>
            <div style={{ fontSize: '16px', width: '70px', overflow: 'hide', height: '18.84px' }} title={`rank ${rank.value}`}>rank {rank.value}</div>
        </RankItem>
    );
};

const getParallelStrategy = async (): Promise<string[]> => {
    return await window.requestData('summary/query/parallelStrategy', {}, 'summary');
};

const setParallelStrategy = async (params: { algorithm: string; ppSize: number; tpSize: number; dpSize: number }): Promise<void> => {
    const response = (await window.requestData('summary/set/parallelStrategy', { ...params }, 'summary')) as unknown as { result: boolean; msg: string };
    if (!response.result) {
        message.error('The parameter is incorrect.');
    }
};
