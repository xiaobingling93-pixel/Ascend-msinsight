/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { cloneElement, useState, useEffect } from 'react';
import { Table } from 'antd';
import type { ColumnsType } from 'antd/es/table';
import Resizor from './Resizor';
type ExtendsColumnType = {minWidth?: number};

const ResizableTitle = (
    props: React.HTMLAttributes<HTMLDivElement> & {
        index?: number;
        className: string;
        resizable: boolean;
        onResize: Function;
    },
): JSX.Element => {
    const { onResize, resizable, index, ...restProps } = props;
    const th = <th {...restProps}/>;
    if (props?.className?.includes('ant-table-row-expand-icon-cell') || !resizable) {
        return th;
    }
    return cloneElement(th, {},
        [ ...th.props.children,
            <Resizor key={th.props.children.length} onResize={onResize}/> ]);
};

// eslint-disable-next-line max-lines-per-function
const ResizeTable = (props: {
    [prop: string]: object | number | string | boolean | undefined | object[];
    id?: string;
    columns: ColumnsType<object> ;
    variableTotalWidth?: boolean;
    minThWidth?: number;
    style?: object;
}): JSX.Element => {
    const { columns: propColumns, variableTotalWidth = false, minThWidth = 50, id, ...restProps } = props;
    const [ columns, setColumns ] = useState < ColumnsType<object>>(propColumns ?? []);
    useEffect(() => {
        setColumns(propColumns ?? []);
    }, [JSON.stringify(propColumns)]);

    const mergeColumns = columns.map((col, index) => ({
        ...col,
        onHeaderCell: () => ({
            onResize: (deltaX: number, width: number, nextWidth?: number): void => {
                const newColumns = [...columns];
                newColumns[index] = {
                    ...newColumns[index],
                    width: Math.max(width, minThWidth, (columns[index] as ExtendsColumnType).minWidth ?? 0),
                };
                if (nextWidth !== null && nextWidth !== undefined && !variableTotalWidth) {
                    newColumns[index + 1] = {
                        ...newColumns[index + 1],
                        width: Math.max(nextWidth, minThWidth, (columns[index + 1] as ExtendsColumnType).minWidth ?? 0),
                    };
                }
                setColumns(newColumns);
            },
            resizable: variableTotalWidth || index !== props.columns.length - 1,
        }),
    }));

    return (
        <div id={id} style={{ ...props.style ?? {} }}>
            <Table
                { ...restProps }
                className={!variableTotalWidth ? '' : 'variableTotalWidth'}
                columns={mergeColumns}
                components={{
                    header: {
                        cell: ResizableTitle,
                    },
                }}
            />
        </div>
    );
};
export default ResizeTable;
