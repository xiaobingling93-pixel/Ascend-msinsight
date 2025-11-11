/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { SearchBox, FlexDiv } from '../utils/styleUtils';
import { Session } from '../entity/session';
import { CurveSession } from '../entity/curveSession';
import { Label } from './Common';
import { Select } from '@insight/lib/components';
import { notNull } from '@insight/lib/utils';
import { useTranslation } from 'react-i18next';
import { groupGet } from '../utils/RequestUtils';

const CurveHeader = observer(({ session, curveSession }:
{ session: Session; curveSession: CurveSession}) => {
    const { t } = useTranslation('statistic');
    const groupByOptions = async (rankId: string): Promise<void> => {
        const options: Array<{ label: string; value: string }> = [];
        const res = await groupGet({ rankId });
        res.groups.forEach(item => {
            options.push({ label: item.label, value: item.label });
        });
        if (options.length > 0) {
            onGroupByChanged(options[0].value);
        }
        runInAction(() => {
            curveSession.groupCondition = options;
        });
    };

    const onRankIdChanged = (value: string): void => {
        runInAction(() => {
            curveSession.rankIdCondition = { ...curveSession.rankIdCondition, value };
        });
    };

    const onGroupByChanged = (value: string): void => {
        runInAction(() => {
            curveSession.groupId = value;
            curveSession.current = 1;
        });
    };

    useEffect(() => {
        runInAction(() => {
            curveSession.rankIdCondition = {
                options: session.iERankIds,
                value: notNull(session.compareRank.rankId) ? session.compareRank.rankId : (session.iERankIds[0] ?? ''),
            };
            const rankId: string = curveSession.rankIdCondition.value;
            groupByOptions(rankId);
        });
    }, [session.iERankIds.join('')]);
    const renderFields = (): JSX.Element[] => {
        const fields = [
            {
                key: 'rankId',
                element:
                    <FlexDiv>
                        <Label name={t('searchCriteria.RankId')} />
                        <Select
                            id={'select-rankId'}
                            value={curveSession.rankIdCondition.value}
                            size="middle"
                            onChange={onRankIdChanged}
                            disabled={false}
                            options={curveSession.rankIdCondition.options.map((rankId: string) => {
                                return {
                                    value: rankId,
                                    label: rankId,
                                };
                            })}
                        />
                    </FlexDiv>,
            },
            {
                key: 'groupId',
                element:
                    <FlexDiv>
                        <Label name={<span>{t('searchCriteria.GroupBy')}</span>} />
                        <Select
                            id={'select-groupId'}
                            value={curveSession.groupId}
                            style={{ width: 180 }}
                            onChange={onGroupByChanged}
                            options={curveSession.groupCondition}
                        />
                    </FlexDiv>,
            },
        ];
        return fields
            .map((field) => field.element);
    };

    return (
        <div className="mb-30">
            <SearchBox>{renderFields()}</SearchBox>
        </div>
    );
});

export default CurveHeader;
