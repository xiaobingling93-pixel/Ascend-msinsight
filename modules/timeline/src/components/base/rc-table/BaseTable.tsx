import styled from '@emotion/styled';
import classNames from 'classnames';
import { keys } from 'lodash';
import ResizeObserver from 'rc-resize-observer';
import { getTargetScrollBarSize } from 'rc-util/lib/getScrollBarSize';
import pickAttrs from 'rc-util/lib/pickAttrs';
import React, { ForwardedRef, useCallback, useEffect, useImperativeHandle, useMemo } from 'react';
import { ReactComponent as SortNormal } from '../../../assets/images/sortNormal.svg';
import { Body } from './Body';
import { ColGroup } from './ColGroup';
import { BodyContext, ResizeContext, TableContext } from './contexts';
import { FixedHolder } from './FixedHolder';
import { Header } from './Header';
import { useAsyncState } from './hooks/useAsyncState';
import { useOrderStatisticTree } from './hooks/useOrderStatisticTree';
import { useStickyOffsets } from './hooks/useStickyOffsets';
import { TableHandle } from './interface';
import { ColumnsType, ColumnType, DefaultRecordType, ExpandableConfig, ExpandIconProps, GetComponentProps, GetRowKey, Key, RowClassName, TableLayout, TriggerEventHandler } from './types';
import { getCellFixedInfo } from './utils/fixUtil';
import { getColumnsKey, isValidValue } from './utils/valueUtil';
import type { EmotionJSX } from '@emotion/react/types/jsx-namespace';

const VIRTUAL_TOLERANCE = 4;

export const INSIGHT_TABLE_PREFIX = 'insight-table';

const SortWrapper = styled.div`
    display: flex;
    justify-content: start;
    align-items: center;
    &>div:last-child {
        display: flex;
        align-items: center;
        margin-left: 4px;
    }
`;

export interface TableProps<RecordType = unknown> {
    prefixCls?: string;
    className?: string;
    data?: readonly RecordType[];
    columns: ColumnsType<RecordType>;
    rowKey?: string | GetRowKey<RecordType>;
    tableLayout?: TableLayout;
    rowHeight?: number;
    scroll: { x?: number | true | string; y?: number | string };

    expandable?: ExpandableConfig<RecordType>;
    rowClassName?: string | RowClassName<RecordType>;

    // Customize
    id?: string;
    showHeader?: boolean;
    onRow?: GetComponentProps<RecordType>;
    onHeaderRow?: GetComponentProps<ReadonlyArray<ColumnType<RecordType>>>;
    emptyText?: React.ReactNode | (() => React.ReactNode);

    // =================================== Internal ===================================
    transformColumns?: (columns: ColumnsType<RecordType>) => ColumnsType<RecordType>;
}

const useExpand = <RecordType extends Record<string, unknown>>(
    data: readonly RecordType[],
    getRowKey: GetRowKey<RecordType>,
    expandable: ExpandableConfig<RecordType>,
    childrenColumnName: string,
): [ Set<Key>, TriggerEventHandler<RecordType>, React.Dispatch<React.SetStateAction<readonly React.Key[]>>] => {
    const {
        onExpand,
        onExpandedRowsChange,
        defaultExpandedRowKeys = [],
        expandedRowKeys,
    } = expandable;

    const [ expandedKeys, setExpandedKeys ] = React.useState(defaultExpandedRowKeys);
    const expandedKeySet = React.useMemo(
        () => new Set(expandedRowKeys ?? expandedKeys),
        [ expandedRowKeys, expandedKeys ],
    );
    const onTriggerExpand: TriggerEventHandler<RecordType> = React.useCallback((record: RecordType) => {
        let newExpandedKeys: Key[];
        const key = getRowKey(record, data.indexOf(record));
        const wasExpanded = expandedKeySet.has(key);
        onExpand?.(!wasExpanded, record);
        if (wasExpanded) {
            expandedKeySet.delete(key);
            newExpandedKeys = [...expandedKeySet];
        } else {
            let keysAdded = new Set<Key>([key]);
            let children = record[childrenColumnName];
            while (Array.isArray(children) && children.length === 1) {
                const child = children[0];
                const childKey = getRowKey(child);
                if (expandedKeySet.has(childKey)) {
                    // child is already expanded, continue expanding next level
                    children = child[childrenColumnName];
                    continue;
                }
                onExpand?.(true, child);
                keysAdded.add(childKey);
                children = child[childrenColumnName];
            }
            newExpandedKeys = [ ...expandedKeySet, ...keysAdded ];
        }
        setExpandedKeys(newExpandedKeys);
        onExpandedRowsChange?.(newExpandedKeys);
    }, [ getRowKey, expandedKeySet, data, onExpand, onExpandedRowsChange ]);

    return [ expandedKeySet, onTriggerExpand, setExpandedKeys ];
}

function renderExpandIcon<RecordType>({
    prefixCls,
    record,
    onExpand,
    expanded,
    expandable,
}: ExpandIconProps<RecordType>): EmotionJSX.Element {
    const iconClassName = `${prefixCls}-row-expand-icon`;

    if (!expandable) {
        return <span className={classNames(iconClassName, `${prefixCls}-row-spaced`)} />;
    }

    const onClick: React.MouseEventHandler<HTMLElement> = useCallback(event => {
        onExpand?.(record, event);
        event.stopPropagation();
    }, [ onExpand, record ]);

    return (<span
        className={classNames(iconClassName, {
            [`${prefixCls}-row-expanded`]: expanded,
            [`${prefixCls}-row-collapsed`]: !expanded,
        })}
        onClick={onClick}
    />);
}

/* eslint-disable max-lines-per-function */
const ForwardBaseTable =  React.forwardRef(function Table<RecordType extends DefaultRecordType>(props: TableProps<RecordType>, ref: ForwardedRef<TableHandle>): JSX.Element {
    const {
        prefixCls = INSIGHT_TABLE_PREFIX,
        className,
        rowClassName,
        data = [],
        rowKey,
        scroll,
        tableLayout,
        rowHeight = 32,
        id,
        showHeader,
        emptyText,
        onRow,
        onHeaderRow,
        transformColumns,
        expandable = {},
    } = props;

    const hasData = data.length !== 0;

    const getRowKey = React.useMemo<GetRowKey<RecordType>>(() => {
        if (typeof rowKey === 'function') {
            return rowKey;
        }
        return (record: RecordType) => {
            const key = rowKey !== undefined ? record[rowKey] as Key : undefined;
            if (key === undefined) {
                throw new Error('row key not set');
            }
            return key;
        };
    }, [rowKey]);

    // ====================== Expand ======================
    if (expandable.showExpandColumn === false) {
        expandable.expandIconColumnIndex = -1;
    }

    const {
        expandIcon = renderExpandIcon,
        expandIconColumnIndex,
        childrenColumnName = 'children',
        indentSize = 15,
    } = expandable;

    const [ expandedKeySet, onTriggerExpand, setExpandedKeys ] = useExpand(data, getRowKey, expandable, childrenColumnName);

    const [ componentWidth, setComponentWidth ] = React.useState(0);

    const flattenColumns = React.useMemo(() => {
        let finalColumns = transformColumns?.(props.columns) ?? props.columns;
        if (finalColumns.length === 0) {
            finalColumns = [{ render: () => null }];
        }
        return finalColumns;
    }, [transformColumns]);

    // ====================== Scroll ======================
    const wholeTableRef = React.useRef<HTMLDivElement>(null);
    const headerTableRef = React.useRef<HTMLDivElement>(null);
    const bodyTableRef = React.useRef<HTMLDivElement>(null);
    const [ scrollTop, setScrollTop ] = React.useState(0);
    const [ colsWidths, updateColsWidths ] = useAsyncState(new Map<React.Key, number>());

    // Convert map to number width
    const colsKeys = getColumnsKey(flattenColumns);
    const pureColWidths = colsKeys.map(columnKey => colsWidths.get(columnKey) as number);
    const colWidths = React.useMemo(() => pureColWidths, [pureColWidths.join('_')]);
    const stickyOffsets = useStickyOffsets(colWidths, flattenColumns.length);
    const horizonScroll = isValidValue(scroll.x) || Boolean(expandable.fixed);
    const fixColumn = horizonScroll && flattenColumns.some(({ fixed }) => fixed);

    // Scroll
    let scrollXStyle: React.CSSProperties = {};
    let scrollTableStyle: React.CSSProperties = {};

    const scrollYStyle: React.CSSProperties = {
        overflowY: 'scroll',
        maxHeight: scroll.y,
    };

    if (horizonScroll) {
        scrollXStyle = { overflowX: 'auto' };
        scrollTableStyle = {
            width: scroll?.x === true ? 'auto' : scroll?.x,
            minWidth: '100%',
        };
    }

    const onColumnResize = React.useCallback((columnKey: React.Key, width: number) => {
        /*
            增加了width===0的判断，原因是在mac上width会传0过滤导致useAsyncState里的immutableStateRef被更新，进而出现所有字段width都为0的情况
            在windows上无此问题，经测试，windows添加width===0的判断也不会造成其他影响
        */
        if (!wholeTableRef.current || width === 0) {
            return;
        }
        updateColsWidths(widths => {
            if (widths.get(columnKey) === width) {
                return widths;
            }
            const newWidths = new Map(widths);
            newWidths.set(columnKey, width);
            return newWidths;
        });
    }, []);

    const onScroll = ({ currentTarget }: React.UIEvent<HTMLDivElement>): void => {
        setScrollTop(currentTarget.scrollTop);
        if (headerTableRef.current && bodyTableRef.current) {
            headerTableRef.current.scrollLeft = bodyTableRef.current.scrollLeft;
        }
    };

    const onWholeTableResize = useCallback(({ width }: { width: number }) => {
        if (width !== componentWidth) {
            setComponentWidth(wholeTableRef.current ? wholeTableRef.current.offsetWidth : width);
        }
    }, [componentWidth]);

    // ===================== Effects ======================
    const [ scrollbarSize, setScrollbarSize ] = React.useState(0);

    React.useEffect(() => {
        bodyTableRef.current && setScrollbarSize(getTargetScrollBarSize(bodyTableRef.current).width);
    }, []);

    // ====================== Render ======================
    // Table layout
    const mergedTableLayout = React.useMemo<TableLayout>(() => {
        if (tableLayout) {
            return tableLayout;
        }
        if (fixColumn) {
            return scroll?.x === 'max-content' ? 'auto' : 'fixed';
        }
        if (flattenColumns.some(({ ellipsis }) => ellipsis)) {
            return 'fixed';
        }
        return 'auto';
    }, [ fixColumn, flattenColumns, tableLayout ]);

    // Empty
    const emptyNode: React.ReactNode = React.useMemo(() => {
        if (typeof emptyText === 'function') {
            return emptyText();
        }
        return emptyText;
    }, [emptyText]);

    // Body
    const viewModel = useOrderStatisticTree(data, childrenColumnName, expandedKeySet, getRowKey);
    const totalHeight = useMemo(() => {
        return viewModel.getTotalHeight(rowHeight);
    }, [ viewModel, rowHeight ]);

    const viewportHeight = scroll?.y as number;

    // adjust scroll position when data changes
    useEffect(() => {
        if (scrollTop + viewportHeight > totalHeight) {
            setScrollTop(Math.max(0, totalHeight - viewportHeight));
        }
    });

    const visibleData = useMemo(() => {
        return viewModel.getVisibleData(scrollTop, viewportHeight, rowHeight, VIRTUAL_TOLERANCE);
    }, [ viewModel, scrollTop, viewportHeight, rowHeight ]);

    // Get ref form parent component
    useImperativeHandle(ref, () => ({
        scrollTo: (node: unknown): void => {
            if (bodyTableRef.current !== null) {
                const scrollTop = viewModel.findNodeIndex(node as RecordType) * rowHeight;
                setScrollTop(scrollTop);
                bodyTableRef.current.scrollTo({top: scrollTop});
            }
        },
        appendExpandedKeys: (res): void => {
            const keysToAdd = res.filter((it) => !expandedKeySet.has(it));
            if (keysToAdd.length > 0) {
                setExpandedKeys([...expandedKeySet, ...keysToAdd]);
            }
        },
        selectFirstRoot: (): void => {
            if (bodyTableRef.current === null) {
                return;
            }
            const rows = Array.from(bodyTableRef.current.querySelectorAll('tr').values());
            // skip the measure row, for more information see Body element
            const firstContentRow = rows.find(it => it.className.indexOf('measure') < 0);
            if ((rows.length > 0) && (rows[rows.length - 1] === firstContentRow)) { // single root node, expand it
                // expand icon class name is defined as `${prefixCls}-row-expand-icon` by convention
                (firstContentRow?.querySelector(`.${prefixCls}-row-expand-icon`) as HTMLElement | null)?.click();
            } else { // multiple root nodes, just select the first
                firstContentRow?.click();
            }
        }
    }), [onTriggerExpand]);

    const bodyTable = (
        <div style={{
            ...scrollTableStyle,
            height: totalHeight,
        }}>
            <table style={{
                ...scrollTableStyle,
                tableLayout: mergedTableLayout,
                position: 'relative',
                top: rowHeight === 0 ? 0 : Math.max(0, Math.floor(scrollTop / rowHeight) - VIRTUAL_TOLERANCE) * rowHeight,
            }}>
                <ColGroup colWidths={flattenColumns.map(({ width }) => width)} columns={flattenColumns} />
                <Body<RecordType>
                    viewModel={visibleData}
                    expandedKeys={expandedKeySet}
                    getRowKey={getRowKey}
                    onRow={onRow}
                    childrenColumnName={childrenColumnName}
                    rowHeight={rowHeight}
                />
            </table>
        </div>
    );

    const bodyContent = (
        <div
            style={{
                ...scrollXStyle,
                ...scrollYStyle,
            }}
            onScroll={onScroll}
            ref={bodyTableRef}
            className={classNames(`${prefixCls}-body`)}
        >
            { hasData ? bodyTable : emptyNode }
        </div>
    );

    // Fixed holder share the props
    const fixedHolderProps = {
        hasData,
        maxContentScroll: horizonScroll && scroll.x === 'max-content',
        colWidths,
        columCount: flattenColumns.length,
        stickyOffsets,
        onHeaderRow,
        scroll,
    };

    const cols = (flattenColumns as ColumnsType<unknown>).map(item => {
        if (keys(item).includes('sorter')) {
            return {
                ...item,
                title: <SortWrapper>
                    { item.title }<div><SortNormal /></div>
                </SortWrapper>,
            };
        } else {
            return item;
        }})

    const groupTableNode = (
        <>
            {showHeader !== false && (
                <FixedHolder
                    {...fixedHolderProps}
                    columns={cols}
                    onHeaderRow={fixedHolderProps.onHeaderRow as GetComponentProps<ReadonlyArray<ColumnType<unknown>>>}
                    className={`${prefixCls}-header`}
                    ref={headerTableRef}
                >
                    {(headerProps): React.ReactElement => (<Header {...headerProps} />)}
                </FixedHolder>
            )}
            {bodyContent}
        </>
    );

    const ariaProps = pickAttrs(props, { aria: true, data: true });

    let wholeTable = (
        <div
            className={classNames(prefixCls, className, {
                [`${prefixCls}-scroll-horizontal`]: horizonScroll,
                [`${prefixCls}-has-fix-left`]: flattenColumns[0].fixed,
                [`${prefixCls}-has-fix-right`]: flattenColumns[flattenColumns.length - 1]?.fixed === 'right',
            })}
            id={id}
            ref={wholeTableRef}
            {...ariaProps}
        >
            <div className={`${prefixCls}-container`}>{groupTableNode}</div>
        </div>
    );

    if (horizonScroll) {
        wholeTable = <ResizeObserver onResize={onWholeTableResize}>{wholeTable}</ResizeObserver>;
    }

    const TableContextValue = React.useMemo(
        () => ({
            prefixCls,
            scrollbarSize,
            fixedInfoList: flattenColumns.map((_, colIndex) =>
                getCellFixedInfo(colIndex, colIndex, flattenColumns, stickyOffsets),
            ),
        }), [
            prefixCls,
            scrollbarSize,
            flattenColumns,
            stickyOffsets,
        ],
    );

    const BodyContextValue = React.useMemo(
        () => ({
            tableLayout: mergedTableLayout,
            flattenColumns,
            rowClassName,
            expandIcon,
            onTriggerExpand,
            expandIconColumnIndex,
            indentSize,
        }),
        [
            mergedTableLayout,
            flattenColumns,
            rowClassName,
            expandIcon,
            onTriggerExpand,
            expandIconColumnIndex,
            indentSize,
        ],
    );

    const ResizeContextValue = React.useMemo(() => ({ onColumnResize }), [onColumnResize]);

    return (
        <TableContext.Provider value={TableContextValue}>
            <BodyContext.Provider value={BodyContextValue}>
                <ResizeContext.Provider value={ResizeContextValue}>{wholeTable}</ResizeContext.Provider>
            </BodyContext.Provider>
        </TableContext.Provider>
    );
});
/* eslint-enable max-lines-per-function */

ForwardBaseTable.defaultProps = {
    rowKey: 'key',
    prefixCls: INSIGHT_TABLE_PREFIX,
    emptyText: (): React.ReactNode => (
        <div style={{
            display: 'flex',
            justifyContent: 'center',
            alignItems: 'center',
            width: '100%',
            height: '100%',
        }}>
            No Data
        </div>
    ),
};

export default ForwardBaseTable as <RecordType extends object = any>(props: React.PropsWithChildren<TableProps<RecordType>> & { ref?: React.Ref<TableHandle> }) => React.ReactElement;
