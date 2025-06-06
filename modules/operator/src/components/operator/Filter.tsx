/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { observer } from 'mobx-react';
import { observable, runInAction, observe } from 'mobx';
import React, { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, InputNumber } from 'ascend-components';
import { getRankInfoKey, getRankInfoLabel, GroupCardRankInfosByHost, Label } from 'ascend-utils';
import type { OptionDataType, OptionMapType, VoidFunction } from '../../utils/interface';
import type { CardRankInfo, Session } from '../../entity/session';
const OPERATOR_TYPE = 'Operator Type';

export interface ConditionType {
    rankInfoKey: string;
    rankId: string;
    dbPath: string;
    group: string;
    topK: number;
    custom: number;
    host: string;
    isCompare: boolean;
}

export const defaultCondition = {
    rankInfoKey: '',
    rankId: '',
    dbPath: '',
    group: OPERATOR_TYPE,
    topK: 15,
    custom: 0,
    host: '',
    isCompare: false,
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
    rankOptions: [],
    hostOptions: [],
    groupOptions: [
        { label: 'Computing Operator', value: 'Operator' },
        { label: 'Computing Operator Type', value: 'Operator Type' },
        { label: 'Computing Operator Name and Input Shape', value: 'Input Shape' },
        { label: 'Communication Operator', value: 'Communication Operator' },
        { label: 'Communication Operator Type', value: 'Communication Operator Type' },
    ],
    topKOptions: [
        { label: '15', value: 15 },
        { label: 'All', value: -1 },
    ],
};
const condition: ConditionType = observable(defaultCondition);
const optionMap: OptionMapType = observable(defaultOptionMap);

const setOptions = (initOptionMap: Partial<OptionMapType> = {}): void => {
    runInAction(() => {
        const keys = Object.keys(initOptionMap) as Array<(keyof OptionMapType)>;
        keys.forEach(key => {
            optionMap[key] = initOptionMap[key] as any ?? [];
        });
    });
};

const setCondition = (initCondition: ConditionType = {} as ConditionType): void => {
    runInAction(() => {
        const keys = Object.keys(initCondition);
        keys.forEach(key => {
            (condition as any)[key] = (initCondition as any)[key];
        });
        if (condition.host === '' || !optionMap.hostOptions.find(item => item.value === condition.host)) {
            condition.host = optionMap.hostOptions[0]?.value as string ?? '';
        }
        if (condition.rankInfoKey === '') {
            condition.rankInfoKey = optionMap.rankOptions?.[0]?.value as string ?? '';
            condition.rankId = optionMap.rankOptions?.[0]?.rankId as string ?? '';
            condition.dbPath = optionMap.rankOptions?.[0]?.dbPath as string ?? '';
        }
    });
};

function handleChange<T>(key: keyof ConditionType, val: T): void {
    runInAction(() => {
        condition[key] = val as never;
        if (key === 'topK' && val === 0) {
            condition.custom = 15;
        } else if (key === 'rankInfoKey') {
            const rankOption = optionMap.rankOptions.find(({ value }) => value === val);
            condition.dbPath = rankOption?.dbPath ?? '';
            condition.rankId = rankOption?.rankId ?? '';
        }
    });
};

function checkRankId(rankId: string): boolean {
    if (rankId !== '' && optionMap.rankOptions.some((item) => rankId === item.rankId as string)) {
        return true;
    }
    return false;
}

function getRankOptions(cards: CardRankInfo[], host: string): Array<OptionDataType & { rankId: string; dbPath: string }> {
    return cards.map((item) => {
        const label = getRankInfoLabel(item.rankInfo);
        return {
            label,
            value: getRankInfoKey(item.rankInfo),
            rankId: item.rankInfo.rankId,
            dbPath: item.dbPath ?? '',
            index: item.index,
        };
    }).sort((a, b) => a.index - b.index);
}

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
        if (session.allCardInfos.length === 0) {
            setOptions(defaultOptionMap);
            setCondition(defaultCondition);
            return;
        }
        const { hosts, cardsMap }: { hosts: string[]; cardsMap: Map<string, CardRankInfo[]> } = GroupCardRankInfosByHost(session.allCardInfos);
        const hostOptions = hosts.map(item => (
            { label: item, value: item, cards: cardsMap.get(item) }
        ) as OptionDataType);
        const host = hosts[0] ?? '';
        const rankOptions = getRankOptions(cardsMap.get(host) ?? [], host);
        setOptions({ hostOptions, rankOptions });
    }, [session.allCardInfos]);

    useEffect(() => {
        runInAction(() => {
            condition.rankId = checkRankId(session.dirInfo.rankId) ? session.dirInfo.rankId : optionMap.rankOptions[0]?.rankId as string ?? '';
            const found = optionMap.rankOptions.find(({ rankId }) => rankId === condition.rankId);
            condition.rankInfoKey = (found?.value ?? '') as string;
            condition.dbPath = found?.dbPath ?? '';
            condition.isCompare = session.dirInfo.isCompare ?? false;
        });
        handleFilterChange({ ...condition, topK: condition.topK !== 0 ? condition.topK : condition.custom });
    }, [session.dirInfo]);

    useEffect(() => {
        const hostOption = optionMap.hostOptions.find(item => item.value === condition.host);
        const rankOptions = getRankOptions(hostOption?.cards ?? [], condition.host);
        setOptions({ rankOptions });
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
    if (condition.isCompare && condition.group === 'Operator') {
        condition.group = 'Operator Type';
    }
    const groupOptions = (condition.isCompare ? optionMap.groupOptions.filter(item => item.value !== 'Operator') : optionMap.groupOptions)
        .map(item => ({
            ...item,
            label: t(item.label as string),
        }));
    const topKOptions = optionMap.topKOptions.map(item => ({
        ...item,
        label: t(item.label as string),
    }));
    return (<div>
        {
            optionMap.hostOptions.length > 0
                ? <FormItem
                    name={t('Host')}
                    content={(<Select
                        value={condition.host}
                        style={{ width: 250 }}
                        onChange={(val: string): void => handleChange('host', val)}
                        options={optionMap.hostOptions}
                        disabled={condition.isCompare}
                    />
                    )}/>
                : <div></div>
        }
        <FormItem
            name={t('Group By')}
            content={(<Select
                id={'select-groupId'}
                value={condition.group}
                style={{ width: 250 }}
                onChange={(val: string): void => handleChange('group', val)}
                options={groupOptions}
            />
            )}/>
        <FormItem
            name={t('Rank ID')}
            content={(<Select
                id={'select-rankId'}
                value={condition.rankInfoKey}
                style={{ width: 200 }}
                onChange={(val: string): void => handleChange('rankInfoKey', val)} // rankId 修改，要联动 dbPath
                options={optionMap.rankOptions}
                showSearch={true}
                disabled={condition.isCompare}
            />
            )}/>
        <FormItem
            name={t('Top')}
            content={(<><Select
                id={'select-top'}
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
