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

import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ResizeTable } from '@insight/lib/resize';
import { Tooltip } from '@insight/lib/components';
import { Advice } from '@insight/lib/utils';
import { AntdTableRow, GetSlowRankAdviseRes, TopElements } from '../../utils/interface';
import type { GenerateConditions } from '../../store/parallelism';
import eventBus from '../../utils/eventBus';

interface SlowRankContainerProps {
    generateConditions: GenerateConditions;
    slowRankRes: GetSlowRankAdviseRes;
}

export const SlowRankTable = ({ generateConditions, slowRankRes }: SlowRankContainerProps): JSX.Element | null => {
    const { t } = useTranslation('summary');
    const [slowRankAnalysis, setSlowRankAnalysis] = useState<JSX.Element | null>(null);
    const generateHeaders = (topNElementData: TopElements): string[] => {
        let fixedHeader = 'slowGroupsTopN';
        const dynamicHeaders = [];
        if (topNElementData.dpSynchronizeTime !== undefined) { dynamicHeaders.push('dpSynchronizeTime'); }
        if (topNElementData.cpSynchronizeTime !== undefined) { dynamicHeaders.push('cpSynchronizeTime'); }
        if (topNElementData.tpSynchronizeTime !== undefined) {
            dynamicHeaders.push('tpSynchronizeTime');
            fixedHeader = 'slowRanksTopN';
        }
        return [fixedHeader, ...dynamicHeaders];
    };

    const generateAntdTableData = (headers: string[], tableData: TopElements[]): AntdTableRow[] => {
        if (tableData.length === 0 || headers.length === 0) {
            return [];
        }
        return tableData.map((item, index) => {
            const rowData: AntdTableRow = {
                key: `${item.index}`,
                index: item.index,
            };
            rowData[headers[0]] = `${item.name} ${t('number')}(${item.index})`;
            for (let i = 1; i < headers.length; i++) {
                rowData[headers[i]] = item[headers[i] as keyof TopElements] ?? ''; // 如果字段不存在，默认空字符串
            }
            return rowData;
        });
    };

    const selectNum = (row: TopElements): void => {
        eventBus.emit('selectSlowRanksTopNum', row.index);
    };

    const generateColumns = (dataKeys: string[]): Array<{dataIndex: string; key: string; title: JSX.Element}> => {
        return dataKeys.map(key => ({
            dataIndex: key,
            key,
            title: (<Tooltip title={t(`SlowRank ToolTip.${key}`)}>
                <span>{t(key)}</span>
            </Tooltip>),
            ...(['slowRanksTopN', 'slowGroupsTopN'].includes(key) ? { render: (text: string, row: TopElements) => <a onClick={(): void => selectNum(row)}>{text}</a> } : {}),
        }));
    };
    const isDefaultParams = generateConditions?.dpSize === 1 && generateConditions?.tpSize === 1 && generateConditions?.ppSize === 1 && generateConditions.cpSize === 1 && generateConditions?.epSize === 1;
    const fetchData = async (): Promise<void> => {
        const params = {
            ...generateConditions,
        };
        if (params.dimension === 'ep-dp') {
            setSlowRankAnalysis(null);
            return;
        }
        if (!slowRankRes.matchSuccess) {
            setSlowRankAnalysis(null);
            return;
        }
        if (!slowRankRes.hasSlowRank && !isDefaultParams) {
            const res: JSX.Element = (
                <div data-testid={'slow-rank-expert-advice'}>
                    <Advice text={t('No problem')} />
                </div>
            );
            setSlowRankAnalysis(res);
            return;
        }
        if (slowRankRes.topNElements.length === 0) {
            setSlowRankAnalysis(null);
            return;
        }
        const keys = generateHeaders(slowRankRes.topNElements[0]);
        const column = generateColumns(keys);
        const tableData = generateAntdTableData(keys, slowRankRes.topNElements);

        const resDiv: JSX.Element = (
            <div data-testid={'slow-rank-expert-advice'}>
                <div>
                    <Advice text={t('slow rank advice')} />
                </div>
                <ResizeTable
                    columns={column}
                    dataSource={tableData}
                    size="small"
                ></ResizeTable>
            </div>
        );
        setSlowRankAnalysis(resDiv);
    };

    useEffect(() => {
        fetchData();
    }, [slowRankRes, t]);

    return slowRankAnalysis;
};
