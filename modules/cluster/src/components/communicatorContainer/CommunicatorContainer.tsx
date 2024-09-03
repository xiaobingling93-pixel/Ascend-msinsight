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
    getOpacity,
    computeOpacity,
    getRankDataById,
} from './ContainerUtils';
import connector from '../../connection';
import styled from '@emotion/styled';
import { displayRect, drawLineSVG, hideLine, transformLine } from './draw';
import { select } from 'd3';
import { CheckboxChangeEvent } from 'antd/lib/checkbox';
import { FormInstance } from 'antd/lib/form';
import { runInAction } from 'mobx';

const RankContainer = styled.div`
    position: relative;
    max-height: 700px;
    overflow: auto;
    margin-top: 40px;
`;

const RankItem = styled.div`
    text-align: center;
    &::before {
        display: block;
        height: 33px;
        width: 33px;
        content: "";
        margin: 1px 21px;
        border-radius: 2px;
        position: absolute;
        background: rgb(255, 255, 255);
    }
    .rankBase{
        height: 35px;
        width: 35px;
        margin: 0 20px;
        position: relative;
    }
    .rankDyeing {
        background-color: #ff0000;
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
    const { t } = useTranslation('summary');
    const [showRank, setShowRank] = useState(true);
    return (
        <div style={{ marginBottom: 24 }}>
            {<CommunicatorHeader session={session} defaultPPSize={session.communicatorData.defaultPPSize}
                unitCount={session.rankCount} setShowRank={setShowRank}></CommunicatorHeader>}
            {<CommunicatorContent session={session} ranksData={session.ranksData} showRank={showRank}/>}
            { !showRank && <div className={'noDataTip'}>{t('NoDataTip')}</div> }
        </div>
    );
});

const updateRankData = (session: Session, values: {ppSize: number; tpSize: number; dpSize: number; algorithm: string},
    unitCount: number, defaultPPSize: number): void => {
    runInAction(() => {
        session.communicatorData = generateCommunicatorData(values, unitCount, defaultPPSize);
        session.ranksData = getRankData(values);
    });
    connector.send({ event: 'updateCommunicatorData', body: session.communicatorData, to: 4 });
    eventBus.emit('activeCommunicator', undefined);
};

const CommunicatorHeader = observer(({ session, defaultPPSize, unitCount, setShowRank }:
{ session: Session; defaultPPSize: number; unitCount: number; setShowRank: React.Dispatch<React.SetStateAction<boolean>> }) => {
    const [form, setForm] = useState({} as FormInstance<any>);
    const [disabled, setDisabled] = useState(false);
    const init = async (): Promise<void> => {
        if (form.setFieldsValue === undefined) {
            setTimeout(() => {
                init();
            });
        } else {
            const { dpSize, tpSize, ppSize, level, algorithm } = (await getParallelStrategy()) as unknown as parallelStrategyType;
            const equal = dpSize === 1 && tpSize === 1 && tpSize === 1;
            if (level === 'collected') {
                setDisabled(true);
            } else {
                setDisabled(false);
            }
            if (level === 'undefined' || equal) {
                setShowRank(false);
            } else {
                setShowRank(true);
            }
            updateRankData(session, { dpSize, tpSize, ppSize, algorithm }, unitCount, defaultPPSize);
            form.setFieldsValue({ dpSize, tpSize, ppSize, algorithm });
        }
    };
    useEffect(() => {
        init();
    }, [session.renderId, session.rankCount]);
    const onClick = (size: number) => (): void => {
        const values: { ppSize: number; tpSize: number; dpSize: number; algorithm: string } = form.getFieldsValue();
        if (values.dpSize * values.tpSize * values.ppSize !== unitCount) {
            message.error('The parameter is incorrect.');
            return;
        }
        setParallelStrategy({ ...values });
        updateRankData(session, values, unitCount, size);
        setShowRank(true);
    };
    return (
        <FormDom unitCount={unitCount} disabled={disabled} defaultPPSize={defaultPPSize} onClick={onClick} getForm={setForm}/>
    );
});

const FormDom = ({ unitCount, disabled, defaultPPSize, onClick, getForm }:
{ unitCount: number; disabled: boolean; defaultPPSize: number; onClick: (size: number) => () => void;
    getForm: (dom: FormInstance<any>) => void; }): JSX.Element => {
    const { t } = useTranslation('summary');
    const selectOptions = [
        { value: 'Megatron-LM(tp-dp-pp)', label: 'Megatron-LM(tp-dp-pp)' },
        { value: 'Megatron-LM(tp-pp-dp)', label: 'Megatron-LM(tp-pp-dp)' },
    ];
    const [form] = Form.useForm();
    getForm(form);
    return (
        <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
            <Form.Item name={'algorithm'} label={t('Algorithm')} style={{ margin: '10px 24px 10px 0' }} initialValue={'Megatron-LM(tp-dp-pp)'}>
                <Select defaultValue="Megatron-LM(tp-dp-pp)" style={{ width: '120px' }} options={selectOptions}/>
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

export async function getDefaultCommunicatorData(): Promise<communicatorContainerData> {
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
    }
    return result;
}

const CommunicatorContent = observer(({ session, ranksData, showRank }: { session: Session; ranksData: ppData[]; showRank: boolean }) => {
    const [dyeingMode, setDyeingMode] = useState('Unstained');
    const svg = select('#parallelDrawLineSVG');
    const onClick = (e: React.MouseEvent<any>): void => {
        e.stopPropagation();
        const target = e.target as HTMLElement;
        if (target.nodeName === 'DIV' && target.className.includes('rankBase')) {
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
        const selectCommunicator = data as communicator | undefined;
        runInAction(() => {
            session.activeCommunicator = selectCommunicator;
        });
    });
    useEffect(() => {
        drawLineSVG(svg, ranksData, session.communicatorData.partitionModes);
    }, [ranksData]);
    return (
        <>
            { (showRank && ranksData.length > 0) && <ParallelSwitch session={session} onChange={onChange} setDyeingMode={setDyeingMode}/> }
            <RankContainer onClick={(e): void => onClick(e)} >
                <svg id='parallelDrawLineSVG' style={{ position: 'absolute', pointerEvents: 'none', zIndex: 10 }}></svg>
                {
                    showRank &&
                    <div style={{ paddingBottom: '40px' }}>
                        {
                            ranksData.map(item => (
                                <Pp key={item.key} ppData={item.values} session={session} dyeingMode={dyeingMode}></Pp>
                            ))
                        }
                    </div>
                }
            </RankContainer>
        </>
    );
});

const RankFloatContainer = ({ rankId, session }: { rankId: number; session: Session }): JSX.Element => {
    const { t } = useTranslation('summary');
    const order = [{ value: 'prepareTime', title: 'Preparing', unit: 'μs' },
        { value: 'computingTime', title: 'Total Computing', unit: 'μs' },
        { value: 'pureComputingTime', title: 'Pure Computing', unit: 'μs' },
        { value: 'communicationOverLappedTime', title: 'Communication(Not Overlapped)', unit: 'μs' },
        { value: 'communicationNotOverLappedTime', title: 'Communication(Overlapped)', unit: 'μs' },
        { value: 'freeTime', title: 'Free', unit: 'μs' },
        { value: 'computeTimeRatio', title: 'Total Computing Ratio', unit: '%' },
        { value: 'communicationTimeRatio', title: 'Communication Ratio', unit: '%' }];
    const rankData = getRankDataById(rankId, session.summaryList);
    return (
        <>
            {
                rankData !== undefined &&
                <div className='rankData'>
                    <div className='rankId'>rank {rankData.rankId}</div>
                    {
                        order.map(item => (
                            rankData[item.value] !== 0 &&
                            <div className='rankDataItem' key={item.value}>
                                <div className='title'>{t(item.title)}</div>
                                <div className='value'>{`${rankData[item.value]} ${item.unit}`}</div>
                            </div>
                        ))
                    }
                </div>
            }
        </>
    );
};

const ParallelSwitch = ({ onChange, session, setDyeingMode }: { onChange: (name: string, checked: boolean) => void;
    setDyeingMode: (value: string) => void; session: Session; }): JSX.Element => {
    const { t } = useTranslation('summary');
    const [form] = Form.useForm();
    const checkboxOptions = [
        { name: 'pipelineParallel', title: 'Pipeline Parallel' },
        { name: 'tensorParallel', title: 'Tensor Parallel' },
        { name: 'dataParallel', title: 'Data Parallel' },
    ];
    const selectOptions = [
        { value: 'None', label: t('None') },
        { value: 'TotalComputingTime', label: t('Total Computing Ratio') },
        { value: 'CommunicationTime', label: t('Communication Ratio') },
    ];
    useEffect(() => {
        form.setFieldsValue({
            dataParallel: false,
            tensorParallel: false,
            pipelineParallel: false,
            dataType: 'None',
        });
        setDyeingMode('None');
        computeOpacity(session.summaryList);
    }, [session.ranksData]);
    return (
        <Form form={form} layout="inline">
            {
                checkboxOptions.map(item => (
                    <Form.Item name={item.name} label={t(item.title)} key={item.name} valuePropName='checked' style={{ marginRight: '40px' }}>
                        <Checkbox onChange={(e: CheckboxChangeEvent): void => { onChange(item.title, e.target.checked); }}></Checkbox>
                    </Form.Item>
                ))
            }
            <Form.Item name={'dataType'} label={t('Data Type')} style={{ marginRight: '40px' }} initialValue={'None'}>
                <Select defaultValue="" style={{ width: '120px' }} onChange={(value: string): void => { setDyeingMode(value); }} options={selectOptions}/>
            </Form.Item>
        </Form>
    );
};

const Pp = ({ session, ppData, dyeingMode }: { session: Session; ppData: dpData[]; dyeingMode: string }): JSX.Element => {
    return (
        <PpContainer id={'PpContainer'}>
            {
                ppData.map(item => (
                    <Dp key={item.key} dpData={item.values} session={session} dyeingMode={dyeingMode}></Dp>
                ))
            }
        </PpContainer>
    );
};

const Dp = ({ session, dpData, dyeingMode }: { session: Session; dpData: tpData[]; dyeingMode: string }): JSX.Element => {
    return (
        <DpContainer>
            {
                dpData.map(item => (
                    <Tp key={item.key} tpData={item.values} session={session} dyeingMode={dyeingMode}></Tp>
                ))
            }
        </DpContainer>
    );
};

const Tp = ({ session, tpData, dyeingMode }: { session: Session; tpData: rankItem[]; dyeingMode: string }): JSX.Element => {
    return (
        <TpContainer style={{ border: '2px white solid' }}>
            {
                tpData.map(item => (
                    <Rank key={item.value} rank={item} session={session} dyeingMode={dyeingMode}></Rank>
                ))
            }
        </TpContainer>
    );
};

const Rank = ({ session, rank, dyeingMode }: { session: Session; rank: rankItem; dyeingMode: string }): JSX.Element => {
    const ref = useRef(null);
    useEffect(() => {
        if (ref.current !== null) {
            (ref.current as HTMLElement).setAttribute('site', rank.site.join(' '));
            (ref.current as HTMLElement).setAttribute('rank-id', rank.value.toString());
        }
    }, [session.ranksData]);
    return (
        <RankItem>
            <Tooltip title={(<RankFloatContainer rankId={rank.value} session={session}/>)} placement="bottom">
                <div className={`rankBase ${dyeingMode === 'None' ? 'rank' : 'rankDyeing'}`} ref={ref}
                    style={{ opacity: getOpacity(rank.value, dyeingMode) }}/>
                <div style={{ fontSize: '16px', width: '70px', overflow: 'hide', height: '18.84px' }}>rank {rank.value}</div>
            </Tooltip>
        </RankItem>
    );
};

const getParallelStrategy = async (): Promise<string[]> => {
    return await window.requestData('summary/query/parallelStrategy', {}, 'summary') ?? {};
};

const setParallelStrategy = async (params: { algorithm: string; ppSize: number; tpSize: number; dpSize: number }): Promise<void> => {
    const response = (await window.requestData('summary/set/parallelStrategy', { ...params }, 'summary')) as unknown as { result: boolean; msg: string };
    if (!response.result) {
        message.error('The parameter is incorrect.');
    }
};
