/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, observe } from 'mobx';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, Radio } from 'lib/components';
import type { RadioChangeEvent } from 'antd';
import { getUsableVal, delayExecute } from 'lib/CommonUtils';
import { Label } from '../Common';
import type { optionDataType, optionMapDataType, VoidFunction } from '../../utils/interface';
import { queryIterations, queryMatrixOperators, queryOperators, queryStages } from '../../utils/RequestUtils';
import type { Session } from '../../entity/session';

export interface ConditionDataType {
    [key: string]: string | string[];
    iterationId: string ;
    stage: string;
    operatorName: string ;
    type: string;
}
export const totalOperator = 'Total Op Info';
const defaultCondition = {
    iterationId: '',
    stage: '',
    operatorName: '',
    type: 'CommunicationMatrix',
};
const defaultOptionMap = {
    iterationOptions: [],
    operatorOptions: [],
    rankIdOptions: [],
    stageOptions: [],
};

const observeCondition = observable({ value: defaultCondition });

export function updateData(filterParams: ConditionDataType): void {
    observeCondition.value = filterParams;
}

const getOptionsAndValue = async (initObj: ConditionDataType, initOptionMap: optionMapDataType, key?: keyof ConditionDataType, val?: any):
Promise<{condition: ConditionDataType;optionMap: optionMapDataType}> => {
    if (key !== undefined) {
        const condition = { ...initObj, [key]: val };
        const optionMap = initOptionMap;
        if (key === 'iterationId') {
            const stageOptions: optionDataType[] = await getStageOptions(val);
            const stage = getUsableVal(initObj.stage, stageOptions, defaultCondition.stage);
            optionMap.stageOptions = stageOptions;
            condition.stage = stage;
        }
        if (['iterationId', 'stage', 'type'].includes(key as string)) {
            const operatorOptions: optionDataType[] = await getOperatorOptions(
                { iterationId: condition.iterationId, rankList: [], stage: condition.stage, type: condition.type });
            optionMap.operatorOptions = operatorOptions;
            condition.operatorName = getUsableVal(initObj.operatorName, operatorOptions, totalOperator);
        }
        return { optionMap, condition };
    }

    // iterationId（step）
    const iterationOptions: optionDataType[] = await getiterationOptions();
    const iterationId = getUsableVal(initObj.iterationId, iterationOptions, defaultCondition.iterationId);

    // stage
    const stageOptions: optionDataType[] = await getStageOptions(iterationId);
    const stage = getUsableVal(initObj.stage, stageOptions, defaultCondition.stage);

    // type
    const type = initObj.type ?? 'CommunicationMatrix';

    // Operator Name
    const operatorOptions: optionDataType[] =
        await getOperatorOptions({ iterationId, rankList: [], stage, type });
    const operatorName = getUsableVal(initObj.operatorName, operatorOptions, totalOperator);

    return { optionMap: { iterationOptions, stageOptions, operatorOptions }, condition: { iterationId, stage, type, operatorName } };
};

// 下拉可选项
const getiterationOptions = async(): Promise<optionDataType[]> => {
    const res: {iterationOrRankId: string[] } = await queryIterations();
    const list: string[] = res?.iterationOrRankId ?? [];
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};
const getStageOptions = async (iterationId: string): Promise<optionDataType[]> => {
    const res: {data: string[] } = await queryStages({ iterationId });
    const list = res?.data ?? [];
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};
const getOperatorOptions = async ({ iterationId, rankList, stage, type }: {iterationId: string;
    rankList: string[]; stage: string;type: string;}):
Promise<optionDataType[]> => {
    const res: {operatorName: string[] } = (type === 'CommunicationDurationAnalysis'
        ? await queryOperators({ iterationId, rankList, stage })
        : await queryMatrixOperators({ iterationId, stage }));
    const list = res?.operatorName ?? [];
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

const Filter = observer(({ session, handleFilterChange }: {session: Session;handleFilterChange: VoidFunction}) => {
    const [condition, setCondition] = useState<ConditionDataType>(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);
    const activeCommunicator = session.activeCommunicator?.value;

    const handleChange = (key: keyof ConditionDataType, val: any): void => {
        updateCondition(condition, key, val);
    };
    const updateCondition = async (initObj: ConditionDataType, key?: keyof ConditionDataType, val?: any): Promise<void> => {
        if (!session.clusterCompleted) {
            delayExecute(() => {
                if (!session.clusterCompleted) {
                    setCondition({ ...defaultCondition, type: condition.type });
                    setOptionMap(defaultOptionMap);
                }
            });
            return;
        }
        const { condition: newCondition, optionMap: newOptionMap } = await getOptionsAndValue(initObj, optionMap, key, val);
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    };

    useEffect(() => {
        observe(observeCondition, () => {
            updateCondition({ ...condition, ...observeCondition.value });
        });
    }, []);
    useEffect(() => {
        updateCondition(condition);
    }, [session.clusterCompleted]);
    useEffect(() => {
        setTimeout(() => {
            if (activeCommunicator !== undefined && activeCommunicator !== condition.stage) {
                updateCondition(condition, 'stage', activeCommunicator);
            }
        });
    }, [activeCommunicator]);
    useEffect(() => {
        handleFilterChange(condition);
    }, [condition]);

    return (<FilterCom handleChange={handleChange} condition={condition} optionMap={optionMap}/>);
});

interface IcomProps {
    optionMap: optionMapDataType;
    condition: ConditionDataType;
    handleChange: (key: keyof ConditionDataType, val: string) => void;
}
function FilterCom({ optionMap, condition, handleChange }: IcomProps): JSX.Element {
    const { t } = useTranslation('communication');
    return (<div>
        <FormItem
            name={t('searchCriteria.Step')}
            content={(<Select
                value={condition.iterationId}
                style={{ width: 120 }}
                onChange={(val: string): void => {
                    handleChange('iterationId', val);
                }}
                options={optionMap.iterationOptions}
            />
            )}/>
        <FormItem
            name={t('searchCriteria.CommunicationGroup')}
            content={(<Select
                value={condition.stage}
                style={{ width: 200 }}
                onChange={(val: string): void => {
                    handleChange('stage', val);
                }}
                options={optionMap.stageOptions}
            />
            )}/>
        <FormItem
            name={t('searchCriteria.OperatorName')}
            content={(
                <Select
                    value={condition.operatorName}
                    style={{ width: 300 }}
                    onChange={(val: string): void => {
                        handleChange('operatorName', val);
                    }}
                    options={optionMap.operatorOptions}
                    showSearch={true}
                />)}/>
        <FormItem content={(
            <Radio.Group value={condition.type}
                onChange={(e: RadioChangeEvent): void => {
                    handleChange('type', e.target.value);
                }}>
                <Radio value={'CommunicationMatrix'}>{t('searchCriteria.CommunicationMatrix')}</Radio>
                <Radio value={'CommunicationDurationAnalysis'}>{t('searchCriteria.CommunicationDurationAnalysis')}</Radio>
            </Radio.Group>)}/>
        <div>
        </div>
    </div>);
}

function FormItem(props: any): JSX.Element {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name}/>
        {props.content}
    </div>);
};

export default Filter;
