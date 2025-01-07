/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Form, InputNumber, Button, Select, Tooltip, CheckboxGroup, Tabs } from 'ascend-components';
import { message } from 'antd';
import type { CheckboxValueType } from 'antd/es/checkbox/Group';
import { QuestionCircleOutlined } from '@ant-design/icons';
import eventBus from '../../utils/eventBus';
import styled from '@emotion/styled';
import { FormInstance } from 'antd/lib/form';
import { ParallelismGraph } from './ParallelismGraph';
import { getParallelStrategy, setParallelStrategy } from '../../utils/RequestUtils';
import { ErrorInfo, ParallelismArrangementParams, ParallelismType } from '../../utils/interface';
import { ParallelSwitchConditionsProvider, useParallelSwitchConditions } from './Context';
import type { TFunction } from 'i18next';

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
        width: 150px;
        background-image: linear-gradient(to right, #24AB36, #d3eed7 45%, #f9d2d2 55%, #E32020);
    }
    .colorScaleNum {
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
    const [disabled, setDisabled] = useState(false);
    const [activeTab, setActiveTab] = useState(generateConditions.dimension);
    const { t } = useTranslation('summary');
    const dimensionOptions = getDimensionOptions(t);
    const init = async (): Promise<void> => {
        const { dpSize, tpSize, ppSize, cpSize, epSize, level, algorithm } = await getParallelStrategy();
        const equal = dpSize === 1 && tpSize === 1 && tpSize === 1 && cpSize === 1;
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
        <FormDom disabled={disabled} onClickGenerate={clickGenerate} form={form}/>
        {showRank && <Tabs
            activeKey={activeTab}
            onChange={handleTabChange}
            items={dimensionOptions}
        />}
    </>;
});

const PARALLEL_STRATEGY_INPUT_PROPS = { min: 0, max: 255, style: { width: '80px' } };
const selectOptions = [
    { value: 'megatron-lm(tp-cp-ep-dp-pp)', label: 'Megatron-LM(tp-cp-ep-dp-pp)' },
    { value: 'megatron-lm(tp-cp-pp-ep-dp)', label: 'Megatron-LM(tp-cp-pp-ep-dp)' },
];
const getDimensionOptions = (t: TFunction): Array<{key: string; label: string}> => [
    { key: 'ep-dp', label: `DP ${t('Dimension')}` },
    { key: 'ep-dp-cp', label: `CP ${t('Dimension')}` },
    { key: 'ep-dp-cp-pp', label: `PP ${t('Dimension')}` },
    { key: 'ep-dp-cp-pp-tp', label: `TP ${t('Dimension')}` },
];

const FormDom = ({ disabled, onClickGenerate, form }:
{disabled: boolean; onClickGenerate: () => void; form: FormInstance<any> }): JSX.Element => {
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
                <Tooltip placement="right" title={disabled ? t('ProhibitConfiguration') : ''}>
                    <div style={{ width: '70px' }}>
                        <Button type="primary" style={{ margin: '10px 32px 10px 0' }} disabled={disabled} onClick={onClickGenerate}>{t('Generate')}</Button>
                    </div>
                </Tooltip>
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
            <ParallelSwitch session={session} dimension={generateConditions?.dimension} />
            <ParallelismGraph session={session} generateConditions={generateConditions} />
            <LegendContainer/>
        </ParallelSwitchConditionsProvider>
    );
});

const ColorScale = ({ session, dyeingMode }: { session: Session; dyeingMode: string }): JSX.Element => {
    const colorScaleData = session.rankDyeingData[dyeingMode];
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

const useParallelTypeOptions = (dimension: ParallelSwitchProps['dimension']): Array<{label: string;value: string; disabled?: boolean}> => {
    const { t } = useTranslation('summary');
    const val = dimension ?? '';
    const ppDisabled = ['ep-dp', 'ep-dp-cp'].includes(val);
    const tpDisabled = ['ep-dp', 'ep-dp-cp', 'ep-dp-cp-pp'].includes(val);
    const cpDisabled = ['ep-dp'].includes(val);

    return [
        { value: 'pp', label: t('Pipeline Parallel'), disabled: ppDisabled },
        { value: 'tp', label: t('Tensor Parallel'), disabled: tpDisabled },
        { value: 'cp', label: t('Context Parallel'), disabled: cpDisabled },
        { value: 'dp', label: t('Data Parallel') },
        { value: 'ep', label: t('Expert Parallel') },
    ];
};

const getDefaultDataTypeOptions = (t: TFunction): Array<{label: string;value: string}> => {
    return [{ label: t('None'), value: 'None' }];
};

interface ParallelSwitchProps {
    session: Session;
    dimension?: GenerateConditions['dimension'] | null;
}

const ParallelSwitch = observer(({ session, dimension }: ParallelSwitchProps): JSX.Element => {
    const { t } = useTranslation('summary');
    const { parallelTypeList, setParallelTypeList, setDyeingMode, dyeingMode, dyeingStep, setDyeingStep } = useParallelSwitchConditions();
    const parallelTypeOptions = useParallelTypeOptions(dimension);

    const dataTypeOptions = useMemo(() => {
        const options = session.dataTypeOptions.map(indicator => {
            return { value: indicator.key, label: t(indicator.name) };
        });
        return getDefaultDataTypeOptions(t).concat(options);
    }, [t, session.indicatorList]);

    return (
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
                <Form.Item label={(<DyeingTipAndLabel/>)}>
                    <InputNumber
                        defaultValue={dyeingStep}
                        min={0.01}
                        max={0.1}
                        step={0.01}
                        maxLength={4}
                        style={{ width: '120px' }}
                        onChange={(value): void => { setDyeingStep(value as number); }}
                    />
                </Form.Item>
            }
            <ColorScale session={session} dyeingMode={dyeingMode} />
        </Form>
    );
});

const DyeingTipAndLabel = (): JSX.Element => {
    const { t } = useTranslation('summary');
    const hit = t('DyeingTooltip', { returnObjects: true }) as string[];
    return <div style={{ display: 'flex' }}>
        <div style={{ marginRight: 5 }}>{t('DyeingStep')}</div>
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
    </div>;
};
