/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import classNames from 'classnames';
import ResizeObserver from 'rc-resize-observer';
import React, { useMemo } from 'react';
import { RefCell as Cell } from './Cell';
import { PerfContext, type PerfRecord, useBodyContext, useResizeContext, useTableContext } from './contexts';
import type { TreeViewModel } from './hooks/useOrderStatisticTree';
import type { GetComponentProps, GetRowKey, Key } from './types';
import { getColumnsKey } from './utils/valueUtil';
import type { EmotionJSX } from '@emotion/react/types/jsx-namespace';

interface MeasureCellProps {
    columnKey: React.Key;
    onColumnResize: (key: React.Key, width: number) => void;
}

function MeasureCell({ columnKey, onColumnResize }: MeasureCellProps): EmotionJSX.Element {
    const ref = React.useRef<HTMLTableCellElement>(null);
    React.useEffect(() => {
        if (ref.current) {
            onColumnResize(columnKey, ref.current.offsetWidth);
        }
    }, []);
  
    return (
        <ResizeObserver data={columnKey}>
            <td ref={ref} style={{ padding: 0, border: 0, height: 0 }}>
                <div style={{ height: 0, overflow: 'hidden' }}>&nbsp;</div>
            </td>
        </ResizeObserver>
    );
}

interface MeasureRowProps {
    prefixCls: string;
    onColumnResize: (key: React.Key, width: number) => void;
    columnKeys: React.Key[];
}

export function MeasureRow({ prefixCls, columnKeys, onColumnResize }: MeasureRowProps): EmotionJSX.Element {
    return (
        <tr className={`${prefixCls}-measure-row`} style={{ height: 0, fontSize: 0 }}>
            <ResizeObserver.Collection
                onBatchResize={(infoList): void => {
                    infoList.forEach(({ data: columnKey, size }) => {
                        onColumnResize(columnKey, size.offsetWidth);
                    });
                }}>
                {columnKeys.map(columnKey => (
                    <MeasureCell key={columnKey} columnKey={columnKey} onColumnResize={onColumnResize} />
                ))}
            </ResizeObserver.Collection>
        </tr>
    );
}

interface BodyRowProps<RecordType> {
    record: RecordType;
    index: number;
    className?: string;
    style?: React.CSSProperties;
    recordKey: Key;
    expandedKeys: Set<Key>;
    onRow?: GetComponentProps<RecordType>;
    indent?: number;
    rowKey: React.Key;
    getRowKey?: GetRowKey<RecordType>;
    childrenColumnName: string;
    rowHeight: number;
}

function BodyRow<RecordType extends Record<string, unknown>>(
    props: BodyRowProps<RecordType>,
): EmotionJSX.Element {
    const {
        className, style, record, index, rowKey,
        expandedKeys, onRow, indent = 0,
        childrenColumnName, rowHeight,
    } = props;
    const { prefixCls, fixedInfoList } = useTableContext();
    const {
        flattenColumns, onTriggerExpand: onExpand,
        rowClassName, indentSize, expandIcon, expandIconColumnIndex,
    } = useBodyContext();

    const expanded = expandedKeys?.has(props.recordKey);

    const hasNestChildren = childrenColumnName !== '' && record && record[childrenColumnName] !== undefined;

    const rowProps: React.HTMLAttributes<HTMLElement> = useMemo(
        () => onRow?.(record, index) ?? {},
        [onRow, record, index]
    );

    const computedRowClassName = useMemo(() => {
        if (typeof rowClassName === 'string') {
            return rowClassName;
        }
        if (typeof rowClassName === 'function') {
            return rowClassName(record, index, indent);
        }
        return '';
    }, [rowClassName, record, indent, index]);

    const columnsKey = getColumnsKey(flattenColumns);
    return (
        <tr
            {...rowProps}
            data-row-key={rowKey}
            className={classNames(
                className,
                `${prefixCls}-row`,
                `${prefixCls}-row-level-${indent}`,
                computedRowClassName,
                rowProps?.className,
            )}
            style={{
                ...style,
                ...(rowProps ? rowProps.style : null),
                height: rowHeight,
            }}
        >
            {flattenColumns.map((column, colIndex) => {
                const { render, dataIndex } = column;
                const key = columnsKey[colIndex];
                const fixedInfo = fixedInfoList[colIndex];

                let expandNode: React.ReactNode;
                if (colIndex === (expandIconColumnIndex || 0)) {
                    expandNode = (
                        <>
                            <span
                                style={{ paddingLeft: `${indentSize * indent}px` }}
                                className={`${prefixCls}-row-indent indent-level-${indent}`}
                            />
                            {expandIcon?.({
                                prefixCls,
                                expanded,
                                expandable: hasNestChildren,
                                record,
                                onExpand,
                            })}
                        </>
                    );
                }

                const additionalProps = useMemo(() => column.onCell?.(record, index), [column, record, index]);

                return (<Cell
                    ellipsis={column.ellipsis}
                    align={column.align}
                    prefixCls={prefixCls}
                    key={key}
                    record={record}
                    component={'td'}
                    index={index}
                    dataIndex={dataIndex}
                    render={render}
                    width={column.width}
                    {...fixedInfo}
                    extraNode={expandNode}
                    additionalProps={additionalProps}
                />);
            })}
        </tr>
    );
}

export interface BodyProps<RecordType extends Record<string, unknown>> {
    viewModel: Array<TreeViewModel<RecordType>>;
    getRowKey: GetRowKey<RecordType>;
    expandedKeys: Set<Key>;
    onRow?: GetComponentProps<RecordType>;
    childrenColumnName: string;
    rowHeight: number;
}

export function Body<RecordType extends Record<string, unknown>>({
    viewModel,
    getRowKey,
    expandedKeys,
    onRow,
    childrenColumnName,
    rowHeight,
}: BodyProps<RecordType>): EmotionJSX.Element {
    const { onColumnResize } = useResizeContext();
    const { prefixCls } = useTableContext();
    const { flattenColumns } = useBodyContext();
  
    // =================== Performance ====================
    const perfRef = React.useRef<PerfRecord>({
        renderWithProps: false,
    });
  
    // ====================== Render ======================
    const bodyNode = React.useMemo(() => {
        const columnsKey = getColumnsKey(flattenColumns);
    
        return (
            <tbody className={`${prefixCls}-tbody`}>
                {/* Measure body column width with additional hidden col */}
                <MeasureRow
                    prefixCls={prefixCls}
                    columnKeys={columnsKey}
                    onColumnResize={onColumnResize}
                />
                {
                    viewModel.map((item, idx) => {
                        const { data, depth } = item;
                        const key = getRowKey(data, idx);
                
                        return (
                            <BodyRow
                                key={key}
                                rowKey={key}
                                record={data}
                                recordKey={key}
                                index={idx}
                                expandedKeys={expandedKeys}
                                onRow={onRow}
                                getRowKey={getRowKey}
                                rowHeight={rowHeight}
                                childrenColumnName={childrenColumnName}
                                indent={depth}
                            />
                        );
                    })
                }
            </tbody>
        );
    }, [
        viewModel,
        prefixCls,
        onRow,
        expandedKeys,
        getRowKey,
        flattenColumns,
        childrenColumnName,
        onColumnResize,
    ]);
  
    return (
        <PerfContext.Provider value={perfRef.current}>
            {bodyNode}
        </PerfContext.Provider>
    );
}
