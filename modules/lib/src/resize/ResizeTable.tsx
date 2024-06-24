/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { cloneElement, useState, useEffect, useRef, useCallback, useMemo } from 'react';
import { Table } from 'antd';
import type { ColumnsType } from 'antd/es/table';
import Resizor from './Resizor';
import { limitInput } from '../utils/Common';
interface ExtendsColumnType {minWidth?: number};

interface IResizableTitleProps extends React.ReactElement {
    index?: number;
    className: string;
    resizable: boolean;
    onResize: (deltaX: number, width: number, nextWidth?: number) => void;
}
const resizableTitle: React.FC<IResizableTitleProps> = (props) => {
    const { onResize, resizable, index, ...restProps } = props;
    const th: React.ReactElement = <th {...restProps}/> as React.ReactElement;
    if (props?.className?.includes('ant-table-row-expand-icon-cell') || !resizable) {
        return th;
    }
    return cloneElement(th, {},
        [...th.props.children,
            <Resizor key={th.props.children.length} onResize={onResize}/>]);
};

interface Iprop<T> {
    [prop: string]: object | number | string | boolean | undefined | object[];
    columns: ColumnsType<T> ;
    dataSource: T[];
    size?: 'small' | 'middle' | 'large' ;
    id?: string;
    variableTotalWidth?: boolean;
    minThWidth?: number;
    style?: object;
    virtual?: boolean;
    scroll?: {x?: number;y?: number;rowHeight?: number};
    pagination?: {showSizeChanger: boolean};
}

// ============================ Resize ============================
const getResizeColumns = ({ columns, index, width, nextWidth, minThWidth, variableTotalWidth }:
{columns: any[];index: number; width: number; nextWidth?: number;minThWidth: number;variableTotalWidth: boolean}): any[] => {
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
    return newColumns;
};

// ============================ virtual ============================
const getTableSize = (prop:
{
    scroll?: {x?: number;y?: number;rowHeight?: number};
    dataSource: unknown[];
},
): { [prop: string]: number} => {
    const { scroll, dataSource } = prop;
    const visibleHeight = scroll?.y ?? 0;
    const itemHeight = scroll?.rowHeight ?? 39;
    const visibleCount = Math.ceil(visibleHeight / itemHeight) + 2;
    const totalCount = dataSource.length;
    const totalHeight = itemHeight * totalCount;
    return { visibleHeight, itemHeight, visibleCount, totalCount, totalHeight };
};
const handleScrollEvent = (e: any, { virtual, itemHeight, visibleCount, tableRef, setRange }:
{ virtual: boolean;
    itemHeight: number;
    visibleCount: number;
    tableRef: React.MutableRefObject<HTMLElement | undefined>;
    setRange: React.Dispatch<React.SetStateAction<[number, number]>>;
},
): void => {
    if (!virtual || itemHeight === 0) {
        return;
    }
    const startIdx = Math.floor(e.target.scrollTop / itemHeight);
    const endIdx = startIdx + visibleCount;
    setRange([startIdx, endIdx]);
    const offset = startIdx * itemHeight;
    if (tableRef?.current !== undefined) {
        (tableRef.current as any).style.top = `${offset}px`;
    }
};
const addVirtualEvent = ({ virtual, divRef, tableRef, scrollEvent, height }:
{
    virtual: boolean;
    divRef: React.MutableRefObject<null>;
    tableRef: React.MutableRefObject<HTMLElement | undefined>;
    scrollEvent: (e: any) => void;
    height: number;
},
) : (() => void) => {
    const voidFun = (): void => {};
    if (!virtual || divRef.current === null) {
        return voidFun;
    }
    const parentNode = (divRef.current as HTMLElement).querySelector('.ant-table-body') as HTMLElement;
    const table = (divRef.current as HTMLElement).querySelector('.ant-table-body table') as HTMLElement;
    if (table === null || parentNode === null) {
        return voidFun;
    }
    tableRef.current = table;
    // 占位div
    const placeholderWrapper = document.createElement('div');
    placeholderWrapper.style.height = `${height}px`;
    parentNode.appendChild(placeholderWrapper);
    parentNode.style.position = 'relative';
    if (height === 0) {
        Object.assign(table.style, { position: 'relative' });
    } else {
        Object.assign(table.style, { position: 'absolute', top: '0', left: '0' });
    }
    // 添加滚动事件
    parentNode.addEventListener('scroll', scrollEvent);
    parentNode.scrollTo({ top: 0 });
    return (): void => {
        // 清理占位div
        parentNode.removeChild(placeholderWrapper);
        parentNode.removeEventListener('scroll', scrollEvent);
    };
};

function ResizeTable<T extends object>(prop: Iprop<T>): JSX.Element {
    const { columns: propColumns, variableTotalWidth = false, minThWidth = 50, id, style, virtual = false, scroll, dataSource, ...restProps } = prop;
    const [columns, setColumns] = useState<ColumnsType<T>>(propColumns ?? []);

    // ============================ Resize ============================
    useEffect(() => {
        setColumns(propColumns ?? []);
    }, [JSON.stringify(propColumns)]);
    const mergeColumns: any = columns.map((col, index) => ({
        ...col,
        onHeaderCell: () => ({
            onResize: (deltaX: number, width: number, nextWidth?: number): void => {
                const newColumns = getResizeColumns({ columns, index, width, nextWidth, minThWidth, variableTotalWidth });
                setColumns(newColumns);
            },
            resizable: variableTotalWidth || index !== propColumns.length - 1,
        }),
    }));

    // ============================ virtual ============================
    const myRef = useRef(null);
    const tableRef = useRef<HTMLElement>();
    const [range, setRange] = useState<[number, number]>([0, 0]);
    const { itemHeight, visibleCount, totalHeight } = useMemo(() => getTableSize(prop), [dataSource, scroll]);
    const scrollEvent = useCallback((e: any) => {
        handleScrollEvent(e, { virtual, itemHeight, visibleCount, tableRef, setRange });
    }, [visibleCount, totalHeight, virtual]);
    useEffect(() => {
        setRange([0, visibleCount]);
        return addVirtualEvent({ virtual, divRef: myRef, tableRef, scrollEvent, height: totalHeight });
    }, [scrollEvent, totalHeight, virtual]);
    const renderList = useMemo(() => dataSource.slice(...range), [dataSource, range]);

    // 出现分页跳转输入框
    useEffect(() => {
        if (prop.pagination?.showSizeChanger) {
            limitInput();
        }
    }, [prop.pagination]);

    return (
        <div id={id} style={{ ...style ?? {} }} ref={myRef}>
            <Table
                { ...restProps }
                scroll={scroll}
                dataSource={virtual ? renderList : dataSource}
                className={!variableTotalWidth ? '' : 'variableTotalWidth'}
                columns={mergeColumns}
                components={{
                    header: {
                        cell: resizableTitle,
                    },
                }}
            />
        </div>
    );
};

export default ResizeTable;
