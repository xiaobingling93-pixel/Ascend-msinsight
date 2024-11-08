/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Form, InputNumber, Button, Select, Checkbox, Tooltip } from 'ascend-components';
import { message } from 'antd';
import { QuestionCircleOutlined } from '@ant-design/icons';
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
    getRankDataById,
    computeRankDyeingData,
    getRankDyeingData,
} from './ContainerUtils';
import connector from '../../connection';
import styled from '@emotion/styled';
import { displayRect, drawLineSVG, hideLine, transformLine } from './draw';
import { select } from 'd3';
import { CheckboxChangeEvent } from 'antd/lib/checkbox';
import { FormInstance } from 'antd/lib/form';
import { runInAction } from 'mobx';
import { clamp } from '../Common';

const RankContainer = styled.div`
    position: relative;
    max-height: 700px;
    overflow: auto;
    margin-top: 10px;
    padding-top: 5px;
    background-color: ${(props): string => props.theme.rankContainerBackgroudColor};
`;

const RankItem = styled.div`
    text-align: center;
    &::before {
        display: block;
        height: 32px;
        width: 32px;
        content: "";
        border-radius: 2px;
        margin: 0 4px;
        position: absolute;
        background: rgb(255, 255, 255);
    }
    .rank, .rankDyeing {
        height: 32px;
        width: 32px;
        position: relative;
        margin: 0 4px;
        border-radius: 2px;
    }
    .rankId {
        font-size: 8px;
        width: 32px;
        overflow: hide;
        height: 14px;
        margin: 0 4px;
    }
`;

const PpContainer = styled.div`
    display: flex;
    text-align: center;
    padding: 8px 16px;
`;

const DpContainer = styled.div`
    position: relative;
    display: flex;
    text-align: center;
`;

const TpContainer = styled.div`
    display: flex;
    text-align: center;
    padding: 12px 8px 6px 8px;
    margin: 0 8px;
    background-color: ${(props): string => props.theme.rankBackgroudColor};
    border-radius: 2px;
    box-shadow: ${(props): string => props.theme.rankBackgroudColor} 0px 2px 4px 0px;
`;

const Legend = styled.div`
    height: 20px;
    .legendContainer {
        display: flex;
        padding-top: 20px;
        position: absolute;
        left: 50%;
        transform: translateX(-50%);
        .legendItem {
            display: flex;
            height: 12px;
            line-height: 12px;
            margin: 0 12px;
            .legendColor {
                width: 12px;
                margin-right: 4px;
            }
            .legendLabel {
                font-size: 12px;
            }
        }
    }
`;

const ColorScaleContainer = styled.div`
    height: 20px;
    line-height: 20px;
    display: flex;
    margin-top: 5px;
    color: ${(props): string => props.theme.svgPlayBackgroundColor};
    .colorScale {
        width: 150px;
        background-image: linear-gradient(to right, #24AB36, #d3eed7 45%, #f9d2d2 55%, #E32020);
    }
    .colorScaleNum {
        margin: 0 5px;
    }
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
        session.rankCountAfterCal = unitCount;
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
            let { dpSize, tpSize, ppSize, level, algorithm } = (await getParallelStrategy()) as unknown as parallelStrategyType;
            const equal = dpSize === 1 && tpSize === 1 && tpSize === 1;
            // 限制数据最多设置为255，如果大于255，则直接使用255
            dpSize = clamp(dpSize, 255);
            tpSize = clamp(tpSize, 255);
            ppSize = clamp(ppSize, 255);
            const totalCount = dpSize * tpSize * ppSize;
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
            updateRankData(session, { dpSize, tpSize, ppSize, algorithm }, totalCount, defaultPPSize);
            form.setFieldsValue({ dpSize, tpSize, ppSize, algorithm });
        }
    };
    useEffect(() => {
        init();
    }, [session.renderId, session.rankCount]);
    const onClick = (size: number) => (): void => {
        const values: { ppSize: number; tpSize: number; dpSize: number; algorithm: string } = form.getFieldsValue();
        // 设置的并行策略乘积需要>=导入的卡数
        if (values.dpSize * values.tpSize * values.ppSize < unitCount) {
            message.error('The parameter is incorrect.');
            return;
        }
        setParallelStrategy({ ...values });
        updateRankData(session, values, values.dpSize * values.tpSize * values.ppSize, size);
        setShowRank(true);
    };
    return (
        <FormDom unitCount={unitCount} disabled={disabled} defaultPPSize={defaultPPSize} onClick={onClick} getForm={setForm}/>
    );
});

const PARALLEL_STRATEGY_INPUT_PROPS = { min: 0, max: 255, style: { width: '120px' }, maxLength: 200 };

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
                <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
            </Form.Item>
            <Form.Item name={'tpSize'} label={t('TPSize')} style={{ margin: '10px 24px 10px 0' }}>
                <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
            </Form.Item>
            <Form.Item name={'dpSize'} label={t('DPSize')} style={{ margin: '10px 24px 10px 0' }}>
                <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
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
    const [dyeingMode, setDyeingMode] = useState('None');
    const [dyeingStep, setDyeingStep] = useState(0.03);
    const svg = select('#parallelDrawLineSVG');
    const onClick = (e: React.MouseEvent<any>): void => {
        e.stopPropagation();
        const target = e.target as HTMLElement;
        if (typeof target.className === 'string' && (target.className.includes('rank') || target.className.includes('rankDyeing'))) {
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
        runInAction(() => { session.activeCommunicator = selectCommunicator; });
    });
    useEffect(() => {
        drawLineSVG(svg, ranksData, session.communicatorData.partitionModes);
    }, [ranksData]);
    return (
        <>
            { (showRank && ranksData.length > 0) && <ParallelSwitch session={session} onChange={onChange}
                setDyeingMode={setDyeingMode} dyeingMode={dyeingMode} setDyeingStep={setDyeingStep}/> }
            <RankContainer onClick={(e): void => onClick(e)} >
                <svg id="parallelDrawLineSVG" style={{ position: 'absolute', pointerEvents: 'none', zIndex: 10 }}></svg>
                {
                    showRank &&
                    <div style={{ paddingBottom: '5px' }} data-testid="parallelRankContainer">
                        {
                            ranksData.map(item => (
                                <Pp key={item.key} ppData={item.values} session={session} dyeingMode={dyeingMode} dyeingStep={dyeingStep}></Pp>
                            ))
                        }
                    </div>
                }
            </RankContainer>
            { (showRank && ranksData.length > 0) && <LegendContainer/> }
        </>
    );
});

const ColorScale = ({ dyeingMode }: { dyeingMode: string }): JSX.Element => {
    const rankDyeingData = getRankDyeingData();
    const colorScaleData = rankDyeingData[dyeingMode];
    return (
        (dyeingMode === 'None')
            ? <></>
            : <ColorScaleContainer>
                <div className="colorScaleNum">{Number(colorScaleData.min.toFixed(2))}</div>
                <div className="colorScale"></div>
                <div className="colorScaleNum">{Number(colorScaleData.max.toFixed(2))}</div>
            </ColorScaleContainer>
    );
};

const LegendContainer = (): JSX.Element => {
    const { t } = useTranslation('summary');
    const legendList = [
        { label: 'Pipeline Parallel', color: '#0277FF' },
        { label: 'Tensor Parallel', color: '#01CEB0' },
        { label: 'Data Parallel', color: '#6948C9' },
    ];
    return (
        <Legend>
            <div className="legendContainer">
                {
                    legendList.map(item => (
                        <div className="legendItem" key={item.label}>
                            <div className="legendColor" style={{ backgroundColor: item.color }}></div>
                            <div className="legendLabel">{t(item.label)}</div>
                        </div>
                    ))
                }
            </div>
        </Legend>
    );
};

const RankFloatContainer = ({ rankId, session }: { rankId: number; session: Session }): JSX.Element => {
    const { t } = useTranslation('summary');
    const order = [{ value: 'prepareTime', title: 'Preparing', unit: 'μs' },
        { value: 'computingTime', title: 'Total Computing', unit: 'μs' },
        { value: 'pureComputingTime', title: 'Pure Computing', unit: 'μs' },
        { value: 'communicationOverLappedTime', title: 'Communication(Overlapped)', unit: 'μs' },
        { value: 'communicationNotOverLappedTime', title: 'Communication(Not Overlapped)', unit: 'μs' },
        { value: 'freeTime', title: 'Free', unit: 'μs' },
        { value: 'computeTimeRatio', title: 'Total Computing Ratio', unit: '%' },
        { value: 'communicationTimeRatio', title: 'Communication Ratio', unit: '%' }];
    const rankData = getRankDataById(rankId, session.summaryList);
    return (
        <>
            {
                rankData !== undefined &&
                <div className="rankData">
                    <div className="rankId">rank {rankData.rankId}</div>
                    {
                        order.map(item => (
                            rankData[item.value] !== 0 &&
                            <div className="rankDataItem" key={item.value}>
                                <div className="title">{t(item.title)}</div>
                                <div className="value">{`${rankData[item.value]} ${item.unit}`}</div>
                            </div>
                        ))
                    }
                </div>
            }
        </>
    );
};

const checkboxOptions = [
    { name: 'pipelineParallel', title: 'Pipeline Parallel' },
    { name: 'tensorParallel', title: 'Tensor Parallel' },
    { name: 'dataParallel', title: 'Data Parallel' },
];

const selectOrder = [
    { value: 'None', label: 'None' },
    { value: 'prepareTime', label: 'Preparing' },
    { value: 'computingTime', label: 'Total Computing' },
    { value: 'pureComputingTime', label: 'Pure Computing' },
    { value: 'communicationOverLappedTime', label: 'Communication(Overlapped)' },
    { value: 'communicationNotOverLappedTime', label: 'Communication(Not Overlapped)' },
    { value: 'freeTime', label: 'Free' },
];

const ParallelSwitch = ({ onChange, session, setDyeingMode, dyeingMode, setDyeingStep }:
{ onChange: (name: string, checked: boolean) => void; setDyeingMode: (value: string) => void;
    session: Session; dyeingMode: string; setDyeingStep: (value: number) => void; }): JSX.Element => {
    const { t } = useTranslation('summary');
    const [form] = Form.useForm();
    const [selectOptions, setSelectOptions] = useState([] as any[]);
    const initSelectOptions = (): void => {
        const rankDyeingData = getRankDyeingData();
        const res: any[] = [];
        selectOrder.forEach(item => {
            if (item.value === 'None' || rankDyeingData[item.value].max !== 0) {
                res.push({ value: item.value, label: t(item.label) });
            }
        });
        setSelectOptions(res);
    };
    useEffect(() => {
        form.setFieldsValue({ dataParallel: false, tensorParallel: false, pipelineParallel: false, dataType: 'None', dyeingStep: 0.03 });
        setDyeingMode('None');
        setDyeingStep(0.03);
        computeRankDyeingData(session.summaryList);
        initSelectOptions();
    }, [session.ranksData]);
    useEffect(() => { initSelectOptions(); }, [session.language]);
    return (
        <Form form={form} layout="inline">
            {
                checkboxOptions.map(item => (
                    <Form.Item name={item.name} label={t(item.title)} key={item.name} valuePropName="checked" style={{ marginRight: '40px' }}>
                        <Checkbox onChange={(e: CheckboxChangeEvent): void => { onChange(item.title, e.target.checked); }}></Checkbox>
                    </Form.Item>
                ))
            }
            <Form.Item name={'dataType'} label={t('Data Type')} style={{ marginRight: '40px' }} initialValue={'None'}>
                <Select defaultValue="" style={{ width: '120px' }} onChange={(value: string): void => { setDyeingMode(value); }} options={selectOptions}/>
            </Form.Item>
            {
                dyeingMode !== 'None' && <Form.Item name={'dyeingStep'} label={(<DyeingTipAndLabel/>)} style={{ marginRight: '40px' }}>
                    <InputNumber defaultValue={0.03} min={0.01} max={0.1} style={{ width: '120px' }} step={0.01} maxLength={4}
                        onChange={(value): void => { setDyeingStep(value as number); }}/>
                </Form.Item>
            }
            <ColorScale dyeingMode={dyeingMode}/>
        </Form>
    );
};

const DyeingTipAndLabel = (): JSX.Element => {
    const { t } = useTranslation('summary');
    const hit = t('DyeingTooltip', { returnObjects: true }) as string[];
    return <div style={{ display: 'flex' }}>
        <Tooltip overlayClassName={'width-auto'} placement="bottom"
            title={
                (
                    <div style={{ padding: 10, whiteSpace: 'nowrap' }}>
                        {
                            hit?.map((item: string) => (
                                <div key={item}>{item}</div>
                            ))
                        }
                    </div>
                )
            }>
            <QuestionCircleOutlined style={{ cursor: 'pointer' }}/>
        </Tooltip>
        <div style={{ marginLeft: 5 }}>{t('DyeingStep')}</div>
    </div>;
};

const Pp = ({ session, ppData, dyeingMode, dyeingStep }: { session: Session; ppData: dpData[]; dyeingMode: string; dyeingStep: number }): JSX.Element => {
    return (
        <PpContainer id={'PpContainer'}>
            {
                ppData.map(item => (
                    <Dp key={item.key} dpData={item.values} session={session} dyeingMode={dyeingMode} dyeingStep={dyeingStep}></Dp>
                ))
            }
        </PpContainer>
    );
};

const Dp = ({ session, dpData, dyeingMode, dyeingStep }: { session: Session; dpData: tpData[]; dyeingMode: string; dyeingStep: number }): JSX.Element => {
    return (
        <DpContainer>
            {
                dpData.map(item => (
                    <Tp key={item.key} tpData={item.values} session={session} dyeingMode={dyeingMode} dyeingStep={dyeingStep}></Tp>
                ))
            }
        </DpContainer>
    );
};

const Tp = ({ session, tpData, dyeingMode, dyeingStep }: { session: Session; tpData: rankItem[]; dyeingMode: string; dyeingStep: number }): JSX.Element => {
    return (
        <TpContainer>
            {
                tpData.map(item => (
                    <Rank key={item.value} rank={item} session={session} dyeingMode={dyeingMode} dyeingStep={dyeingStep}></Rank>
                ))
            }
        </TpContainer>
    );
};

const Rank = ({ session, rank, dyeingMode, dyeingStep }: { session: Session; rank: rankItem; dyeingMode: string; dyeingStep: number }): JSX.Element => {
    const ref = useRef(null);
    const [opacity, setOpacity] = useState({});
    useEffect(() => {
        if (ref.current !== null) {
            (ref.current as HTMLElement).setAttribute('site', rank.site.join(' '));
            (ref.current as HTMLElement).setAttribute('rank-id', rank.value.toString());
        }
    }, [session.ranksData]);
    useEffect(() => {
        setOpacity(getOpacity(rank.value, dyeingMode, dyeingStep));
    }, [dyeingMode, dyeingStep]);
    return (
        <RankItem>
            <Tooltip title={(<RankFloatContainer rankId={rank.value} session={session}/>)} placement="bottom">
                <div className={`${dyeingMode !== 'None' ? 'rankDyeing' : 'rank'}`} ref={ref}
                    style={opacity}/>
            </Tooltip>
            <div className={'rankId'}>{rank.value}</div>
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
