/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { cloneElement, useState, useEffect } from 'react';
import { Table } from 'antd';
import Resizor from './Resizor';

const ResizableTitle = (
    props: React.HTMLAttributes<any> & {
        index?: number;
        className: string;
        resizable: boolean;
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
const ResizeTable = (props: any): JSX.Element => {
    const { columns: propColumns, variableTotalWidth = false, minThWidth = 0, id, ...restProps } = props;
    const [ columns, setColumns ] = useState<any[]>([]);

    useEffect(() => {
        const newColumns = [...props.columns].map((item: any, index: number) => ({ ...item }));
        setColumns(newColumns);
    }, [props.columns]);

    const mergeColumns = columns.map((col, index) => ({
        ...col,
        onHeaderCell: () => ({
            onResize: (deltaX: number, width: number, nextWidth?: number): void => {
                const newColumns = [...columns];
                newColumns[index] = {
                    ...newColumns[index],
                    width: Math.max(width, minThWidth),
                };
                if (nextWidth !== null && nextWidth !== undefined && variableTotalWidth !== true) {
                    newColumns[index + 1] = {
                        ...newColumns[index + 1],
                        width: Math.max(nextWidth, minThWidth),
                    };
                }
                setColumns(newColumns);
            },
            resizable: variableTotalWidth !== true ? index !== props.columns.length - 1 : true,
        }),
    }));

    return (
        <div id={id} style={{ ...props.style ?? {} }}>
            <Table
                { ...restProps }
                className={variableTotalWidth !== true ? '' : 'variableTotalWidth'}
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
