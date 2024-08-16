/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import { css } from '@emotion/react';
import styled from '@emotion/styled';
import { Empty } from 'ascend-components';
import classNames from 'classnames';
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import { useMemo } from 'react';
import { ReactComponent as ExpandIcon } from '../../../assets/images/insights/PullDownIcon.svg';
import type { TreeNode } from '../../../entity/common';
import { getAutoKey } from '../../../utils/dataAutoKey';
import type { TableProps } from '../../base/rc-table';
import { INSIGHT_TABLE_PREFIX } from '../../base/rc-table/BaseTable';
import type { TableHandle } from '../../base/rc-table/interface';
import { Table } from '../../base/table';
import type { TableState } from '../types';

// constants
const EMPTY_VIEW_HEIGHT = 22;
const TABLE_ROW_HEIGHT = 32;
const TABLE_HEAD_HEIGHT = TABLE_ROW_HEIGHT - 1;
const BORDER_HEIGHT = 2;

interface TableHeightProps {
    tableHeight: number;
}

const tableStyle = (props: TableHeightProps): ReturnType<typeof css> => css`height: ${props.tableHeight - BORDER_HEIGHT}px`;

const calcEffectiveNum = (num: number, fractionDigits: number = 2): number => {
    return Number(num.toFixed(fractionDigits));
};

export type AutoAdjustedTableProps = { height: number } & TableState & Omit<TableProps<any>, 'scroll'>;

const useEmptyViewMargin = (height: number): number => {
    const [emptyViewMargin, setEmptyViewMargin] = React.useState(calcEffectiveNum((height - EMPTY_VIEW_HEIGHT) / 2));

    React.useEffect(() => {
        const marginHeight = calcEffectiveNum((height - EMPTY_VIEW_HEIGHT) / 2);
        setEmptyViewMargin(marginHeight);
    }, [height]);

    return emptyViewMargin;
};

const StyledTable = styled(Table)`
    width: 100%;
    .insight-table .insight-table-tbody > tr > td:first-of-type,
    .insight-table .insight-table-thead > tr > th:first-of-type {
        padding-left: 24px;
    }
    .insight-table .insight-table-thead > tr > th {
        .active {
            color: ${(p): string => p.theme.buttonFontColor};
        }
    }
    .insight-table .insight-table-thead > tr > .insight-table-column-has-sorters[aria-sort] {
        color: ${(p): string => p.theme.fontColor};
        border-bottom: 2px solid ${(p): string => p.theme.buttonFontColor};
    }
    .insight-table {
        font-size: 12px;
    }
    .insight-table .insight-table-thead > tr > .insight-table-cell-fix-right {
        padding-left: 16px;
        background: ${(props): string => props.theme.contentBackgroundColor};
    }
    .insight-table-tbody > tr.insight-table-row-selected > td {
        background: ${(props): string => props.theme.bgColorLight};
    }
    .insight-table-tbody > tr > td.insight-table-cell-row-hover, tr.insight-table-row:hover > td.insight-table-cell {
        background: ${(props): string => props.theme.bgColorLight};
        transition: none;
    }
    .insight-table .insight-table-thead > tr > th.insight-table-cell {
        background-color: ${(props): string => props.theme.bgColorLight};
        color: ${(props): string => props.theme.textColorSecondary};
        font-weight: 500;
        height: ${TABLE_ROW_HEIGHT}px;
    }
    .insight-table .insight-table-thead > tr > td.insight-table-cell {
        color: ${(props): string => props.theme.textColorTertiary};
    }
    .insight-table .insight-table-row > tr > td.insight-table-cell {
        color: ${(props): string => props.theme.textColorTertiary};
    }
    tr.insight-table-row:hover {
        cursor: pointer;
    }
    .insight-table-header {
        min-height: ${TABLE_ROW_HEIGHT}px;
        span.insight-table-column-sorter.insight-table-column-sorter-full{
            display: none;
        }

        .insight-table-cell svg {
            g g g path:nth-of-type(even) {
                fill: #7A7A7A;
            }
        }

        .insight-table-cell[aria-sort="ascending"] svg {
            g g g path:nth-of-type(odd) {
                fill: #526ECC;
            }

            g g g path:nth-of-type(even) {
                fill: #7A7A7A;
            }
        }

        .insight-table-cell[aria-sort="descending"] svg {
            g g g path:nth-of-type(even) {
                fill: #526ECC;
            }

            g g g path:nth-of-type(odd) {
                fill: #7A7A7A;
            }
        }
    }
    .insight-table-body {
        overflow-y: overlay !important; /* overriding an element-inline style */
        ${tableStyle}
        user-select:text;
        ::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        ::-webkit-scrollbar-track {
            border-radius: 8px;
        }
        ::-webkit-scrollbar-thumb {
            border-radius: 8px;
            background: ${(props): string => props.theme.scrollbarColor};
        }
        ::-webkit-scrollbar-corner {
            background: transparent;
        }
    }
    td > div > .expanded {
        g use {
            fill: ${(props): string => props.theme.fontColor};
        }
    }
    td > div > .unexpanded {
        g use {
            fill: ${(props): string => props.theme.arrowUnexpandedBgColor};
        }
    }
    .ant-empty .ant-empty-description {
        color: ${(props): string => props.theme.fontColor};
    }

    .ant-spin {
        background-color: ${(props): string => props.theme.contentBackgroundColor};
        transition: none;
    }
    .ant-spin-container {
        transition: none;
    }
    .ant-spin-container::after {
        transition: none;
    }
    .filterIcon {
        path {
            fill: ${(props): string => props.theme.filterIconColor};
        }
    }
`;

export const AutoAdjustedTable = React.forwardRef((props: AutoAdjustedTableProps, ref?: React.Ref<TableHandle>): JSX.Element => {
    const { height, isLoading, data, rowKey, rowHeight = TABLE_ROW_HEIGHT } = props;
    const getKey = useMemo(() => rowKey ?? getAutoKey, [rowKey]); // memo
    // when data is empty don't decrease TABLE_HEAD_HEIGHT, or table height would be short
    const tableBodyHeight = data?.length === 0 ? height : height - TABLE_HEAD_HEIGHT;
    const scrollerHeight = calcEffectiveNum(tableBodyHeight);
    const marginTop = useEmptyViewMargin(height);
    const { t } = useTranslation();
    AutoAdjustedTable.displayName = 'AutoAdjustedTable';
    return (<StyledTable
        tableHeight={ scrollerHeight }
        {...props}
        ref={ref}
        rowHeight={rowHeight}
        dataSource={ data }
        showSorterTooltip={false}
        rowKey={ getKey }
        isLoading = { isLoading }
        expandable={{
            ...props.expandable,
            expandIcon: ({ expanded, onExpand, record }): JSX.Element => {
                const node = record as TreeNode<object>;
                if (!node.children) {
                    return <div style={{ float: 'left', width: '10px', height: '16px', margin: '0 4px 0 0' }}/>;
                }
                return <div style={{ float: 'left', height: '16px', margin: '0 4px 0 0' }}
                    className={classNames(`${INSIGHT_TABLE_PREFIX}-row-expand-icon`, {
                        expanded,
                        unexpanded: !expanded,
                    })}
                    onClick={(e: React.MouseEvent<HTMLElement, MouseEvent>): void => onExpand(record, e)}>
                    <ExpandIcon
                        style={{ transform: `rotate(${expanded ? 0 : '-90deg'}) translate(${expanded ? '-2' : '1'}px, ${expanded ? '0' : '-2'}px)`, cursor: 'pointer' }}/>
                </div>;
            },
        }}
        locale={ { emptyText: () => isLoading ? null : <Empty style={{ marginTop, textAlign: 'center' }} description={t('No Data')} imageStyle={{ display: 'none' }}></Empty> } }
        scroll={ { x: data?.length ? 'max-content' : undefined, y: scrollerHeight } } />);
});
