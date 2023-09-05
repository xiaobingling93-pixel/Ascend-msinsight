/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select, Radio } from 'antd';
import { Label, notNullObj } from '../Common';
import { optionDataType, VoidFunction } from '../../utils/interface';
import { queryIterations, queryOperators, queryRanks, queryStages } from '../../utils/RequestUtils';
import { Session } from '../../entity/session';

export interface ConditionDataType{
    iterationId: string ;
    rankIds: string[];
    operatorName: string ;
    type?: string;
    stage: string;
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}

const getiterationOptions = async(): Promise<optionDataType[]> => {
    const res: {iterationsOrRanks: Array<{iteration_id: string } > } = await queryIterations();
    const list: string[] = res.iterationsOrRanks.map(item => item.iteration_id);
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

const getStageOptions = async (iterationId: string): Promise<optionDataType[]> => {
    const res: {data: string[] } = await queryStages({ iterationId });
    const list = res.data;
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

const getOperatorOptions = async ({ iterationId, rankIds, stage }: {iterationId: string;
    rankIds: string[]; stage: string;}):
Promise<optionDataType[]> => {
    const res: {operators: Array<{op_name: string } > } = await queryOperators({ iterationId, rankIds, stage }); ;
    const list = res.operators.map((item: any) => item.op_name);
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

export const getRankIdOptions = async (iterationId: string): Promise<optionDataType[]> => {
    const res: {iterationsOrRanks: Array<{rank_id: string } > } = await queryRanks({ iterationId });
    const list = res.iterationsOrRanks.map(item => item.rank_id);
    const options: optionDataType[] = list.map(item => ({ value: item, label: item }));
    return options;
};

const getOptions = async(): Promise<any> => {
    // step
    const iterationOptions: optionDataType[] = await getiterationOptions();
    const firstIterationId = iterationOptions[0]?.value as string;
    // stage
    const stageOptions: optionDataType[] = await getStageOptions(firstIterationId);
    const firstStage = stageOptions[0]?.value as string;
    // Operator Name
    const operatorOptions: optionDataType[] =
        await getOperatorOptions({ iterationId: firstIterationId, rankIds: [], stage: firstStage });

    return { iterationOptions, operatorOptions, stageOptions };
};

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

const Filter = observer((props: {session: Session;handleFilterChange: VoidFunction}) => {
    const [ conditions, setConditions ] = useState<ConditionDataType>(defaultCondition);
    const [ optionMap, setOptionMap ] = useState<optionMapDataType>(defaultOptionMap);

    // 初始化
    useEffect(() => {
        init();
    }, [props.session.allRankIds]);
    useEffect(() => {
        if (props.session.activeCommunicator?.ranks !== undefined) {
            setConditions({ ...conditions, stage: `(${props.session.activeCommunicator.ranks.join(',')})` });
        }
    }, [props.session.activeCommunicator]);

    // Iteration ID联动Stage
    useEffect(() => {
        handleRelatedChange('iterationId', conditions.iterationId);
    }, [conditions.iterationId]);
    // Stage 联动算子
    useEffect(() => {
        handleRelatedChange('stage', conditions.stage);
    }, [conditions.stage]);

    // 筛选条件变化
    useEffect(() => {
        const list = [ 'operatorName', 'type' ];
        if (notNullObj(conditions, list)) {
            props.handleFilterChange(conditions);
        }
    }, [conditions]);

    const init = async(): Promise<void> => {
        const newOptionsMap: optionMapDataType = await getOptions();
        // 初始可选项
        setOptionMap({ ...optionMap, ...newOptionsMap });
        // 初始查询条件
        const iterationId = newOptionsMap.iterationOptions[0]?.value as string;
        const stage = newOptionsMap.stageOptions[0]?.value as string;
        setConditions({ ...conditions, iterationId, stage, operatorName: totalOperator });
    };

    // 联动的条件
    const handleRelatedChange = async (source: string, value: string): Promise<void> => {
        if (source === 'iterationId') {
            const stageOptions: optionDataType[] = await getStageOptions(value);
            const stage = stageOptions[0]?.value as string;
            setOptionMap({ ...optionMap, stageOptions });
            setConditions({ ...conditions, stage });
        }
        const operatorOptions: optionDataType[] = await getOperatorOptions(
            { iterationId: conditions.iterationId, rankIds: [], stage: conditions.stage });
        setOptionMap({ ...optionMap, operatorOptions });
        if (!operatorOptions.map(item => item.value).includes(conditions.operatorName)) {
            setConditions({ ...conditions, operatorName: totalOperator });
        }
    };

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[] | number[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} optionMap={optionMap} />);
});

const FilterCom = ({ conditions, handleChange, optionMap = {} }: any): JSX.Element => {
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
};

const FormItem = (props: any): JSX.Element => {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name}/>
        {props.content}
    </div>);
};

export default Filter;
