/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { useEffect } from 'react';
import { Select } from 'antd';
import { Label } from '../Common';
import { optionMapType, VoidFunction } from '../../utils/interface';
import { Session } from '../../entity/session';

export interface ConditionType {
    rankId: string ;
    group: string;
    topK: number;
    [prop: string]: any;
}

const defaultCondition = {
    rankId: '',
    group: 'Operator',
    topK: 15,
};
const defaultOptionMap = {
    rankIdOptions: [],
    groupOptions: [
        { label: 'Opertator', value: 'Operator' },
        { label: 'Operator Type', value: 'Operator Type' },
        { label: 'Operator Name and Input Shape', value: 'Input Shape' },
    ],
    topKOptions: [
        { label: '15', value: 15 },
    ],
};
const condition: ConditionType = observable(defaultCondition);
const optionMap: optionMapType = observable(defaultOptionMap);

const setOptions = async(initOptionMap?: optionMapType): Promise<void> => {
    // rankId
    const rankIdOptions = initOptionMap?.rankIdOptions ?? defaultOptionMap.rankIdOptions;
    // group
    const groupOptions = initOptionMap?.groupOptions ?? defaultOptionMap.groupOptions;
    // topK
    const topKOptions = initOptionMap?.topKOptions ?? defaultOptionMap.topKOptions;
    runInAction(() => {
        optionMap.rankIdOptions = rankIdOptions;
        optionMap.groupOptions = groupOptions;
        optionMap.topKOptions = topKOptions;
    });
};

const setCondition = (initCondition?: ConditionType): void => {
    const rankId = initCondition?.rankId ?? optionMap.rankIdOptions[0]?.value as string ?? defaultCondition.rankId;
    const group = initCondition?.group ?? optionMap.groupOptions[0]?.value as string ?? defaultCondition.group;
    const topK = initCondition?.topK ?? optionMap.topKOptions[0]?.value as number ?? defaultCondition.topK;

    runInAction(() => {
        condition.group = group;
        condition.rankId = rankId;
        condition.topK = topK;
    });
};

const handleChange = (key: string, val: string | number): void => {
    runInAction(() => {
        condition[key] = val;
    });
};

const Filter = observer(({ session, handleFilterChange }: {session: Session;handleFilterChange: VoidFunction}) => {
    // 初始化
    useEffect(() => {
        observe(condition, () => {
            handleFilterChange(condition);
        });
        observe(optionMap, () => {
            setCondition();
        });
        setOptions();
    }, [ ]);

    useEffect(() => {
        const rankIdOptions = session.allRankIds.map((item, index) => ({ label: item, value: index }));
        setOptions({ rankIdOptions });
    }, [session.allRankIds]);
    return (<FilterCom />);
});

const FilterCom = observer((): JSX.Element => {
    return (<div>
        <FormItem
            name="RankId"
            content={(<Select
                value={condition.rankId}
                style={{ width: 200 }}
                onChange={val => handleChange('rankId', val)}
                options={optionMap.rankIdOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name="Group By"
            content={(<Select
                value={condition.group}
                style={{ width: 250 }}
                onChange={val => handleChange('group', val)}
                options={optionMap.groupOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name="Top"
            content={(<Select
                value={condition.topK}
                style={{ width: 100 }}
                onChange={val => handleChange('topK', val)}
                options={optionMap.topKOptions}
                showSearch={true}
            />
            )}/>
    </div>);
});

const FormItem = (props: any): JSX.Element => {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name} style={{ minWidth: '50px', display: 'inline-block' }}/>
        {props.content}
    </div>);
};

export default Filter;
