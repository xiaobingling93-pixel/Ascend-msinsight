/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select } from 'antd';
import { Label, notNull } from '../Common';
import { getStepsData } from './PpBandwidthAnalysis';
import { getPpContainerData } from '../communicatorContainer/ContainerUtisl';
import _ from 'lodash';

export interface ConditionDataType {
    step: string | number;
    stage: string;
    orderBy?: string ;
    top?: number;
    ranks: number[];
}

interface optionDataType{
    key?: string;
    label: React.ReactNode;
    value: string | number ;
    ranks?: number[];
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}

const Filter = observer((props: any) => {
    const [ conditions, setConditions ] = useState<ConditionDataType>(
        { step: '', stage: '', orderBy: 'computingTime', top: 0, ranks: [] });
    const [ options, setOptions ] = useState<optionMapDataType>({});
    // 初始化
    useEffect(() => {
        initDefault();
    }, [props.session.communicatorData]);
    useEffect(() => {
        if (notNull(conditions.step) && notNull(conditions.stage)) {
            props.handleFilterChange(conditions);
        }
    }, [ conditions.step, conditions.stage ]);
    const initDefault = async (): Promise<void> => {
        const stepList: string[] = await getStepsData();
        const stepOptions: optionDataType[] = stepList.map(item => ({ value: item, label: item }));
        const stageOptions: optionDataType[] = getPpContainerData(props.session.communicatorData, 'pp') as optionDataType[];
        const stage: string = stageOptions.length > 0 ? stageOptions[0].value as string : '';
        const ranks: number[] = stageOptions.length > 0 ? stageOptions[0].ranks as number[] : [];
        setOptions({ stepOptions, stageOptions });
        setConditions({ ...conditions, step: stepList[0], stage, ranks });
    };

    useEffect(() => {
        const name = props.session.activeCommunicator?.name as string;
        if (name?.startsWith('stage')) {
            setConditions({ ...conditions, stage: props.session.activeCommunicator?.value, ranks: props.session.activeCommunicator?.ranks });
        }
    }, [props.session.activeCommunicator]);

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[]): void => {
        if (prop === 'stage') {
            const option = _.find(options.stageOptions, item => item.value === val);
            if (option !== undefined) {
                setConditions({ ...conditions, [prop as string]: val, ranks: option.ranks as number[] });
            }
        } else {
            setConditions({ ...conditions, [prop]: val });
        }
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} session={props.session}/>);
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
