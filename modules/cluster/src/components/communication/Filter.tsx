/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, observe } from 'mobx';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, Radio } from 'ascend-components';
import type { RadioChangeEvent } from 'antd';
import { getUsableVal, delayExecute } from 'ascend-utils';
import { Label } from '../Common';
import type { optionDataType, optionMapDataType, VoidFunction } from '../../utils/interface';
import { queryIterations, queryMatrixOperators, queryOperators, queryStages } from '../../utils/RequestUtils';
import type { Session } from '../../entity/session';
import type { partitionMode } from '../communicatorContainer/ContainerUtils';
import _ from 'lodash';

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

function getDefaultStage(inputArray: Array<{value: string | number}>): string | number {
    if (inputArray.length === 0) {
        return '';
    }
    // 寻找第一个通信域非单个的情况
    const result = inputArray.find(item =>
        getAllNumberFromString(item.value as string).length > 1 || !isNumberPairsFormat(item.value as string));
    if (!result) {
        return inputArray[0].value;
    }
    return result.value;
}

const getOptionsAndValue = async (session: Session, initObj: ConditionDataType, initOptionMap: optionMapDataType, key?: keyof ConditionDataType, val?: any):
Promise<{condition: ConditionDataType;optionMap: optionMapDataType}> => {
    if (key !== undefined) {
        const condition = { ...initObj, [key]: val };
        const optionMap = initOptionMap;
        if (key === 'iterationId') {
            const stageOptions: optionDataType[] = await getStageOptions(val, session);
            const stage = getUsableVal(initObj.stage, stageOptions, defaultCondition.stage, getDefaultStage);
            optionMap.stageOptions = stageOptions;
            condition.stage = stage.toString();
        }
        if (['iterationId', 'stage', 'type'].includes(key as string)) {
            const operatorOptions: optionDataType[] = await getOperatorOptions(
                { iterationId: condition.iterationId, rankList: [], stage: condition.stage, type: condition.type });
            optionMap.operatorOptions = operatorOptions;
            condition.operatorName = getUsableVal(initObj.operatorName, operatorOptions, totalOperator).toString();
        }
        return { optionMap, condition };
    }

    // iterationId（step）
    const iterationOptions: optionDataType[] = await getiterationOptions();
    const iterationId = getUsableVal(initObj.iterationId, iterationOptions, defaultCondition.iterationId).toString();

    // stage
    const stageOptions: optionDataType[] = await getStageOptions(iterationId, session);
    const stage = getUsableVal(initObj.stage, stageOptions, defaultCondition.stage, getDefaultStage).toString();

    // type
    const type = initObj.type ?? defaultCondition.type;

    // Operator Name
    const operatorOptions: optionDataType[] =
        await getOperatorOptions({ iterationId, rankList: [], stage, type });
    const operatorName = getUsableVal(initObj.operatorName, operatorOptions, totalOperator).toString();

    return { optionMap: { iterationOptions, stageOptions, operatorOptions }, condition: { iterationId, stage, type, operatorName } };
};

// 下拉可选项
const getiterationOptions = async(): Promise<optionDataType[]> => {
    const res: {iterationOrRankId: string[] } = await queryIterations();
    const list: string[] = res?.iterationOrRankId ?? [];
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

/**
 * 校验字符串是否为(1, 2, 3.....)格式
 * @param str
 */
function isNumberPairsFormat(str: string): boolean {
    const pattern: RegExp = /^\(\s*\d+(?:\s*,\s*\d+)*(?:\s*,)?\s*\)$/;
    return pattern.test(str);
}

// 通过stage获取对应的并行策略，存在多个并行策略时，策略间使用'/'进行分割
function getParallelStrategyByStage(partitionModes: partitionMode[], stage: string): string {
    const strategyList: string[] = ['pp', 'tp', 'dp', 'tpOrDp'];
    const matchingKeys: string[] = [];
    partitionModes.forEach((obj) => {
        const found = obj.communicators.some(communicator => communicator.value === stage);
        if (found && strategyList.includes(obj.mode)) {
            matchingKeys.push(obj.mode);
        }
    });
    if (matchingKeys.length === 0) {
        return '';
    } else {
        return matchingKeys.join('/');
    }
}

// 从字符串中获取所有数字
function getAllNumberFromString(str: string): number[] {
    // 使用正则表达式提取所有数字
    const matches = str.match(/\d+/g);

    // 如果没有匹配到数字，返回空数组
    if (!matches) {
        return [];
    }

    // 将匹配到的数字字符串转换为数字
    return matches.map(Number);
}

// stage中的rank内容可能并没有排序，该方法对rank内容进行从小到大排序处理
function sortStageNumber(stage: string): string {
    const numbers = getAllNumberFromString(stage);
    numbers.sort((a: number, b: number) => a - b);
    return `(${_.join(numbers, ', ')}${(numbers.length > 1 ? ')' : ',)')}`;
}

// 对比stage数据，先对长度进行排序（长的排前面），再对内容排序（首数字小的排前面）
function compareStageInfo(stageA: string, stageB: string): number {
    const aRankList = getAllNumberFromString(stageA);
    const bRankList = getAllNumberFromString(stageB);
    if (aRankList.length !== bRankList.length) {
        return aRankList.length <= bRankList.length ? 1 : -1;
    }

    for (let i = 0; i < aRankList.length; ++i) {
        if (aRankList[i] < bRankList[i]) {
            return -1;
        }
        if (aRankList[i] > bRankList[i]) {
            return 1;
        }
    }
    return 0;
}

const getStageOptions = async (iterationId: string, session: Session): Promise<optionDataType[]> => {
    const res: {data: string[] } = await queryStages({ iterationId });
    const list = res?.data ?? [];
    const modes = session.communicatorData.partitionModes;
    const options: optionDataType[] = list
        .map(item => {
            // 如果stage是由数字组成，则对数据进行重排序，否则不做任何处理（p2p的情况）
            const stageAfterSort = isNumberPairsFormat(item) ? sortStageNumber(item) : item;
            const strategy = getParallelStrategyByStage(modes, stageAfterSort);
            const label = strategy.length === 0 ? stageAfterSort : `${strategy}:${stageAfterSort}`;
            return { value: item, strategy, label, stageAfterSort };
        })
        .sort((a, b) => {
            // 存在并行策略的排在前面
            if (a.strategy && !b.strategy) {
                return -1;
            }
            if (!a.strategy && b.strategy) {
                return 1;
            }
            // 如果都存在并行策略，且策略相同，则按通信域中rank数量进行排序（从大到小），数量相同，则根据数字对位比较排序（小数字排前）
            if (a.strategy && b.strategy && a.strategy === b.strategy) {
                return compareStageInfo(a.stageAfterSort, b.stageAfterSort);
            }
            // 如果都存在并行策略，但策略不同，则按策略的字母排序
            if (a.strategy && b.strategy && a.strategy !== b.strategy) {
                return a.strategy > b.strategy ? 1 : -1;
            }
            // 如果都没有并行策略，则按通信域中rank数量进行排序（从大到小），数量相同，则根据数字对位比较排序（小数字排前）
            if (!a.strategy && !b.strategy) {
                return compareStageInfo(a.stageAfterSort, b.stageAfterSort);
            }
            // 其他情况保持原有顺序
            return 0;
        });
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
        const { condition: newCondition, optionMap: newOptionMap } = await getOptionsAndValue(session, initObj, optionMap, key, val);
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
    }, [session.clusterCompleted, session.communicatorData.partitionModes]);
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
            />)}/>
        <FormItem
            name={t('searchCriteria.CommunicationGroup')}
            content={(<Select
                value={condition.stage}
                style={{ width: 200 }}
                onChange={(val: string): void => {
                    handleChange('stage', val);
                }}
                filterOption={(input: any, option: any): boolean => (option?.label as string).toLowerCase().includes(input.toLowerCase())}
                options={optionMap.stageOptions}
                showSearch={true}
            />)}/>
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
