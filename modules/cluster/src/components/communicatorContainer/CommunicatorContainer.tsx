/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Form, InputNumber, InputGroup, InputSplit, Button, Select, CheckboxGroup, Tabs } from 'ascend-components';
import { message, Popconfirm } from 'antd';
import type { CheckboxValueType } from 'antd/es/checkbox/Group';
import eventBus from '../../utils/eventBus';
import styled from '@emotion/styled';
import { FormInstance } from 'antd/lib/form';
import { ParallelismGraph } from './ParallelismGraph';
import { getParallelStrategy, setParallelStrategy } from '../../utils/RequestUtils';
import { ErrorInfo, ParallelismArrangementParams, ParallelismType } from '../../utils/interface';
import { ParallelSwitchConditionsProvider, useParallelSwitchConditions } from './Context';
import type { TFunction } from 'i18next';
import { COLOR } from '../Common';

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
                flex: none;
                width: 12px;
                margin-right: 4px;
            }
            .legendLabel {
                flex: none;
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
        width: 200px;
        background-image: linear-gradient(to right, ${COLOR.BAND_3}, ${COLOR.BAND_2},${COLOR.BAND_1},${COLOR.BAND_0});
    }
    .colorScaleNum {
        color:${(props): string => props.theme.textColorTertiary};
        margin: 0 5px;
    }
`;

interface CommunicatorContainerProps {
    session: Session;
    generateConditions: GenerateConditions;
    onGenerateConditionsChange: (params: GenerateConditions) => void;
}

export const CommunicatorContainer = observer(({ session, generateConditions, onGenerateConditionsChange }: CommunicatorContainerProps) => {
    const { t } = useTranslation('summary');
    const [showRank, setShowRank] = useState(false);

    return (
        <div style={{ marginBottom: 24 }}>
            {<CommunicatorHeader
                session={session}
                showRank={showRank}
                setShowRank={setShowRank}
                generateConditions={generateConditions}
                setGenerateConditions={onGenerateConditionsChange}
            />}

            {
                showRank
                    ? <CommunicatorContent
                        session={session}
                        generateConditions={generateConditions}
                    />
                    : <div className={'noDataTip'}>{t('NoDataTip')}</div>
            }
        </div>
    );
});

export type GenerateConditions = ParallelismArrangementParams;
interface CommunicatorHeaderProps {
    session: Session;
    showRank: boolean;
    setShowRank: React.Dispatch<React.SetStateAction<boolean>>;
    generateConditions: GenerateConditions;
    setGenerateConditions: (params: GenerateConditions) => void;
}
const CommunicatorHeader = observer(({ session, showRank, setShowRank, generateConditions, setGenerateConditions }: CommunicatorHeaderProps) => {
    const [form] = Form.useForm();
    const [isCollectedConfiguration, setIsCollectedConfiguration] = useState(false);
    const [activeTab, setActiveTab] = useState(generateConditions.dimension);
    const { t } = useTranslation('summary');
    const dimensionOptions = useMemo(() => {
        const options = getDimensionOptions(t, generateConditions);

        // 无 CP 维度时，选中 DP 维度
        if (generateConditions.cpSize === 1 && activeTab === 'ep-dp-pp-cp') {
            setActiveTab('ep-dp');
        }

        return options.filter(option => {
            // 当 cpSize = 1，隐藏 cp 维度视图
            return !(generateConditions.cpSize === 1 && option.key === 'ep-dp-pp-cp');
        });
    }, [generateConditions.cpSize]);

    const init = async (): Promise<void> => {
        const { dpSize, tpSize, ppSize, cpSize, epSize, level, algorithm } = await getParallelStrategy();
        const equal = dpSize === 1 && tpSize === 1 && tpSize === 1 && cpSize === 1;
        if (level === 'collected') {
            setIsCollectedConfiguration(true);
        } else {
            setIsCollectedConfiguration(false);
        }
        if (level === 'undefined' || equal) {
            setShowRank(false);
        } else {
            setShowRank(true);
        }
        form.setFieldsValue({ dpSize, tpSize, ppSize, cpSize, epSize, algorithm });
        setGenerateConditions({ algorithm, dimension: activeTab, ppSize, tpSize, cpSize, dpSize, epSize });
        eventBus.emit('activeCommunicator', undefined);
    };

    useEffect(() => {
        init();
    }, []);

    const clickGenerate = async (): Promise<void> => {
        const values: GenerateConditions = form.getFieldsValue();
        // 设置的并行策略乘积需要>=导入的卡数
        if (values.dpSize * values.tpSize * values.ppSize * values.cpSize < session.rankCount) {
            message.error('The product of DP, CP, TP and PP size must be greater than the Device Count');
            return;
        }
        try {
            await setParallelStrategy({ ...values });
            setGenerateConditions({ ...values, dimension: activeTab });
            setShowRank(true);
            eventBus.emit('activeCommunicator', undefined);
            setIsCollectedConfiguration(false);
        } catch (e) {
            const errMsg = (e as ErrorInfo)?.message;
            if (errMsg !== undefined) {
                message.error(errMsg);
            }
        }
    };

    const handleTabChange = (key: string): void => {
        const dimension = key as ParallelismArrangementParams['dimension'];
        setActiveTab(dimension);

        setGenerateConditions({ ...generateConditions, dimension });
        eventBus.emit('activeCommunicator', undefined);
    };

    return <>
        <FormDom isCollectedConfiguration={isCollectedConfiguration} onClickGenerate={clickGenerate} form={form}/>
        {showRank && <Tabs
            activeKey={activeTab}
            onChange={handleTabChange}
            items={dimensionOptions}
        />}
    </>;
});

const PARALLEL_STRATEGY_INPUT_PROPS = { min: 1, max: 10000, style: { width: '80px' } };
const selectOptions = [
    { value: 'megatron-lm(tp-cp-ep-dp-pp)', label: 'Megatron-LM(tp-cp-ep-dp-pp)' },
    { value: 'megatron-lm(tp-cp-pp-ep-dp)', label: 'Megatron-LM(tp-cp-pp-ep-dp)' },
    { value: 'mindspeed(tp-cp-ep-dp-pp)', label: 'MindSpeed(tp-cp-ep-dp-pp)' },
];
const getDimensionOptions = (t: TFunction, generateConditions: GenerateConditions): Array<{key: string; label: string}> => {
    const { cpSize } = generateConditions;
    return [
        { key: 'ep-dp', label: `DP ${t('Dimension')} >` },
        { key: 'ep-dp-pp', label: `DP + PP ${t('Dimension')} >` },
        { key: 'ep-dp-pp-cp', label: `DP + PP + CP  ${t('Dimension')} >` },
        { key: 'ep-dp-pp-cp-tp', label: cpSize === 1 ? `DP + PP + TP ${t('Dimension')}` : `DP + PP + CP + TP ${t('Dimension')}` },
    ];
};

const FormDom = ({ isCollectedConfiguration, onClickGenerate, form }:
{isCollectedConfiguration: boolean; onClickGenerate: () => void; form: FormInstance<any> }): JSX.Element => {
    const { t } = useTranslation('summary');

    return (
        <>
            <Form form={form} labelAlign={'left'} layout="inline" className={'CommunicatorHeader'}>
                <Form.Item name={'algorithm'} label={t('Algorithm')} style={{ margin: '10px 24px 10px 0' }} initialValue={'megatron-lm(tp-cp-ep-dp-pp)'}>
                    <Select defaultValue="megatron-lm(tp-cp-ep-dp-pp)" style={{ width: 220 }} options={selectOptions}/>
                </Form.Item>
                <Form.Item name={'ppSize'} label={t('PPSize')} style={{ margin: '10px 24px 10px 0' }}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                <Form.Item name={'tpSize'} label={t('TPSize')} style={{ margin: '10px 24px 10px 0' }}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                <Form.Item name={'cpSize'} label={t('CPSize')} style={{ margin: '10px 24px 10px 0' }}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                <Form.Item name={'dpSize'} label={t('DPSize')} style={{ margin: '10px 24px 10px 0' }}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                <Form.Item name={'epSize'} label={t('EPSize')} style={{ margin: '10px 24px 10px 0' }}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                <Popconfirm
                    placement="right"
                    disabled={!isCollectedConfiguration}
                    title={<div style={{ maxWidth: 400 }}>{t('GenerateConfirm')}</div>}
                    onConfirm={onClickGenerate}
                >
                    <Button
                        type="primary"
                        style={{ margin: '10px 32px 10px 0' }}
                        onClick={isCollectedConfiguration ? undefined : onClickGenerate}
                    >
                        {t('Generate')}
                    </Button>
                </Popconfirm>
            </Form>
        </>
    );
};

interface CommunicatorContentProps {
    session: Session;
    generateConditions: GenerateConditions | null;
}

const CommunicatorContent = observer(({ session, generateConditions }: CommunicatorContentProps) => {
    return (
        <ParallelSwitchConditionsProvider>
            <ParallelSwitch session={session} generateConditions={generateConditions} dimension={generateConditions?.dimension} />
            <ParallelismGraph session={session} generateConditions={generateConditions} />
            <LegendContainer/>
        </ParallelSwitchConditionsProvider>
    );
});

const ColorScale = ({ min, max }: { min: number | null; max: number | null }): JSX.Element => {
    const isRangeEmpty = min === null || max === null;
    return isRangeEmpty
        ? <></>
        : <ColorScaleContainer>
            <div className="colorScaleNum">{min}</div>
            <div className="colorScale"></div>
            <div className="colorScaleNum">{max}</div>
        </ColorScaleContainer>
    ;
};

const LegendContainer = (): JSX.Element => {
    const { t } = useTranslation('summary');
    const legendList = [
        { label: 'Pipeline Parallel', color: '#0277FF' },
        { label: 'Tensor Parallel', color: '#01CEB0' },
        { label: 'Data Parallel', color: '#6948C9' },
        { label: 'Context Parallel', color: '#FD2F2F' },
        { label: 'Expert Parallel', color: '#EE891D' },
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

const useParallelTypeOptions = (dimension: ParallelSwitchProps['dimension'], generateConditions: GenerateConditions | null):
Array<{label: string;value: string; disabled?: boolean}> => {
    const { t } = useTranslation('summary');
    const { cpSize } = generateConditions ?? {};
    const val = dimension ?? '';
    const ppDisabled = ['ep-dp'].includes(val);
    const tpDisabled = ['ep-dp', 'ep-dp-pp', 'ep-dp-pp-cp'].includes(val);
    const cpDisabled = ['ep-dp', 'ep-dp-pp'].includes(val);

    return [
        { value: 'pp', label: t('Pipeline Parallel'), disabled: ppDisabled, visible: true },
        { value: 'tp', label: t('Tensor Parallel'), disabled: tpDisabled, visible: true },
        { value: 'cp', label: t('Context Parallel'), disabled: cpDisabled, visible: cpSize !== 1 },
        { value: 'dp', label: t('Data Parallel'), visible: true },
        { value: 'ep', label: t('Expert Parallel'), visible: true },
    ].filter(option => option.visible);
};

const getDefaultDataTypeOptions = (t: TFunction): Array<{label: string;value: string}> => {
    return [{ label: t('None'), value: 'None' }];
};

interface ParallelSwitchProps {
    session: Session;
    dimension?: GenerateConditions['dimension'] | null;
    generateConditions: GenerateConditions | null;
}

const ParallelSwitch = observer(({ session, dimension, generateConditions }: ParallelSwitchProps): JSX.Element => {
    const { t } = useTranslation('summary');
    const { parallelTypeList, setParallelTypeList, setDyeingMode, dyeingMode, startVal, endVal, setStartVal, setEndVal } = useParallelSwitchConditions();
    const parallelTypeOptions = useParallelTypeOptions(dimension, generateConditions);
    const [range, setRange] = useState<number[]>([]);

    const dataTypeOptions = useMemo(() => {
        const options = session.dataTypeOptions.map(indicator => {
            return { value: indicator.key, label: t(indicator.name) };
        });
        const commOptions = session.dynamicsIndicatorList.map(indicator => {
            return { value: indicator.key, label: `${indicator.key.toUpperCase()}-${t(indicator.name)}` };
        });
        return getDefaultDataTypeOptions(t).concat(options).concat(commOptions);
    }, [t, session.indicatorList, session.dynamicsIndicatorList]);

    useEffect(() => {
        const { min = null, max = null } = session.rankDyeingData[dyeingMode] ?? {};
        if (min !== null && max !== null) {
            setStartVal(min);
            setEndVal(max);
            setRange([min, max]);
        }
    }, [dyeingMode]);

    return (
        <>
            <Form layout="inline" data-testid="parallelSwitch">
                <Form.Item>
                    <CheckboxGroup
                        value={parallelTypeList}
                        options={parallelTypeOptions}
                        onChange={(checkedValues: CheckboxValueType[]): void => { setParallelTypeList(checkedValues as ParallelismType[]); }}
                    ></CheckboxGroup>
                </Form.Item>
                <Form.Item label={t('Data Type')}>
                    <Select id="dataType" defaultValue={dyeingMode} value={dyeingMode} style={{ width: '140px' }} onChange={(value: string): void => { setDyeingMode(value); }} options={dataTypeOptions}/>
                </Form.Item>
                {
                    dyeingMode !== 'None' &&
                <>
                    <Form.Item label={t('VisibleRange')}>
                        <InputGroup compact>
                            <InputNumber
                                data-testid="input-dyeing-minimum"
                                value={startVal}
                                min={range[0]}
                                max={range[1]}
                                step={1}
                                center
                                style={{ width: 100 }}
                                placeholder={t('Minimum')}
                                onChange={(value): void => { setStartVal(value as number); }}
                            />
                            <InputSplit placeholder="~" disabled />
                            <InputNumber
                                data-testid="input-dyeing-maximum"
                                value={endVal}
                                min={range[0]}
                                max={range[1]}
                                step={1}
                                center
                                style={{ width: 100, borderLeft: 0 }}
                                placeholder={t('Maximum')}
                                onChange={(value): void => { setEndVal(value as number); }}
                            />
                        </InputGroup>
                    </Form.Item>
                    <ColorScale min={startVal} max={endVal} />
                </>
                }
            </Form>
        </>
    );
});
