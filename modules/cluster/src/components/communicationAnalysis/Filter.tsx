/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { useEffect } from 'react';
import { Select, Radio } from 'antd';
import { Label } from '../Common';
import { optionDataType, optionMapDataType, VoidFunction } from '../../utils/interface';
import { queryIterations, queryOperators, queryRanks, queryStages } from '../../utils/RequestUtils';
import { Session } from '../../entity/session';

export interface ConditionDataType{
    iterationId: string ;
    rankIds: string[];
    operatorName: string ;
    type?: string;
    stage: string;
}
export const totalOperator = 'Total Op Info';
const defaultCondition = {
    iterationId: '',
    stage: '',
    rankIds: [],
    operatorName: '',
    type: 'CommunicationDurationAnalysis',
};
const defaultOptionMap = {
    iterationOptions: [],
    operatorOptions: [],
    rankIdOptions: [],
    stageOptions: [],
};
const conditions: ConditionDataType = observable(defaultCondition);
const optionMap: optionMapDataType = observable(defaultOptionMap);

const setOptions = async(initObj = {} as ConditionDataType): Promise<void> => {
    // step
    const iterationOptions: optionDataType[] = await getiterationOptions();
    const iterationId = initObj.iterationId || iterationOptions[0]?.value as string;
    runInAction(() => {
        optionMap.iterationOptions = iterationOptions;
        conditions.iterationId = iterationId;
    });
    // stage
    const stageOptions: optionDataType[] = await getStageOptions(iterationId);
    const stage = initObj.stage || stageOptions[0]?.value as string;
    runInAction(() => {
        optionMap.stageOptions = stageOptions;
        conditions.stage = stage;
    });

    // Operator Name
    const operatorOptions: optionDataType[] =
        await getOperatorOptions({ iterationId, rankList: [], stage });
    const operatorName = initObj.operatorName || totalOperator;
    runInAction(() => {
        optionMap.operatorOptions = operatorOptions;
        conditions.operatorName = operatorName;
    });
};

// 下拉可选项
const getiterationOptions = async(): Promise<optionDataType[]> => {
    const res: {iterationOrRankId: string[] } = await queryIterations();
    const list: string[] = res.iterationOrRankId;
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};
const getStageOptions = async (iterationId: string): Promise<optionDataType[]> => {
    const res: {data: string[] } = await queryStages({ iterationId });
    const list = res.data;
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};
const getOperatorOptions = async ({ iterationId, rankList, stage }: {iterationId: string;
    rankList: string[]; stage: string;}):
Promise<optionDataType[]> => {
    const res: {operatorName: string[] } = await queryOperators({ iterationId, rankList, stage });
    const list = res.operatorName;
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

export const getRankIdOptions = async (iterationId: string): Promise<optionDataType[]> => {
    const res: {iterationsOrRanks: Array<{rank_id: string } > } = await queryRanks({ iterationId });
    const list = res.iterationsOrRanks.map(item => item.rank_id);
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

const handleChange = (key: keyof ConditionDataType, val: any): void => {
    runInAction(() => {
        conditions[key] = val;
    });
    handleRelatedChange(key, val);
};

const handleRelatedChange = async (source: string, value: string): Promise<void> => {
    if (source === 'iterationId') {
        const stageOptions: optionDataType[] = await getStageOptions(value);
        const stage = conditions.stage || stageOptions[0]?.value as string;
        runInAction(() => {
            optionMap.stageOptions = stageOptions;
            conditions.stage = stage;
        });
    }
    if ([ 'iterationId', 'stage' ].includes(source)) {
        const operatorOptions: optionDataType[] = await getOperatorOptions(
            { iterationId: conditions.iterationId, rankList: [], stage: conditions.stage });
        runInAction(() => {
            optionMap.operatorOptions = operatorOptions;
            if (!operatorOptions.map(item => item.value).includes(conditions.operatorName)) {
                conditions.operatorName = totalOperator;
            }
        });
    }
};

const Filter = observer(({ session, handleFilterChange }: {session: Session;handleFilterChange: VoidFunction}) => {
    const activeCommunicator = session.activeCommunicator?.value;
    // 初始化
    useEffect(() => {
        setOptions();
        observe(conditions, (change) => {
            handleFilterChange(conditions);
        });
    }, [ ]);
    useEffect(() => {
        setOptions({ stage: activeCommunicator } as ConditionDataType);
    }, [session.allRankIds]);
    useEffect(() => {
        setTimeout(() => {
            if (activeCommunicator !== undefined && activeCommunicator !== conditions.stage) {
                setOptions({ ...conditions, stage: activeCommunicator });
            }
        });
    }, [activeCommunicator]);

    return (<FilterCom />);
});

const FilterCom = observer((): JSX.Element => {
    return (<div>
        <FormItem
            name="Step"
            content={(<Select
                value={conditions.iterationId}
                style={{ width: 120 }}
                onChange={val => handleChange('iterationId', val)}
                options={optionMap.iterationOptions}
            />
            )}/>
        <FormItem
            name="Communication Group"
            content={(<Select
                value={conditions.stage}
                style={{ width: 200 }}
                onChange={val => handleChange('stage', val)}
                options={optionMap.stageOptions}
            />
            )}/>
        <FormItem
            name="Operator Name"
            content={(
                <Select
                    value={conditions.operatorName}
                    style={{ width: 300 }}
                    onChange={val => handleChange('operatorName', val)}
                    options={optionMap.operatorOptions}
                    showSearch={true}
                />)}/>
        <FormItem content={(
            <Radio.Group value={conditions.type}
                onChange={(e) => { handleChange('type', e.target.value); }}>
                <Radio value={'CommunicationDurationAnalysis'}>Communication Duration Analysis</Radio>
                <Radio value={'CommunicationMatrix'}>Communication Matrix</Radio>
            </Radio.Group>)}/>
        <div>
        </div>
    </div>);
});

const FormItem = (props: any): JSX.Element => {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name}/>
        {props.content}
    </div>);
};

export default Filter;
