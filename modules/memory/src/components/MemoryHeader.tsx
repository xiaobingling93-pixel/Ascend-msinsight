/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { SearchBox, FlexDiv } from '../utils/styleUtils';
import { MemoryHeaderStrategy } from '../utils/strategyUtils';
import { Session } from '../entity/session';
import { MemorySession, ConditionType } from '../entity/memorySession';
import { Label, useHit, joinStringArray } from './Common';
import { Select } from 'ascend-components';
import { GroupRankIdsByHost } from 'ascend-utils';
import { useTranslation } from 'react-i18next';

const groupByOptions = [
    { label: 'Overall', value: 'Overall' },
    { label: 'Stream', value: 'Stream' },
];

const MemoryHeader = observer(({ strategy, session, memorySession }:
{strategy: MemoryHeaderStrategy; session: Session; memorySession: MemorySession}) => {
    const [hostCondition, setHostCondition] = useState<ConditionType>(memorySession.hostCondition);
    const [rankIdCondition, setRankIdCondition] = useState<ConditionType>(memorySession.rankIdCondition);
    const [groupId, setGroupId] = useState<string>(memorySession.groupId);
    const isCompare: boolean = session.compareRank.isCompare;
    const { t } = useTranslation('memory');
    const hit = useHit();
    const groupByCondition = groupByOptions.map(item => ({
        ...item,
        label: t(`searchCriteria.${item.label}`),
    }));

    const onHostChanged = (value: string): void => {
        setHostCondition({ ...hostCondition, value });
        runInAction(() => {
            memorySession.hostCondition = { ...hostCondition, value };
        });
    };

    const onRankIdChanged = (value: string): void => {
        setRankIdCondition({ ...rankIdCondition, value });
        runInAction(() => {
            memorySession.rankIdCondition = { ...rankIdCondition, value };
        });
    };

    const onGroupByChanged = (value: string): void => {
        setGroupId(value);
        runInAction(() => {
            memorySession.groupId = value;
        });
    };

    useEffect(() => {
        const { hosts, ranks } = GroupRankIdsByHost(session.memoryRankIds);
        setHostCondition({ options: hosts, value: hosts[0] ?? '', ranks });
        runInAction(() => {
            memorySession.hostCondition = { ...hostCondition, options: hosts, value: hosts[0] ?? '', ranks };
        });
    }, [joinStringArray(session.memoryRankIds)]);

    useEffect(() => {
        // 只对RankId为数字做排序，不能转为数字的字符串则不排序
        const rankIdOptions: string[] = JSON.parse(JSON.stringify(hostCondition.ranks?.get(hostCondition.value) ?? []))
            .sort((a: any, b: any) => Number(a) - Number(b));
        if (rankIdOptions.length === 0) {
            setRankIdCondition({ options: [], value: '' });
            runInAction(() => {
                memorySession.rankIdCondition = { options: [], value: '' };
            });
            return;
        }
        const rankIdValue = (rankIdCondition.value === undefined || rankIdCondition.value === '') ? rankIdOptions[0] : rankIdCondition.value;
        setRankIdCondition({ options: rankIdOptions, value: rankIdValue });
        runInAction(() => {
            memorySession.rankIdCondition = { options: rankIdOptions, value: rankIdValue };
        });
    }, [hostCondition.options, hostCondition.value, hostCondition.ranks]);

    useEffect(() => {
        setRankIdCondition({ options: rankIdCondition.options, value: rankIdCondition.options[0] });
        runInAction(() => {
            memorySession.rankIdCondition = { options: rankIdCondition.options, value: rankIdCondition.options[0] };
        });
    }, [session.isClusterMemoryCompletedSwitch]);

    useEffect(() => {
        if (session.compareRank.rankId === rankIdCondition.value) {
            return;
        }
        if (session.memoryRankIds.includes(session.compareRank.rankId)) {
            onRankIdChanged(session.compareRank.rankId);
        }
    }, [session.compareRank.rankId, joinStringArray(session.memoryRankIds)]);

    const renderFields = (): JSX.Element[] => {
        const fields = [
            {
                key: 'host',
                element:
                    <FlexDiv>
                        <Label name={t('searchCriteria.Host')} />
                        <Select
                            id={'select-host'}
                            value={hostCondition.value}
                            size="middle"
                            onChange={onHostChanged}
                            disabled={isCompare}
                            options={hostCondition.options.map((host: string) => {
                                return { value: host, label: host };
                            })}
                        />
                    </FlexDiv>,
            },
            {
                key: 'rankId',
                element:
                    <FlexDiv>
                        <Label name={t('searchCriteria.RankId')} />
                        <Select
                            id={'select-rankId'}
                            value={rankIdCondition.value}
                            size="middle"
                            onChange={onRankIdChanged}
                            disabled={isCompare}
                            options={rankIdCondition.options.map((rankId: string) => {
                                return {
                                    value: rankId,
                                    label: rankId.replace(`${hostCondition.value} `, ''),
                                };
                            })}
                        />
                    </FlexDiv>,
            },
            {
                key: 'groupId',
                element:
                    <FlexDiv>
                        <Label name={<span>{t('searchCriteria.GroupBy')}{hit}</span>} />
                        <Select
                            id={'select-groupId'}
                            value={groupId}
                            style={{ width: 180 }}
                            onChange={onGroupByChanged}
                            options={groupByCondition}
                        />
                    </FlexDiv>,
            },
        ];
        return fields
            .filter((field) => strategy.shouldDisplay(field.key))
            .map((field) => field.element);
    };

    return (
        <div className="mb-30">
            <SearchBox>{renderFields()}</SearchBox>
        </div>
    );
});

export default MemoryHeader;
