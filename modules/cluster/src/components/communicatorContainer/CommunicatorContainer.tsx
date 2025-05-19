/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { ReactNode, useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, Form, InputGroup, InputNumber, InputSplit, Select, Tabs, Tooltip } from 'ascend-components';
import { message, Popconfirm, Spin } from 'antd';
import eventBus from '../../utils/eventBus';
import styled from '@emotion/styled';
import { FormInstance } from 'antd/lib/form';
import { Loading, ParallelismGraph } from './ParallelismGraph';
import { getParallelStrategy, setParallelStrategy } from '../../utils/RequestUtils';
import { ErrorInfo, ParallelismArrangementParams } from '../../utils/interface';
import { ParallelSwitchConditionsProvider, useParallelSwitchConditions } from './Context';
import type { TFunction } from 'i18next';
import { COLOR } from '../Common';
import cls from 'classnames';
import { isEqual } from 'lodash';
import { AimOutlined } from '@ant-design/icons';

const ParallelismGraphHeader = styled.div`
    display: grid;
    grid-template-columns: 1fr auto;
    align-items: center;
    gap: 20px;
    padding: 20px 0 10px;
`;

const Legend = styled.div`
    .legendContainer {
        display: flex;
        justify-content: center;
        flex-wrap: wrap;
        gap: 8px;
        user-select: none;

        .legendItem {
            display: flex;
            height: 12px;
            line-height: 12px;
            margin: 0 4px;
            cursor: pointer;
            color: ${(props): string => props.theme.textColor};

            &.disabled {
                cursor: not-allowed;
                color: ${(props): string => props.theme.textColorDisabled};

                .legendColor {
                    opacity: 0.2;
                }
            }

            &:not(.disabled):hover {
                opacity: 0.9;
            }

            .legendColor {
                padding: 2px;
                flex: none;
                width: 24px;
                margin-right: 4px;
                border: 2px solid;
                border-radius: 2px;

                .legendColorContent {
                    height: 100%
                }
            }
            .legendLabel {
                flex: none;
                font-size: 12px;
            }
        }
    }
`;

const ColorScaleContainer = styled.div<{ equal: boolean }>`
    display: flex;
    align-items: center;
    color: ${(props): string => props.theme.svgPlayBackgroundColor};
    .colorScale {
        width: 200px;
        height: 16px;
        background: ${(props): string => props.equal
        ? COLOR.BAND_1
        : `linear-gradient(to right, ${COLOR.BAND_3}, ${COLOR.BAND_2},${COLOR.BAND_1},${COLOR.BAND_0})`}
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
    loading: boolean;
}

export const CommunicatorContainer = observer(({ session, generateConditions, onGenerateConditionsChange, loading }: CommunicatorContainerProps) => {
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
                    ? <div style={{ position: 'relative' }}>
                        {
                            loading &&
                            <Loading style={{ paddingTop: 100 }}>
                                <Spin spinning={loading} />
                            </Loading>
                        }
                        <ParallelSwitchConditionsProvider>
                            <CommunicatorContent
                                session={session}
                                generateConditions={generateConditions}
                            />
                        </ParallelSwitchConditionsProvider>
                    </div>
                    : <div className={'noDataTip'}>{t('NoDataTip')}</div>
            }
        </div>
    );
});

const DimensionTabExtraContent = (): JSX.Element => {
    const { t } = useTranslation('summary');
    const content = t('DimensionTooltipContent', { returnObjects: true }) as string[];
    const tooltip = content.map((item, index) => <div style={{ padding: '6px 0' }} key={index}>{item}</div>);

    return <Form.Item style={{ marginBottom: 0 }} label={t('Parallel Dimension')} tooltip={<div>{tooltip}</div>}></Form.Item>;
};

export type GenerateConditions = ParallelismArrangementParams;
interface CommunicatorHeaderProps {
    session: Session;
    showRank: boolean;
    setShowRank: React.Dispatch<React.SetStateAction<boolean>>;
    generateConditions: GenerateConditions;
    setGenerateConditions: (params: GenerateConditions) => void;
}
interface CollectedConfiguration {
    dpSize: number;
    ppSize: number;
    tpSize: number;
    epSize: number;
    cpSize: number;
    moeTpSize: number;
}
const CommunicatorHeader = observer(({ session, showRank, setShowRank, generateConditions, setGenerateConditions }: CommunicatorHeaderProps) => {
    const [form] = Form.useForm();
    const [activeTab, setActiveTab] = useState(generateConditions.dimension);
    const collectedConfiguration = useRef<CollectedConfiguration | null>(null);
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
    }, [generateConditions.cpSize, t]);

    const init = async (): Promise<void> => {
        const { dpSize, tpSize, ppSize, cpSize, epSize, moeTpSize = 1, level, algorithm } = await getParallelStrategy();
        const equal = dpSize === 1 && tpSize === 1 && tpSize === 1 && cpSize === 1;
        if (level === 'collected') {
            collectedConfiguration.current = {
                dpSize, tpSize, ppSize, cpSize, epSize, moeTpSize: moeTpSize ?? 1,
            };
        } else {
            collectedConfiguration.current = null;
        }
        if (level === 'undefined' || equal) {
            setShowRank(false);
        } else {
            setShowRank(true);
        }
        form.setFieldsValue({ dpSize, tpSize, ppSize, cpSize, epSize, moeTpSize, algorithm });
        setGenerateConditions({ algorithm, dimension: activeTab, ppSize, tpSize, cpSize, dpSize, epSize, moeTpSize });
        eventBus.emit('activeCommunicator', undefined);
    };

    useEffect(() => {
        init();
    }, []);

    const clickGenerate = async (): Promise<void> => {
        const formData: GenerateConditions = form.getFieldsValue();

        // 由于选择不同 algorithm，moeTpSize 或 cpSize 可能隐藏，导致获取不到对应的值，此处单独处理
        const values: GenerateConditions = {
            ...formData,
            moeTpSize: formData.algorithm === 'mindie-llm(tp-dp-ep-pp-moetp)' ? formData.moeTpSize : 1,
            cpSize: formData.algorithm === 'mindie-llm(tp-dp-ep-pp-moetp)' ? 1 : formData.cpSize,
        };
        // 设置的并行策略乘积需要>=导入的卡数
        if (values.algorithm !== 'mindie-llm(tp-dp-ep-pp-moetp)') {
            if (values.dpSize * values.tpSize * values.ppSize * values.cpSize < session.rankCount) {
                message.error('The product of DP, CP, TP and PP size must be greater than the Device Count');
                return;
            }
        }
        try {
            await setParallelStrategy({ ...values });
            setGenerateConditions({ ...values, dimension: activeTab });
            setShowRank(true);
            eventBus.emit('activeCommunicator', undefined);
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
        <FormDom collectedConfiguration={collectedConfiguration} onClickGenerate={clickGenerate} form={form}/>
        {showRank && <Tabs
            type="card"
            size="small"
            tabBarGutter={4}
            tabBarExtraContent={{ left: <DimensionTabExtraContent /> }}
            activeKey={activeTab}
            onChange={handleTabChange}
            items={dimensionOptions}
        />}
    </>;
});

const PARALLEL_STRATEGY_INPUT_PROPS = { min: 1, max: 10000, style: { width: '80px' } };
const selectOptions = [
    { value: 'megatron-lm(tp-cp-ep-dp-pp)', label: 'Megatron-LM (tp-cp-ep-dp-pp)' },
    { value: 'megatron-lm(tp-cp-pp-ep-dp)', label: 'Megatron-LM (tp-cp-pp-ep-dp)' },
    { value: 'mindspeed(tp-cp-ep-dp-pp)', label: 'MindSpeed (tp-cp-ep-dp-pp)' },
    { value: 'mindie-llm(tp-dp-ep-pp-moetp)', label: 'MindIE-LLM (tp-dp-ep-pp-moetp)' },
];
const getDimensionOptions = (t: TFunction, generateConditions: GenerateConditions): Array<{key: string; label: ReactNode}> => {
    const { cpSize } = generateConditions;
    return [
        { key: 'ep-dp', label: <Tooltip title={t('DPDimensionTooltip')}>{'DP'}</Tooltip> },
        { key: 'ep-dp-pp', label: <Tooltip title={t('PPDimensionTooltip')}>{'DP + PP'}</Tooltip> },
        { key: 'ep-dp-pp-cp', label: <Tooltip title={t('CPDimensionTooltip')}>{'DP + PP + CP'}</Tooltip> },
        {
            key: 'ep-dp-pp-cp-tp',
            label: cpSize === 1
                ? <Tooltip title={t('TPDimensionTooltip')}>{'DP + PP + TP'}</Tooltip>
                : <Tooltip title={t('TPDimensionTooltip')}>{'DP + PP + CP + TP'}</Tooltip>,
        },
    ];
};

const FormDom = (
    {
        collectedConfiguration,
        onClickGenerate,
        form,
    }:
    {
        collectedConfiguration: React.MutableRefObject<CollectedConfiguration | null>;
        onClickGenerate: () => void;
        form: FormInstance<any>;
    },
): JSX.Element => {
    const { t } = useTranslation('summary');
    const algorithm = Form.useWatch('algorithm', form);
    const [popconfirmVisible, setPopconfirmVisible] = useState(false);

    const handleValueChange = useCallback((): void => {
        if (collectedConfiguration.current === null) {
            setPopconfirmVisible(false);
            return;
        }

        const mismatchCollectedConfiguration = !isEqual(collectedConfiguration.current, form.getFieldsValue(['ppSize', 'tpSize', 'cpSize', 'dpSize', 'epSize', 'moeTpSize']));
        setPopconfirmVisible(mismatchCollectedConfiguration);
    }, []);

    return <Form
        data-testId="form-generate-parallelism"
        form={form}
        layout="inline"
        initialValues={{
            algorithm: 'megatron-lm(tp-cp-ep-dp-pp)',
        }}
        onValuesChange={handleValueChange}
        onFinish={onClickGenerate}
    >
        <Form.Item name={'algorithm'} label={t('Algorithm')} tooltip={t('AlgorithmTooltip')}>
            <Select defaultValue="megatron-lm(tp-cp-ep-dp-pp)" style={{ width: 220 }} options={selectOptions}/>
        </Form.Item>
        <Form.Item name={'ppSize'} label={t('PPSize')}>
            <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
        </Form.Item>
        <Form.Item name={'tpSize'} label={t('TPSize')}>
            <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
        </Form.Item>
        {
            algorithm === 'mindie-llm(tp-dp-ep-pp-moetp)'
                ? <Form.Item name={'moeTpSize'} label={t('MOE-TP Size')}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
                : <Form.Item name={'cpSize'} label={t('CPSize')}>
                    <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
                </Form.Item>
        }

        <Form.Item name={'dpSize'} label={t('DPSize')}>
            <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
        </Form.Item>
        <Form.Item name={'epSize'} label={t('EPSize')}>
            <InputNumber {...PARALLEL_STRATEGY_INPUT_PROPS}/>
        </Form.Item>
        <Form.Item>
            <Popconfirm
                placement="right"
                disabled={!popconfirmVisible}
                title={<div style={{ maxWidth: 400 }}>{t('GenerateConfirm')}</div>}
                onConfirm={onClickGenerate}
            >
                <Button
                    type="primary"
                    htmlType="submit"
                    onClick={popconfirmVisible ? undefined : onClickGenerate}
                >
                    {t('Generate')}
                </Button>
            </Popconfirm>
        </Form.Item>
    </Form>;
};

interface CommunicatorContentProps {
    session: Session;
    generateConditions: GenerateConditions | null;
}

const CommunicatorContent = observer(({ session, generateConditions }: CommunicatorContentProps) => {
    const { dyeingMode, startVal, endVal } = useParallelSwitchConditions();
    const [targetRankIndex, setTargetRankIndex] = useState<number | null>(null);
    const [targetTrigger, setTargetTrigger] = useState<boolean>(false);

    const handleChange = useCallback((index: number | null): void => {
        setTargetRankIndex(index);
        setTargetTrigger((prevState) => !prevState);
    }, []);

    return (
        <>
            <ParallelSwitch session={session} onTargetRankIndexChange={handleChange} />
            <ParallelismGraphHeader>
                <LegendContainer generateConditions={generateConditions} />
                <div>
                    {dyeingMode !== 'None' && <ColorScale min={startVal} max={endVal}/>}
                </div>
            </ParallelismGraphHeader>
            <ParallelismGraph
                session={session}
                generateConditions={generateConditions}
                targetRankIndex={targetRankIndex}
                targetTrigger={targetTrigger}
            />
        </>
    );
});

const ColorScale = ({ min, max }: { min: number | null; max: number | null }): JSX.Element => {
    const isRangeEmpty = min === null || max === null;
    return isRangeEmpty
        ? <></>
        : <ColorScaleContainer equal={min === max}>
            <div className="colorScaleNum">{min}</div>
            <div className="colorScale"></div>
            <div className="colorScaleNum">{max}</div>
        </ColorScaleContainer>
    ;
};

interface LegendItem {
    label: string;
    value: 'ep' | 'dp' | 'cp' | 'pp' | 'tp' | 'moeTp';
    color: string;
    checked: boolean;
    visible: boolean;
    disabled: boolean;
}

const defaultLegendItemList: LegendItem[] = [
    { value: 'pp', label: 'Pipeline Parallelism', color: '#0277FF', checked: true, disabled: false, visible: true },
    { value: 'tp', label: 'Tensor Parallelism', color: '#01CEB0', checked: true, disabled: false, visible: true },
    { value: 'cp', label: 'Context Parallelism', color: '#FD2F2F', checked: true, disabled: false, visible: true },
    { value: 'dp', label: 'Data Parallelism', color: '#6948C9', checked: true, disabled: false, visible: true },
    { value: 'ep', label: 'Expert Parallelism', color: '#EE891D', checked: true, disabled: false, visible: true },
    { value: 'moeTp', label: 'MOE Tensor Parallelism', color: '#D53F78', checked: true, disabled: false, visible: true },
];

interface LegendContainerProps {
    generateConditions: GenerateConditions | null;
}

const LegendContainer = ({ generateConditions }: LegendContainerProps): JSX.Element => {
    const { t } = useTranslation('summary');
    const { parallelTypeList, setParallelTypeList } = useParallelSwitchConditions();
    const [parallelTypeOptions, setParallelTypeOptions] = useState<LegendItem[]>(defaultLegendItemList);
    const { cpSize } = generateConditions ?? {};

    useEffect(() => {
        const { dimension = '', algorithm = '' } = generateConditions ?? {};
        const ppDisabled = ['ep-dp'].includes(dimension);
        const tpDisabled = ['ep-dp', 'ep-dp-pp', 'ep-dp-pp-cp'].includes(dimension);
        const cpDisabled = ['ep-dp', 'ep-dp-pp'].includes(dimension);
        const epDisabled = ['ep-dp', 'ep-dp-pp'].includes(dimension) && algorithm === 'mindie-llm(tp-dp-ep-pp-moetp)';

        const options = parallelTypeOptions.map((option) => {
            const disabled = (option.value === 'pp' && ppDisabled) ||
                (option.value === 'tp' && tpDisabled) ||
                (option.value === 'cp' && cpDisabled) ||
                (option.value === 'ep' && epDisabled) ||
                option.value === 'moeTp';
            return {
                ...option,
                checked: parallelTypeList.includes(option.value),
                disabled,
                visible: option.value !== 'cp' || (option.value === 'cp' && cpSize !== 1),
            };
        });

        setParallelTypeOptions(options);
    }, [JSON.stringify(generateConditions), parallelTypeList]);

    const handleClickLegend = (item: LegendItem): void => {
        if (item.disabled) {
            return;
        }

        item.checked = !item.checked;
        const list = parallelTypeOptions.filter(option => option.checked).map(option => option.value);

        setParallelTypeOptions(parallelTypeOptions);
        setParallelTypeList(list);
    };

    return (
        <Legend>
            <div className="legendContainer">
                {
                    parallelTypeOptions.filter(item => item.visible)
                        .map(item => (
                            <div
                                className={cls('legendItem', {
                                    checked: item.checked,
                                    disabled: item.disabled,
                                })}
                                key={item.value}
                                onClick={(): void => handleClickLegend(item)}
                            >
                                <div className="legendColor" style={{ borderColor: item.color }}>
                                    <div className="legendColorContent" style={{ backgroundColor: item.checked ? item.color : 'unset' }}></div>
                                </div>
                                <div className="legendLabel">{t(item.label)}</div>
                            </div>
                        ))
                }
            </div>
        </Legend>
    );
};

const getDefaultDataTypeOptions = (t: TFunction): Array<{label: string;value: string}> => {
    return [{ label: t('None'), value: 'None' }];
};

interface ParallelSwitchProps {
    session: Session;
    onTargetRankIndexChange: (rankIndex: number | null) => void;
}

const ParallelSwitch = observer(({ session, onTargetRankIndexChange }: ParallelSwitchProps): JSX.Element => {
    const { t } = useTranslation('summary');
    const { setDyeingMode, dyeingMode, startVal, endVal, setStartVal, setEndVal, rankIndex, setRankIndex } = useParallelSwitchConditions();
    const [range, setRange] = useState<number[]>([]);
    const [unit, setUnit] = useState('');

    const dataTypeOptions = useMemo(() => {
        const options = session.dataTypeOptions.map(indicator => {
            return { value: indicator.key, label: t(indicator.name) };
        });
        const commOptions = session.dynamicsIndicatorList.map(indicator => {
            return { value: indicator.key, label: `${indicator.key.toUpperCase()}-${t(indicator.name)}` };
        });
        return getDefaultDataTypeOptions(t).concat(options).concat(commOptions);
    }, [t, session.indicatorList, session.dynamicsIndicatorList]);

    const handleFindRank = useCallback((targetIndex: number | null) => {
        if (targetIndex === null) {
            return;
        }
        onTargetRankIndexChange(targetIndex);
    }, [onTargetRankIndexChange]);

    useEffect(() => {
        const { min = null, max = null } = session.rankDyeingData[dyeingMode] ?? {};
        const activeUnit = session.indicatorMap.get(dyeingMode)?.unit ?? session.dynamicsIndicatorMap.get(dyeingMode)?.unit ?? '';

        setUnit(activeUnit);
        if (min !== null && max !== null) {
            setStartVal(min);
            setEndVal(max);
            setRange([min, max]);
        }
    }, [dyeingMode]);

    return (
        <div className="flex items-center">
            <Form layout="inline" data-testid="parallelSwitch">
                <Form.Item label={t('Performance Metric')}>
                    <Select id="dataType" defaultValue={dyeingMode} value={dyeingMode} style={{ width: '140px' }} onChange={(value: string): void => { setDyeingMode(value); }} options={dataTypeOptions}/>
                </Form.Item>
                {
                    dyeingMode !== 'None' &&
                        <Form.Item label={`${t('VisibleRange')} (${unit})`}>
                            <InputGroup compact>
                                <InputNumber
                                    data-testid="input-dyeing-minimum"
                                    value={startVal}
                                    min={range[0]}
                                    max={Math.min(range[1], endVal ?? range[1])}
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
                                    min={Math.max(range[0], startVal ?? range[0])}
                                    max={range[1]}
                                    step={1}
                                    center
                                    style={{ width: 100, borderLeft: 0 }}
                                    placeholder={t('Maximum')}
                                    onChange={(value): void => { setEndVal(value as number); }}
                                />
                            </InputGroup>
                        </Form.Item>
                }
            </Form>
            <Form layout="inline" onFinish={(): void => handleFindRank(rankIndex)}>
                <Form.Item label={t('Target Index')}>
                    <InputNumber
                        value={rankIndex}
                        min={0}
                        step={1}
                        style={{ width: 80 }}
                        onChange={(value): void => { setRankIndex(value as number); }}
                    />
                </Form.Item>
                <Form.Item>
                    <Button
                        type="primary"
                        htmlType="submit"
                        icon={<AimOutlined />}
                        disabled={rankIndex === null}
                    >
                        {t('Find')}
                    </Button>
                </Form.Item>
            </Form>
        </div>
    );
});
