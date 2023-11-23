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
const OPERATOR_TYPE = 'Operator Type';

export interface ConditionType {
    rankId: string ;
    group: string;
    topK: number;
    custom: number;
}

export const defaultCondition = {
    rankId: '',
    group: OPERATOR_TYPE,
    topK: 15,
    custom: 0,
};
const defaultOptionMap = {
    rankIdOptions: [],
    groupOptions: [
        { label: 'Operator', value: 'Operator' },
        { label: 'Operator Type', value: 'Operator Type' },
        { label: 'Operator Name and Input Shape', value: 'Input Shape' },
    ],
    topKOptions: [
        { label: '15', value: 15 },
    ],
};
const condition: ConditionType = observable(defaultCondition);
const optionMap: optionMapType = observable(defaultOptionMap);

const setOptions = async(initOptionMap: optionMapType = {}): Promise<void> => {
    runInAction(() => {
        const keys = Object.keys(initOptionMap);
        keys.forEach(key => {
            optionMap[key] = initOptionMap[key] ?? [];
        });
    });
};

const setCondition = (initCondition: ConditionType = {} as ConditionType): void => {
    runInAction(() => {
        const keys = Object.keys(initCondition);
        keys.forEach(key => {
            (condition as any)[key] = (initCondition as any)[key];
        });
        if (condition.rankId === '') {
            condition.rankId = optionMap.rankIdOptions[0]?.value as string ?? '';
        }
    });
};

function handleChange<T>(key: keyof ConditionType, val: T): void {
    runInAction(() => {
        condition[key] = val as never;
        if (key === 'topK' && val === -1) {
            condition.custom = 15;
        }
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
    }, [ ]);

    useEffect(() => {
        const rankIdOptions = session.allRankIds.map((item, index) => ({ label: item, value: item }));
        setOptions({ rankIdOptions });
        if (session.allRankIds.length === 0) {
            setOptions(defaultOptionMap);
            setCondition(defaultCondition);
        }
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
                max={100000000}
                value={condition.custom}
                onChange={val => handleChange('custom', val)}
                controls={false}
                precision={0}
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
