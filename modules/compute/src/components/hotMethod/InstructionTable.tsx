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
import React from 'react';
import type { TFunction } from 'i18next';
import type { ColumnsType, ColumnType } from 'antd/es/table';
import type { AlignType } from 'rc-table/lib/interface';
import type { InstrsColumnType } from './defs';
import { FieldType, NOT_APPLICABLE } from './defs';
import { Tooltip } from '@insight/lib/components';
import Bar, { BarType, StallBar } from './Bar';
import { limitInput } from '@insight/lib/utils';
import { RegisterDependency } from '@/components/hotMethod/RegisterDependency';

const onFilterDropdownOpenChange = (open: boolean): void => {
    if (open) {
        limitInput();
    }
};

const isNegativeNum = (item: any): boolean => {
    return !isNaN(Number(item)) && Number(item) < 0;
};
// 筛选列，是数字且小于0，用NA表示
const getFilterText = (item: any, dataIndex?: string): any => {
    if (isNegativeNum(item)) {
        return NOT_APPLICABLE;
    } else if (dataIndex === 'L2Cache Hit Rate') {
        return isNaN(Number(item)) ? NOT_APPLICABLE : Number(item);
    } else {
        return item;
    }
};

const getFilters = (curInstrData: InstrsColumnType[], dataIndex: string): any[] => {
    const items = [...new Set(curInstrData.map(item => item[dataIndex as keyof InstrsColumnType]))];
    let hasNegative = false;
    return items.reduce<any[]>((pre, cur) => {
        const text = getFilterText(cur, dataIndex);
        if (text === NOT_APPLICABLE) {
            if (!hasNegative) {
                pre.push({ text: NOT_APPLICABLE, value: NOT_APPLICABLE });
                hasNegative = true;
            }
        } else {
            pre.push({ text, value: cur });
        }
        return pre;
    }, []);
};

// 更新指令表的显示列
export const getInstrColumns = (dynamicFields: Record<string, FieldType>, t: TFunction, curInstrData: InstrsColumnType[]): ColumnsType<InstrsColumnType> => {
    const columns: ColumnsType<InstrsColumnType> = getDynamicInstrColumns(t, dynamicFields, curInstrData);
    // 没有有筛选功能的列
    const unfilterableCols = ['index', 'RealStallCycles', 'Register Dependencies'];
    columns.forEach((col: ColumnType<InstrsColumnType>) => {
        if (col.dataIndex !== undefined && !unfilterableCols.includes(String(col.dataIndex))) {
            const filters = getFilters(curInstrData, String(col.dataIndex));

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
    { title: '#', dataIndex: 'index', width: 60, align: 'right' as AlignType, ellipsis: true, sorter: true },
    {
        title: 'Source',
        dataIndex: 'Source',
        sorter: true,
        ellipsis: { showTitle: false },
        render: (source: string): React.ReactNode => (
            <Tooltip placement="topLeft" title={source} >
                {source}
            </Tooltip>
        ),
    },
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
        ellipsis: true,
        width: 115,
        render: (realStallCycles: number | string, record: InstrsColumnType): string | React.ReactElement =>
            <StallBar real={record.RealStallCycles as number} theoretical={record.TheoreticalStallCycles as number}/>,
        className: 'height20',
    },
    {
        title: 'GPR Status',
        ellipsis: true,
        width: 115,
        render: (record: InstrsColumnType): string | React.ReactElement => {
            return <RegisterDependency
                tracks={record['GPR Status']}
                cellPadding={6} />;
        },
        className: 'height20',
    },
];

export const percentageColConfig = {
    width: 120,
    ellipsis: true,
    render: (percent: number): React.ReactNode => {
        return <Bar value={percent} type={BarType.PERCENT}/>;
    },
    className: 'height20',
};

interface IParams<T> {
    colName: string;
    fieldType: FieldType;
    presetCols: ColumnsType<T>;
    t: TFunction;
    defaultSort?: boolean;
}
export const getColConfig = <T extends object>({ colName, fieldType, presetCols, t, defaultSort = true }: IParams<T>): ColumnType<T> => {
    const col = presetCols.find(colConfig => colConfig.title === colName);
    if (!col && (fieldType === FieldType.PERCENTAGE || colName === 'L2Cache Hit Rate')) {
        return {
            ...percentageColConfig,
            sorter: defaultSort,
            title: t(colName),
            dataIndex: colName,
        };
    }
    return col
        ? { ...col, title: t(colName) }
        : {
            ellipsis: true,
            sorter: defaultSort,
            title: t(colName),
            dataIndex: colName,
            // 数据是int或者float时，数值为-1显示为NA
            render: (value: React.Key): React.ReactNode =>
                [FieldType.INT, FieldType.FLOAT].includes(fieldType) && typeof value === 'number' && value < 0 ? NOT_APPLICABLE : value,
        };
};

const checkHasStallCycles = (curInstrData: InstrsColumnType[]): boolean => {
    for (let i = 0; i < curInstrData.length; i++) {
        if (curInstrData[i].RealStallCycles <= 0 && curInstrData[i].TheoreticalStallCycles <= 0) {
            continue;
        }
        return curInstrData[i].RealStallCycles >= 0 && curInstrData[i].TheoreticalStallCycles >= 0;
    }
    return false;
};

// 默认显示列
const defaultCols = ['#', 'Address', 'Pipe', 'Source', 'Instructions Executed', 'Cycles'];
// 如果存在放在前面
const headerCols = ['#', 'Address', 'Pipe', 'Source', 'Instructions Executed'];
// 如果存在放在最后的列
const endCols = ['Cycles', 'StallCycles'];
// 不显示的列
const notDisplayedCols = ['AscendC Inner Code', 'RealStallCycles', 'TheoreticalStallCycles'];
const noSortCols = ['GPR Status'];

export const getDynamicInstrColumns = (t: TFunction, dynamicFields: Record<string, FieldType> = {}, curInstrData: InstrsColumnType[] = []):
ColumnsType<InstrsColumnType> => {
    const dynamicCols = Object.keys(dynamicFields).filter(col => dynamicFields[col] !== FieldType.SKIP);
    // 兼容无动态列版本
    const allCols = dynamicCols.length === 0 ? [...defaultCols] : ['#', ...dynamicCols];
    if ((dynamicCols.length === 0 && checkHasStallCycles(curInstrData)) || dynamicCols.includes('RealStallCycles')) {
        allCols.push('StallCycles');
    }
    const cols = [...headerCols.filter(colName => allCols.includes(colName)),
        ...allCols.filter(colName => !headerCols.includes(colName) && !notDisplayedCols.includes(colName) && !endCols.includes(colName)),
        ...endCols.filter(colName => allCols.includes(colName)),
    ];
    return cols.map(colName => getColConfig<InstrsColumnType>(
        { colName, fieldType: dynamicFields[colName], presetCols: instrsColsConfig, t, defaultSort: !noSortCols.includes(colName) }));
};
