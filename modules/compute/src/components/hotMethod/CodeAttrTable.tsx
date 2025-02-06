/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Ilinetable } from './defs';
import type { TFunction } from 'i18next';
import type { ColumnsType } from 'antd/es/table';
import { FieldType, NOT_APPLICABLE } from './defs';
import React from 'react';

// 列配置
const codeCols: ColumnsType<Ilinetable> = [];

// 固定显示列
const fixedCols = ['Instructions Executed', 'Cycles'];
// 不显示的列
const notDisplayedCols = ['Address Range', 'Line'];

export const getCodeColumns = (t: TFunction, dynamicFields: Record<string, FieldType> = {}): ColumnsType<Ilinetable> => {
    const dynamicCols = Object.keys(dynamicFields).filter(col => dynamicFields[col] !== FieldType.SKIP);
    const cols = dynamicCols.length === 0
        ? fixedCols
        : [...fixedCols.filter(colName => dynamicCols.includes(colName)),
            ...dynamicCols.filter(colName => !fixedCols.includes(colName) && !notDisplayedCols.includes(colName)),
        ]
    ;
    return cols.map(colName => {
        const col = codeCols.find(colItem => colItem.title === colName);
        return col
            ? { ...col, title: t(colName) }
            : {
                title: t(colName),
                dataIndex: colName,
                ellipsis: true,
                // 数据是int或者float时，数值为-1显示为NA
                render: (value: React.Key): React.ReactNode =>
                    [FieldType.INT, FieldType.FLOAT].includes(dynamicFields[colName]) && typeof value === 'number' && value < 0 ? NOT_APPLICABLE : value,
            };
    });
};
