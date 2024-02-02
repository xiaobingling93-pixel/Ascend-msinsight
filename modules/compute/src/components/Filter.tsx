/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { type ReactElement, useEffect } from 'react';
import { Select } from 'antd';
import { Label } from './Common';
import type { optionDataType, optionMapDataType } from '../utils/interface';
import type { Session } from '../entity/session';

export interface ConditionType {
    core: string ;
    source: string;
};

export const totalOperator = 'Total Op Info';
const defaultCondition = {
    core: '',
    source: '',
};
const defaultOptionMap = {
    coreOptions: [],
    sourceOptions: [],
};
const condition: ConditionType = observable(defaultCondition);
const optionMap: optionMapDataType = observable(defaultOptionMap);

const setOptions = async(initObj = {} as ConditionType,
    initOptionMap: optionMapDataType = defaultOptionMap): Promise<void> => {
    // core
    const coreOptions: optionDataType[] = initOptionMap.coreOptions;
    const core = initObj.core ?? coreOptions[0]?.value as string ?? defaultCondition.core;
    // source
    const sourceOptions: optionDataType[] = initOptionMap.sourceOptions;
    const source = initObj.source ?? sourceOptions[0]?.value as string ?? defaultCondition.source;
    runInAction(() => {
        optionMap.sourceOptions = sourceOptions;
        optionMap.coreOptions = coreOptions;
        condition.source = source;
        condition.core = core;
    });
};

const handleChange = (key: keyof ConditionType, val: string): void => {
    runInAction(() => {
        condition[key] = val;
    });
};

const Filter = observer(({ session, handleFilterChange }:
{session: Session;handleFilterChange: (condition: ConditionType) => void}) => {
    // 初始化
    useEffect(() => {
        setOptions();
        observe(condition, (change) => {
            handleFilterChange(condition);
        });
    }, []);

    useEffect(() => {
        const coreOptions = session.coreList.map((item, index) => ({ label: item, value: index }));
        const sourceOptions = session.sourceList.map(item => ({ label: item, value: item }));
        setOptions({} as ConditionType, { coreOptions, sourceOptions });
    }, [session.coreList, session.sourceList]);
    return (<FilterCom />);
});

const FilterCom = observer((): JSX.Element => {
    return (<div>
        <FormItem
            name="Core"
            style={{ width: 'calc(30% - 40px)', minWidth: '270px' }}
            content={(<Select
                value={condition.core}
                style={{ width: 'calc(100% - 70px)' }}
                onChange={val => handleChange('core', val)}
                options={optionMap.coreOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name="Source"
            style={{ width: 'calc(70% - 300px)', minWidth: '700px' }}
            content={(<Select
                value={condition.source}
                style={{ width: 'calc(100% - 70px)' }}
                onChange={val => handleChange('source', val)}
                options={optionMap.sourceOptions}
                showSearch={true}
            />
            )}/>
    </div>);
});

const FormItem = (props: {name: string;style?: React.CSSProperties;content: ReactElement}): JSX.Element => {
    return (<div style={{
        display: 'inline-block',
        height: '30px',
        lineHeight: '30px',
        margin: '0 20px 10px 0',
        ...props.style ?? {},
    }}>
        <Label name={props.name} style={{ width: '50px', display: 'inline-block' }}/>
        {props.content}
    </div>);
};

export default Filter;
