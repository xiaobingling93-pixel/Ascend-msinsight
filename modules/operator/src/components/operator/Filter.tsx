/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, InputNumber } from 'ascend-components';
import { GroupRankIdsByHost, Label } from 'ascend-utils';
import type { optionMapType, VoidFunction } from '../../utils/interface';
import type { Session } from '../../entity/session';
const OPERATOR_TYPE = 'Operator Type';

export interface ConditionType {
    rankId: string ;
    group: string;
    topK: number;
    custom: number;
    host: string;
}

export const defaultCondition = {
    rankId: '',
    group: OPERATOR_TYPE,
    topK: 15,
    custom: 0,
    host: '',
};

export interface FilterType {
    name: string[];
    opName: string[];
    type: string[];
    opType: string[];
    accCore: string[];
}

export const defaultFilterType = {
    name: [],
    opName: [],
    type: [],
    opType: [],
    accCore: [],
};

const defaultOptionMap = {
    rankIdOptions: [],
    hostsOptions: [],
    groupOptions: [
        { label: 'Computing Operator', value: 'Operator' },
        { label: 'Computing Operator Type', value: 'Operator Type' },
        { label: 'Computing Operator Name and Input Shape', value: 'Input Shape' },
        { label: 'HCCL Operator', value: 'HCCL Operator' },
        { label: 'HCCL Operator Type', value: 'HCCL Operator Type' },
    ],
    topKOptions: [
        { label: '15', value: 15 },
        { label: 'All', value: -1 },
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
        if (condition.host === '' || !optionMap.hostsOptions.find(item => item.value === condition.host)) {
            condition.host = optionMap.hostsOptions[0]?.value as string ?? '';
        }
        if (condition.rankId === '' || !optionMap.rankIdOptions.find(item => item.value === condition.rankId)) {
            condition.rankId = optionMap.rankIdOptions[0]?.value as string ?? '';
        }
    });
};

function handleChange<T>(key: keyof ConditionType, val: T): void {
    runInAction(() => {
        condition[key] = val as never;
        if (key === 'topK' && val === 0) {
            condition.custom = 15;
        }
    });
};

const Filter = observer(({ session, handleFilterChange }: {session: Session;handleFilterChange: VoidFunction}) => {
    // 初始化
    useEffect(() => {
        observe(condition, () => {
            handleFilterChange({ ...condition, topK: condition.topK !== 0 ? condition.topK : condition.custom });
        });
        observe(optionMap, () => {
            setCondition();
        });
    }, []);

    useEffect(() => {
        const { hosts, ranks }: { hosts: string[]; ranks: Map<string, string[]> } = GroupRankIdsByHost(session.allRankIds);
        const hostsOptions = hosts.map(item => (
            { label: item, value: item, data: ranks.get(item) }
        ));
        const host = hosts[0] ?? '';
        const rankIdOptions = (ranks.get(host) ?? []).map((item, index) => (
            { label: (!item.startsWith('[MSPROF]') ? item : item.substring(item.lastIndexOf('__') + 2)).replace(`${host} `, ''), value: item }
        ));
        setOptions({ hostsOptions, rankIdOptions });
        if (session.allRankIds.length === 0) {
            setOptions(defaultOptionMap);
            setCondition(defaultCondition);
        }
    }, [session.allRankIds]);

    useEffect(() => {
        const hostOption = optionMap.hostsOptions.find(item => item.value === condition.host);
        const rankIdOptions = (hostOption?.data ?? [] as string[]).map((item: string) => (
            { label: (!item.startsWith('[MSPROF]') ? item : item.substring(item.lastIndexOf('__') + 2)).replace(`${condition.host} `, ''), value: item }
        ));
        setOptions({ rankIdOptions });
    }, [condition.host]);

    useEffect(() => {
        const { total } = session;
        if (total < 1) {
            return;
        }
        const topKOptions = [
            { label: '15', value: 15 },
            { label: 'All', value: -1 },
            { label: 'Custom', value: 0 },
        ];
        setOptions({ topKOptions });
    }, [session.total]);
    return (<FilterCom session={session}/>);
});

const FilterCom = observer(({ session }: {session: Session}): JSX.Element => {
    const { t } = useTranslation('operator', { keyPrefix: 'searchCriteria' });
    const groupOptions = optionMap.groupOptions.map(item => ({
        ...item,
        label: t(item.label as string),
    }));
    const topKOptions = optionMap.topKOptions.map(item => ({
        ...item,
        label: t(item.label as string),
    }));
    return (<div>
        {
            optionMap.hostsOptions.length > 0
                ? <FormItem
                    name={t('Host')}
                    content={(<Select
                        value={condition.host}
                        style={{ width: 250 }}
                        onChange={(val: string): void => handleChange('host', val)}
                        options={optionMap.hostsOptions}
                    />
                    )}/>
                : <div></div>
        }
        <FormItem
            name={t('Group By')}
            content={(<Select
                value={condition.group}
                style={{ width: 250 }}
                onChange={(val: string): void => handleChange('group', val)}
                options={groupOptions}
            />
            )}/>
        <FormItem
            name={t('Rank ID')}
            content={(<Select
                value={condition.rankId}
                style={{ width: 200 }}
                onChange={(val: string): void => handleChange('rankId', val)}
                options={optionMap.rankIdOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name={t('Top')}
            content={(<><Select
                value={condition.topK}
                style={{ width: 100 }}
                onChange={(val: string): void => handleChange('topK', val)}
                options={topKOptions}
            />
            <InputNumber
                min={0}
                max={100000000}
                value={condition.custom}
                onChange={(val: string | number | null): void => handleChange('custom', val)}
                controls={false}
                precision={0}
                style={{ marginLeft: '10px', width: '80px', display: condition.topK === 0 ? 'inline-block' : 'none' }} />
            </>
            )}/>
    </div>);
});

const FormItem = (props: any): JSX.Element => {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name} style={{ display: 'inline-block' }}/>
        {props.content}
    </div>);
};

export default Filter;
