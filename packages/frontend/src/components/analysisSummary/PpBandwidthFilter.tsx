/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select } from 'antd';
import { Label, notNull } from '../Common';
import { getStagesData, getStepsData } from './PpBandwidthAnalysis';

export interface ConditionDataType {
    step: string | number;
    stage: string;
    orderBy?: string ;
    top?: number;
}

interface optionDataType{
    key?: string;
    label: React.ReactNode;
    value: string | number ;
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}

// Top可选项： 1、2、4、8.......n(All)
const getTopOptions = (count: number): optionDataType[] => {
    const logIndex = Math.ceil(Math.log2(count > 0 ? count : 1));
    const toplist = [];
    for (let i = 0; i < logIndex; i++) {
        toplist.push(Math.pow(2, i));
    }
    const topOptions: optionDataType[] = toplist.map(item => ({ value: item, label: item }));
    if (count > 0) {
        topOptions.push({ value: count, label: `${count} ( All )` });
    }
    return topOptions;
};

const Filter = observer((props: any) => {
    const [ conditions, setConditions ] = useState<ConditionDataType>(
        { step: '', stage: '', orderBy: 'computingTime', top: 0 });
    const [ options, setOptions ] = useState<optionMapDataType>({});
    // 初始化
    useEffect(() => {
        initDefault();
    }, []);
    useEffect(() => {
        if (notNull(conditions.step) && notNull(conditions.stage)) {
            props.handleFilterChange(conditions);
        }
    }, [ conditions.step, conditions.stage ]);
    const initDefault = async (): Promise<void> => {
        const stepList: string[] = await getStepsData();
        const stepOptions: optionDataType[] = stepList.map(item => ({ value: item, label: item }));
        const stages: string[] = await getStagesData({ stepId: stepList[0] });
        const stageOptions: optionDataType[] = stages.map(item => ({ value: item, label: item }));
        const topOptions = getTopOptions(stages.length);
        setOptions({ stepOptions, topOptions, stageOptions });
        setConditions({ ...conditions, step: stepList[0], stage: stages[0] });
    };

    useEffect(() => {
        const name = props.session.activeCommunicator?.name as string;
        if (name?.startsWith('stage')) {
            setConditions({ ...conditions, stage: props.session.activeCommunicator?.value });
        }
    }, [props.session.activeCommunicator]);

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} />);
});

const FilterCom = (props: any): JSX.Element => {
    const { conditions, handleChange = [], options = {} } = props;
    return (<div style={ { margin: '0 20px 10px', textAlign: 'left' }}>
        <Label name="Step" />
        <Select
            value={conditions.step}
            style={{ width: 120 }}
            onChange={(val: any) => handleChange('step', val)}
            options={options.stepOptions}
        />
        <Label name="Stage"/>
        <Select
            value={conditions.stage}
            style={{ width: 200 }}
            onChange={val => handleChange('stage', val)}
            options={options.stageOptions}
        />
    </div>);
};

export default Filter;
