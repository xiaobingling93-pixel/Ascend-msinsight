/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { FlexDiv, SearchBox } from '../utils/styleUtils';
import { Session } from '../entity/session';
import { CurveSession } from '../entity/curveSession';
import { Label } from './Common';
import { Select } from '@insight/lib/components';
import { notNull } from '@insight/lib/utils';
import { useTranslation } from 'react-i18next';
import { groupGet } from '../utils/RequestUtils';

const CurveHeader = observer(({ session, curveSession }: { session: Session; curveSession: CurveSession }) => {
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
            // 联合导入时session.compareRank.rankId可能不是服务化的cardId，需校验session.compareRank.rankId是否匹配，不匹配默认取session.iERankIds[0]
            const isValidCompareRank = notNull(session.compareRank.rankId) && session.iERankIds.includes(session.compareRank.rankId);
            curveSession.rankIdCondition = {
                options: session.iERankIds,
                value: isValidCompareRank ? session.compareRank.rankId : (session.iERankIds[0] ?? ''),
            };
            const rankId: string = curveSession.rankIdCondition.value;
            groupByOptions(rankId);
        });
    }, [session.iERankIds.join('')]);
    const renderFields = (): JSX.Element[] => {
        return [
            <FlexDiv key="rankId">
                <Label name={t('searchCriteria.RankId')}/>
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
            <FlexDiv key="groupId">
                <Label name={<span>{t('searchCriteria.GroupBy')}</span>}/>
                <Select
                    id={'select-groupId'}
                    value={curveSession.groupId}
                    style={{ width: 180 }}
                    onChange={onGroupByChanged}
                    options={curveSession.groupCondition}
                />
            </FlexDiv>,
        ];
    };

    return (
        <div className="mb-30">
            <SearchBox>{renderFields()}</SearchBox>
        </div>
    );
});

export default CurveHeader;
