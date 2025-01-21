/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import type { TFunction } from 'i18next';
import type { ColumnsType, ColumnType } from 'antd/es/table';
import type { AlignType } from 'rc-table/lib/interface';
import type { InstrsColumnType } from './defs';
import { Tooltip } from 'ascend-components';
import Bar, { StallBar } from './Bar';
import { limitInput } from 'ascend-utils';

const onFilterDropdownOpenChange = (open: boolean): void => {
    if (open) {
        limitInput();
    }
};
// 更新指令表的显示列
export const getInstrColumns = (dynamicCols: string[], t: TFunction, curInstrData: InstrsColumnType[]): ColumnsType<InstrsColumnType> => {
    const columns: ColumnsType<InstrsColumnType> = getDynamicInstrColumns(t, dynamicCols);
    // 没有有筛选功能的列
    const unfilterableCols = ['index', 'RealStallCycles'];
    columns.forEach((col: ColumnType<InstrsColumnType>) => {
        if (col.dataIndex !== undefined && !unfilterableCols.includes(String(col.dataIndex))) {
            const items = [...new Set(curInstrData.map(item => item[col.dataIndex as keyof InstrsColumnType]))];
            const filters = items.map(item => ({
                text: item,
                value: item,
            }));
            Object.assign(col, {
                filters,
                filterMode: 'menu',
                filterSearch: true,
                onFilterDropdownOpenChange,
            });
        }
    });
    return columns;
};

const instrsColsConfig = [
    { title: '#', dataIndex: 'index', width: 50, align: 'right' as AlignType, ellipsis: true },
    { title: 'Address', dataIndex: 'Address', ellipsis: true },
    { title: 'Pipe', dataIndex: 'Pipe', ellipsis: true },
    {
        title: 'Source',
        dataIndex: 'Source',
        ellipsis: { showTitle: false },
        render: (source: string): React.ReactNode => (
            <Tooltip placement="topLeft" title={source} >
                {source}
            </Tooltip>
        ),
    },
    { title: 'Instructions Executed', dataIndex: 'Instructions Executed', ellipsis: true, sorter: true },
    {
        title: 'Cycles',
        dataIndex: 'Cycles',
        width: 150,
        ellipsis: true,
        sorter: true,
        render: (cycles: number | string, record: InstrsColumnType): string | React.ReactElement => {
            if (cycles === '') {
                return '';
            }
            return <Bar value={Number(cycles)} max={record.maxCycles ?? cycles}/>;
        },
        className: 'height20',
    },
    {
        title: 'StallCycles',
        dataIndex: 'RealStallCycles',
        ellipsis: true,
        width: 115,
        render: (realStallCycles: number | string, record: InstrsColumnType): string | React.ReactElement =>
            <StallBar real={record.RealStallCycles as number} theoretical={record.TheoreticalStallCycles as number}/>,
        className: 'height20',
    },
    { title: 'RegisterNum', dataIndex: 'RegisterNum', ellipsis: true },
];

export const getDynamicInstrColumns = (t: TFunction, fields: string[] = []): ColumnsType<InstrsColumnType> => {
    const allCols = [...fields];
    if (allCols.includes('RealStallCycles')) {
        allCols.push('StallCycles');
    }
    // 固定显示列
    const fixedCols = ['#', 'Address', 'Pipe', 'Source', 'Instructions Executed'];
    // 如果存在放在最后的列
    const endCols = ['Cycles', 'StallCycles'];
    // 不显示的列
    const notDisplayedCols = ['AscendC Inner Code', 'RealStallCycles', 'TheoreticalStallCycles', 'index', 'cycles', 'maxCycles'];
    const cols = fields.length === 0
        ? [...fixedCols, 'Cycles']
        : ['#', ...fixedCols.filter(colName => allCols.includes(colName)),
            ...allCols.filter(colName => !fixedCols.includes(colName) && !notDisplayedCols.includes(colName) && !endCols.includes(colName)),
            ...endCols.filter(colName => allCols.includes(colName)),
        ];
    return cols.map(colName => {
        const col = instrsColsConfig.find(colConfig => colConfig.title === colName);
        return col
            ? { ...col, title: t(colName) }
            : { title: t(colName), dataIndex: colName, ellipsis: true, sorter: true };
    });
};
