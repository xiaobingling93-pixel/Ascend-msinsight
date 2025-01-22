/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Ilinetable } from './defs';
import type { TFunction } from 'i18next';
import type { ColumnsType } from 'antd/es/table';

// 列配置
const codeCols = [
    {
        title: 'Instructions Executed',
        dataIndex: 'Instructions Executed',
        ellipsis: true,
    },
];

// 固定显示列
const fixedCols = ['Instructions Executed', 'Cycles'];
// 不显示的列
const notDisplayedCols = ['Address Range', 'Line'];

export const getCodeColumns = (t: TFunction, fields: string[] = []): ColumnsType<Ilinetable> => {
    const cols = fields.length === 0
        ? fixedCols
        : [...fixedCols.filter(colName => fields.includes(colName)),
            ...fields.filter(colName => !fixedCols.includes(colName) && !notDisplayedCols.includes(colName)),
        ]
    ;
    return cols.map(colName => {
        const col = codeCols.find(colItem => colItem.title === colName);
        return col
            ? { ...col, title: t(colName) }
            : { title: t(colName), dataIndex: colName, ellipsis: true };
    });
};
