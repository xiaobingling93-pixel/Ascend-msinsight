/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ResizeTable } from 'ascend-resize';
import { Advice } from 'ascend-utils/Common';
import { AntdTableRow, GetSlowRankAdviseRes, TopElements } from '../../utils/interface';
import type { GenerateConditions } from '../../store/parallelism';

interface SlowRankContainerProps {
    generateConditions: GenerateConditions;
    slowRankRes: GetSlowRankAdviseRes;
}

export const SlowRankTable = ({ generateConditions, slowRankRes }: SlowRankContainerProps): JSX.Element | null => {
    const { t } = useTranslation('summary');
    const [slowRankAnalysis, setSlowRankAnalysis] = useState<JSX.Element | null>(null);
    const generateHeaders = (topNElementData: TopElements): string[] => {
        const fixedHeader = 'slowRankTopN';
        const dynamicHeaders = [];
        if (topNElementData.dpSynchronizeTime !== undefined) { dynamicHeaders.push('dpSynchronizeTime'); }
        if (topNElementData.cpSynchronizeTime !== undefined) { dynamicHeaders.push('cpSynchronizeTime'); }
        if (topNElementData.tpSynchronizeTime !== undefined) { dynamicHeaders.push('tpSynchronizeTime'); }
        return [fixedHeader, ...dynamicHeaders];
    };

    const generateAntdTableData = (headers: string[], tableData: TopElements[]): AntdTableRow[] => {
        if (tableData.length === 0 || headers.length === 0) {
            return [];
        }
        return tableData.map((item, index) => {
            const rowData: AntdTableRow = {
                key: `${item.index}`,
            };
            rowData[headers[0]] = `${item.name} ${t('number')}(${item.index})`;
            for (let i = 1; i < headers.length; i++) {
                rowData[headers[i]] = item[headers[i] as keyof TopElements] ?? ''; // 如果字段不存在，默认空字符串
            }
            return rowData;
        });
    };

    const generateColumns = (dataKeys: string[]): Array<{dataIndex: string; key: string; title: string}> => {
        return dataKeys.map(key => ({
            dataIndex: key,
            key,
            title: t(key),
        }));
    };

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
        if (!slowRankRes.hasSlowRank) {
            const res: JSX.Element = (
                <div>
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
            <div>
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
