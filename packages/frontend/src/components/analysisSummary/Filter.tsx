/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select } from 'antd';
import { Label, MultiSelectWithAll } from '../Common';

export interface ConditionDataType {
    step: string | number;
    rankIds: string[];
    orderBy: string ;
    top: number;
}

interface optionDataType{
    key?: string;
    label: React.ReactNode;
    value: string | number ;
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}
const orderOptions = [
    { label: 'Computing', value: 'computingTime' },
    { label: 'Communication(Not Overlapped)', value: 'communicationNotOverLappedTime' },
    { label: 'Communication(Overlapped)', value: 'communicationOverLappedTime' },
    { label: 'Free', value: 'freeTime' },
];

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
        { step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 });
    const [ options, setOptions ] = useState<optionMapDataType>({});
    // 初始化
    useEffect(() => {
        initDefault();
    }, [props.groupData.init]);
    useEffect(() => {
        props.handleFilterChange(conditions);
    }, [ conditions.step, conditions.orderBy ]);
    useEffect(() => {
        props.handleFilterChange(conditions, false);
    }, [ conditions.rankIds, conditions.top ]);
    const initDefault = async (): Promise<void> => {
        const stepList: number[] = props.groupData.stepList;
        const stepOptions: optionDataType[] = [ 'All', ...stepList ].map(item => ({ value: item, label: item }));
        const rankIds: number[] = props.groupData.rankList;
        const rankIdOptions: optionDataType[] = rankIds.map(item => ({ value: item, label: item }));
        const topOptions = getTopOptions(rankIds.length);
        setOptions({ stepOptions, topOptions, rankIdOptions, orderOptions });
        setConditions({ ...conditions, top: rankIds.length });
    };

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} />);
});

const FilterCom = (props: any): JSX.Element => {
    const { conditions, handleChange = [], options = {} } = props;
    return (<div style={ { margin: '0 20px 10px' }}>
        <Label name="Step" />
        <Select
            value={conditions.step}
            style={{ width: 120 }}
            onChange={(val: any) => handleChange('step', val)}
            options={options.stepOptions}
        />
        <Label name="Rank ID"/>
        <MultiSelectWithAll
            value={conditions.rankIds}
            onChange={(val: any) => handleChange('rankIds', val)}
            options={options.rankIdOptions}
            style={{ width: 150 }}
            maxTagCount={1}
        />
        <Label name="Order By"/>
        <Select
            value={conditions.orderBy}
            style={{ width: 280 }}
            onChange={(val: any) => handleChange('orderBy', val)}
            options={options.orderOptions}
        />
        <Label name="Top"/>
        <Select
            value={conditions.top}
            style={{ width: 120 }}
            onChange={(val: any) => handleChange('top', val)}
            options={options.topOptions}
        />
    </div>);
};

export default Filter;
