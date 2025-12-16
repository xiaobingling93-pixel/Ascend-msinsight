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
import type { TFunction } from 'i18next';
import { type IblockData } from './Index';
import { ResizeTable } from '@insight/lib/resize';
import { getSet, firstLetterUpper } from '@insight/lib/utils';
import { LimitHit } from '../../LimitSet';
import { CompareData } from '../../../utils/interface';
import { Icondition } from './Filter';
import { type Theme, useTheme } from '@emotion/react';
import { getContextElement, renderExpandColumn } from '../../Common';

interface Iprops {
    condition: Icondition;
    data: Array<CompareData<IblockData>>;
}

interface Iobj {
    [x: string]: string | Iobj[];
}

interface ItableConfig {
    blockType: string;
    cols: any[];
    dataset: Iobj[];
}

function getFullCols({ blockType, blockTypeData, t, setExpandedKeys, isCompared, theme }: {blockType: string;
    blockTypeData: Array<CompareData<IblockData>>; t: TFunction; setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>;
    isCompared: boolean; theme: Theme;}): any[] {
    const firstCol = {
        title: blockType?.toUpperCase(),
        dataIndex: 'name',
        ellipsis: true,

    };
    const units = getSet(getSet(blockTypeData, 'compare') as IblockData[], 'unit') as string[];
    const restCols = units.map((item, index) => (
        {
            title: t(firstLetterUpper(item)),
            dataIndex: item,
            ellipsis: true,
            render: (text: string, record: any): JSX.Element => getContextElement(text, record, theme, t),
        }
    ));
    if (isCompared) {
        restCols.push({
            title: t('Details'),
            dataIndex: 'action',
            ellipsis: true,
            render: (text: string, record: any): JSX.Element => {
                return renderExpandColumn(record, setExpandedKeys, t);
            },
        });
        restCols.splice(0, 0, {
            title: t(firstLetterUpper('source')),
            dataIndex: 'source',
            ellipsis: true,
            render: (text: string, record: any): JSX.Element => <div>{text}</div>,
        });
    }
    return [firstCol, ...restCols];
}

function getRowBaseData(data: IblockData): Iobj {
    const dataObj: Iobj = {};
    if (dataObj[data.name] === undefined) {
        dataObj.name = data.name;
    }
    dataObj[data.unit] = data.value;
    return dataObj;
}

function Index({ condition, data }: Iprops): JSX.Element {
    const [tablelist, setTablelist] = useState<ItableConfig[]>([]);
    const [limit, setLimit] = useState({ overlimit: false, maxSize: 5000, current: 0 });
    const { t } = useTranslation('details');
    const theme = useTheme();
    const [expandedRowKeys, setExpandedKeys] = React.useState<string[]>([]);

    const updateTable = (): void => {
        const allData = data.filter(item => item.compare.blockId === condition.blockId);
        setLimit({ ...limit, overlimit: allData.length > limit.maxSize, current: allData.length });
        const showData = allData.slice(0, limit.maxSize);
        // 按照blockType分表
        const blockTypeSet = getSet(getSet(showData, 'compare') as IblockData[], 'blockType') as string[];
        const dataGroupByBlockType = blockTypeSet.map(blockType => {
            const blockTypeData = showData.filter(item => item.compare.blockType === blockType);
            const cols = getFullCols(
                { blockType, blockTypeData, t, setExpandedKeys, isCompared: condition.isCompared, theme });
            const dataset: Iobj[] = blockTypeData.map(item => {
                const compare: Iobj = getRowBaseData(item.compare);
                if (!condition.isCompared) {
                    return compare;
                }
                compare.source = t('Comparison');
                const diff = getRowBaseData(item.diff);
                const baseline = getRowBaseData(item.baseline);
                diff.source = t('Difference');
                baseline.source = t('Baseline');
                diff.children = [compare, baseline] as Iobj[];
                return diff;
            });
            return { blockType, cols, dataset };
        });
        setTablelist(dataGroupByBlockType);
    };
    useEffect(() => {
        updateTable();
    }, [condition, data, t]);
    return (
        <div data-testId="computeWorkloadTable">
            {tablelist.length === 0 && (<div style={{ textAlign: 'center', color: 'var(--grey15) ' }}>No data</div>) }
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${t('All Instruction Records')} (${limit.current})`}/>}
            {tablelist.map(item => (
                <ResizeTable
                    key={item.blockType}
                    size="small"
                    columns={item.cols ?? []}
                    dataSource={item.dataset.map((row, rowIndex) => { return { ...row, key: `${item.blockType}_${rowIndex}` }; })}
                    scroll={item.dataset.length > 10 ? { y: 500 } : undefined}
                    pagination={false}
                    expandable={{
                        expandIcon: () => <></>,
                        expandedRowKeys,
                    }}
                />
            ))}
        </div>
    );
}

export default Index;
