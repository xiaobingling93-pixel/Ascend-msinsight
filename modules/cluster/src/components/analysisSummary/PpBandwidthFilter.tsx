/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select } from 'antd';
import { Label } from '../Common';
import { getStepsData } from './PpBandwidthAnalysis';
import { getAllPpStageIds, getPpContainerData } from '../communicatorContainer/ContainerUtils';

export interface ConditionDataType {
    step: string;
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

const Filter = observer((props: any) => {
    const [options, setOptions] = useState<optionMapDataType>({});
    // 初始化
    useEffect(() => {
        if (props.session?.clusterCompleted === false) {
            setOptions({ stepOptions: [], stageOptions: [] });
            props.setConditions({ step: '', stage: '' });
            return;
        }
        initDefault();
        props.setAllStageIds(getAllPpStageIds(props.session.communicatorData));
    }, [props.session.communicatorData]);
    const initDefault = async (): Promise<void> => {
        const stepList: string[] = await getStepsData();
        const stepOptions: optionDataType[] = stepList.map(item => ({ value: item, label: item }));
        const stageOptions: optionDataType[] = getPpContainerData(props.session.communicatorData, 'pp') as optionDataType[];
        const stage: string = stageOptions.length > 0 ? stageOptions[0].value as string : '';
        setOptions({ stepOptions, stageOptions });
        props.setConditions({ ...props.conditions, step: stepList[0], stage });
    };

    useEffect(() => {
        const name = props.session.activeCommunicator?.name as string;
        if (name?.startsWith('stage')) {
            props.setConditions({ ...props.conditions, stage: props.session.activeCommunicator?.value });
        }
    }, [props.session.activeCommunicator]);

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[]): void => {
        props.setConditions({ ...props.conditions, [prop]: val });
    };

    return (<FilterCom conditions={props.conditions} handleChange={handleChange} options={options} session={props.session}/>);
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
