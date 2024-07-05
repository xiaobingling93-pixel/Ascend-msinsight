import classNames from 'classnames';
import React, { ReactNode } from 'react';
import { PerfContext } from './contexts';
import { AlignType, CellEllipsisType, CellType, ColumnType, DataIndex, DefaultRecordType, RenderedCell } from './types';
import { getPathValue, isValidValue } from './utils/valueUtil';

function isRenderedCell<RecordType>(
    data: React.ReactNode | RenderedCell<RecordType>,
): data is RenderedCell<RecordType> {
    return typeof data === 'object' && !Array.isArray(data) && !React.isValidElement(data);
}

interface CellProps<RecordType extends DefaultRecordType> {
    prefixCls?: string;
    record?: RecordType;
    index?: number;
    dataIndex?: DataIndex;
    render?: ColumnType<RecordType>['render'];
    component: 'td' | 'th';
    children?: React.ReactNode;
    colSpan?: number;
    ellipsis?: CellEllipsisType;
    align?: AlignType;
    width?: number | string;

    // Fixed
    fixLeft?: number | false;
    fixRight?: number | false;
    lastFixLeft?: boolean;
    firstFixRight?: boolean;

    // ====================== Private Props ======================
    extraNode?: React.ReactNode;
    additionalProps?: React.TdHTMLAttributes<HTMLTableCellElement>;

    rowType?: 'header' | 'body';
    isSticky?: boolean;
}

const extractTitle = ({
    ellipsis,
    rowType,
    children,
}: Pick<CellProps<any>, 'ellipsis' | 'rowType' | 'children'>): string => {
    const ellipsisConfig = ellipsis === true ? { showTitle: true } : ellipsis;
    if (ellipsisConfig && (ellipsisConfig.showTitle || rowType === 'header')) {
        if (typeof children === 'string' || typeof children === 'number') {
            return children.toString();
        } else if (React.isValidElement(children) && typeof children.props.children === 'string') {
            return children.props.children;
        }
    }
    return '';
};
  
function Cell<RecordType extends DefaultRecordType>({
    prefixCls,
    record,
    index,
    dataIndex,
    render,
    children,
    component: Component,
    colSpan,
    fixLeft,
    fixRight,
    lastFixLeft,
    firstFixRight,
    extraNode,
    additionalProps = {},
    ellipsis,
    align,
    rowType,
    isSticky,
    width,
}: CellProps<RecordType>, ref: React.Ref<any>): React.ReactElement | null {
    const cellPrefixCls = `${prefixCls}-cell`;

    const perfRecord = React.useContext(PerfContext);

    const [childNode, legacyCellProps] = React.useMemo<
        [React.ReactNode, CellType<RecordType>] | [React.ReactNode]
    >(() => {
        if (isValidValue(children)) {
            return [children];
        }

        const value = getPathValue<Record<string, unknown> | React.ReactNode, RecordType>(record, dataIndex);

        let returnChildNode = value as ReactNode;
        let cellProps: CellType<RecordType> | undefined = undefined;

        if (render) {
            const renderData = render(value, record!, index!);
            if (renderData && isRenderedCell(renderData)) {
                returnChildNode = renderData.children;
                cellProps = renderData.props;
                perfRecord.renderWithProps = true;
            } else {
                returnChildNode = renderData;
            }
        }
        return cellProps ? [returnChildNode, cellProps] : [returnChildNode];
    }, [
        // Always re-render if `renderWithProps`
        perfRecord.renderWithProps ? Math.random() : 0,
        children,
        dataIndex,
        perfRecord,
        record,
        render,
    ]);

    let mergedChildNode = childNode;
    if (isRenderedCell(mergedChildNode)) {
        mergedChildNode = null;
    }
    if (ellipsis && (lastFixLeft || firstFixRight)) {
        mergedChildNode = <span className={`${cellPrefixCls}-content`}>{mergedChildNode}</span>;
    }
    const {
        colSpan: cellColSpan,
        style: cellStyle,
        className: cellClassName,
        ...restCellProps
    } = legacyCellProps || {};
    const mergedColSpan = (cellColSpan !== undefined ? cellColSpan : colSpan) ?? 1;

    if (mergedColSpan === 0) {
        return null;
    }

    const fixedStyle: React.CSSProperties = {};
    const isFixLeft = typeof fixLeft === 'number';
    const isFixRight = typeof fixRight === 'number';

    if (isFixLeft) {
        fixedStyle.position = 'sticky';
        fixedStyle.left = fixLeft;
    }
    if (isFixRight) {
        fixedStyle.position = 'sticky';
        fixedStyle.right = fixRight;
    }

    // remove?
    const onMouseEnter: React.MouseEventHandler<HTMLTableCellElement> = event => {
        additionalProps?.onMouseEnter?.(event);
    };
    const onMouseLeave: React.MouseEventHandler<HTMLTableCellElement> = event => {
        additionalProps?.onMouseLeave?.(event);
    };

    const title = extractTitle({ rowType, ellipsis, children: childNode });

    const props: React.TdHTMLAttributes<HTMLTableCellElement> & { ref: React.Ref<any> } = {
        title,
        ...restCellProps,
        ...additionalProps,
        colSpan: mergedColSpan === 1 ? undefined : mergedColSpan,
        className: classNames(
            cellPrefixCls,
            {
                [`${cellPrefixCls}-fix-left`]: isFixLeft,
                [`${cellPrefixCls}-fix-left-last`]: lastFixLeft,
                [`${cellPrefixCls}-fix-right`]: isFixRight,
                [`${cellPrefixCls}-fix-right-first`]: firstFixRight,
                [`${cellPrefixCls}-ellipsis`]: ellipsis,
                [`${cellPrefixCls}-with-append`]: extraNode,
                [`${cellPrefixCls}-fix-sticky`]: (isFixLeft || isFixRight) && isSticky,
            },
            additionalProps.className,
            cellClassName,
        ),
        style: { ...additionalProps.style, textAlign: align, ...fixedStyle, ...cellStyle, width, maxWidth: width },
        onMouseEnter,
        onMouseLeave,
        ref: ref,
    };

    return (
        <Component {...props} >
            {extraNode}
            {mergedChildNode}
        </Component>
    );
}

export const RefCell = React.forwardRef<any, CellProps<any>>(Cell);
RefCell.displayName = 'Cell';
