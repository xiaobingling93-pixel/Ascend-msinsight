/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { useEffect } from 'react';
import { Select, InputNumber } from 'antd';
import { Label } from '../Common';
import { optionMapType, VoidFunction } from '../../utils/interface';
import { Session } from '../../entity/session';

export interface ConditionType {
    rankId: string ;
    group: string;
    topK: number;
    custom?: number;
}

export const defaultCondition = {
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
    const rankId = getValue(
        [ initCondition?.rankId, condition.rankId, optionMap.rankIdOptions[0]?.value, defaultCondition.rankId ]);
    const group = getValue(
        [ initCondition?.group, condition.group, optionMap.groupOptions[0]?.value, defaultCondition.group ]);
    const topK = getValue(
        [ initCondition?.topK, condition.topK, optionMap.topKOptions[0]?.value, defaultCondition.topK ]);
    runInAction(() => {
        condition.group = group as string;
        condition.rankId = rankId as string;
        condition.topK = topK as number;
    });
};

function getValue<T>(list: T[]): T | undefined {
    if (list.length === 0) {
        return;
    }
    for (let i = 0; i < list.length; i++) {
        if (list[i] !== undefined && list[i] !== null && list[i] !== '') {
            return list[i];
        }
    }
    return list[list.length - 1];
};

function handleChange<T>(key: keyof ConditionType, val: T): void {
    runInAction(() => {
        condition[key] = val as never;
    });
};

const Filter = observer(({ session, handleFilterChange }: {session: Session;handleFilterChange: VoidFunction}) => {
    // 初始化
    useEffect(() => {
        observe(condition, () => {
            handleFilterChange({ ...condition, topK: condition.topK !== -1 ? condition.topK : condition.custom });
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

    useEffect(() => {
        const { total } = session;
        if (total < 1) {
            return;
        }
        const topKOptions = [
            { label: '15', value: 15 },
            { label: `${total}(All)`, value: total },
            { label: 'Custom', value: -1 },
        ];
        setOptions({ topKOptions });
    }, [session.total]);
    return (<FilterCom session={session}/>);
});

const FilterCom = observer(({ session }: {session: Session}): JSX.Element => {
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
            content={(<><Select
                value={condition.topK}
                style={{ width: 100 }}
                onChange={val => handleChange('topK', val)}
                options={optionMap.topKOptions}
                showSearch={true}
            />
            <InputNumber
                min={0}
                max={session.total}
                onChange={val => handleChange('custom', val)}
                controls={false}
                formatter={val => String(Number(val))}
                style={{ marginLeft: '10px', width: '80px', display: condition.topK === -1 ? 'inline-block' : 'none' }} />
            </>
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
