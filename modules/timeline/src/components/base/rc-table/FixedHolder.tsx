import React, { useMemo } from 'react';
import { ColGroup } from './ColGroup';
import { useTableContext } from './contexts';
import { HeaderProps } from './Header';
import { ColumnsType, ColumnType } from './types';

function useColumnWidth(colWidths: readonly number[], columnCount: number): number[] | null {
    return useMemo(() => {
        const cloneColumns: number[] = [];
        for (let i = 0; i < columnCount; i++) {
            const val = colWidths[i];
            if (val === undefined) {
                return null;
            } else {
                cloneColumns[i] = val;
            }
        }
        return cloneColumns;
    }, [colWidths.join('_'), columnCount]);
}

interface FixedHeaderProps<RecordType> extends HeaderProps<RecordType> {
    className: string;
    hasData: boolean;
    maxContentScroll: boolean;
    colWidths: readonly number[];
    columCount: number;
    children: (info: HeaderProps<RecordType>) => React.ReactNode;
}

export const FixedHolder = React.forwardRef<HTMLDivElement, FixedHeaderProps<unknown>>(
    ({
        className, hasData, columns, colWidths, columCount,
        stickyOffsets, maxContentScroll, children,
        ...props
    }, ref) => {
        const { prefixCls, scrollbarSize } = useTableContext();

        const isAllWidthSet = React.useMemo(
            () => columns.every(column => column.width !== undefined && column.width >= 0),
            [columns],
        );

        // Add scrollbar column
        const lastColumn = columns[columns.length - 1];
        const scrollBarColumn: ColumnType<unknown> & { scrollbar: true } = {
            fixed: lastColumn?.fixed,
            scrollbar: true,
            onHeaderCell: () => ({ className: `${prefixCls}-cell-scrollbar` }),
        };

        const columnsWithScrollbar = useMemo<ColumnsType<unknown>>(
            () => (scrollbarSize > 0 ? [ ...columns, scrollBarColumn ] : columns),
            [ scrollbarSize, columns ],
        );

        const headerStickyOffsets = useMemo(() => {
            const { right } = stickyOffsets;
            return {
                ...stickyOffsets,
                right: [ ...right.map(width => width + scrollbarSize), 0 ],
            };
        }, [scrollbarSize, stickyOffsets]);

        const mergedColumnWidth = useColumnWidth(colWidths, columCount);

        return (
            <div
                style={{ overflow: 'hidden' }}
                ref={ref}
                className={className}
            >
                <table
                    style={{
                        tableLayout: 'fixed',
                        visibility: !hasData || mergedColumnWidth ? undefined : 'hidden',
                    }}
                >
                    {/* 解决搜索内容为空时，表头字段位置会改变导致过滤按钮被隐藏的问题，去掉了ColGroup外层的判断条件{(hasData || !maxContentScroll || isAllWidthSet) && (<ColGroup... )} */}
                    <ColGroup
                        colWidths={mergedColumnWidth ? [ ...mergedColumnWidth, scrollbarSize ] : []}
                        columCount={columCount + 1}
                        columns={columnsWithScrollbar}
                        />
                    {children({
                        ...props,
                        stickyOffsets: headerStickyOffsets,
                        columns: columnsWithScrollbar,
                    })}
                </table>
            </div>
        );
    },
);

FixedHolder.displayName = 'FixedHolder';
