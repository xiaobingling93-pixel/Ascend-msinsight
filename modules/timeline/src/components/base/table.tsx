/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import styled from '@emotion/styled';
import React from 'react';
import AntdTable, { type TableProps } from './rc-table';
import type { TableHandle } from './rc-table/interface';

const Support = React.forwardRef(
    (props: TableProps<any>, ref: React.Ref<TableHandle>) => {
        return <AntdTable { ...props } ref={ref} />;
    },
);
Support.displayName = 'table';
export const Table = styled(Support)`
    .insight-table .insight-table-thead th.insight-table-cell {
        background-color: ${(props): string => props.theme.contentBackgroundColor};
        border-bottom-color: unset;
        color: unset;
        &:not(:last-child):not(:nth-last-child(2)):not(.ant-table-selection-column):not(.ant-table-row-expand-icon-cell):not([colspan])::before {
            background-color: ${(p): string => p.theme.borderColorLight};
            position: absolute;
            top: 50%;
            right: 0;
            width: 1px;
            height: 1.6em;
            transform: translateY(-50%);
            transition: background-color .3s;
            content: "";
        }
        &.insight-table-column-has-sorters{
            cursor: pointer;
        }
    }

    color: ${(props): string => props.theme.tableTextColor};

    .insight-table-cell {
        white-space: nowrap;
        overflow: hidden;
        text-overflow: ellipsis;
    }
    /* highlight selected row */
    tr.insight-table-row-selected {
        background-color: ${(props): string => props.theme.selectBackgroundColor};
    }
    /* highlight hovered row */
    tr.insight-table-row:hover {
        background-color: ${(props): string => props.theme.selectBackgroundColor};
    }
    .insight-table-tbody > tr > td {
        border-bottom: solid 1px ${(props): string => props.theme.tableBorderColor};
    }
    .insight-table-header {
        border-bottom: solid 1px ${(props): string => props.theme.tableBorderColor};
        .insight-table-thead .insight-table-cell {
            border-bottom: none;
        }
    }
    /* hide header row if no data */
    .insight-table-empty .insight-table-header {
        display: none;
    }
    .ant-empty-description {
        color: gray;
    }
    tr.insight-table-placeholder > td.insight-table-cell {
        border-bottom: unset;
    }
    /* don't highlight table on hover */
    .insight-table-tbody > tr.insight-table-placeholder:hover > td {
        background-color: unset;
    }
    .insight-table-cell-fix-right, .insight-table-cell-fix-left {
        background-color: ${(props): string => props.theme.contentBackgroundColor};
        z-index: 5;
    }
    .insight-table .insight-table-thead > tr > th,
    .insight-table .insight-table-tbody > tr > td {
        padding: 0 8px;
    }
    .insight-table-thead > tr > th,
    .insight-table-tbody > tr > td {
        position: relative;
        padding: 16px 16px;
        overflow-wrap: break-word;
    }
    .insight-table-thead > tr > th {
        position: relative;
        color: rgba(0, 0, 0, 0.85);
        font-weight: 500;
        text-align: left;
        background: #fafafa;
        border-bottom: 1px solid #f0f0f0;
        transition: background 0.3s ease;
    }
    .insight-table-row-indent {
        float: left;
        height: 1px;
    }
    .insight-table table {
        width: 100%;
        text-align: left;
        border-radius: 2px 2px 0 0;
        border-collapse: separate;
        border-spacing: 0;
    }
    .insight-table-filter-column {
        display: flex;
    }
    .insight-table-filter-trigger {
        position: relative;
        display: flex;
        align-items: center;
        padding: 0 0 0 4px;
        font-size: 12px;
        border-radius: 2px;
        cursor: pointer;
        transition: all 0.3s;
        color: #bfbfbf;
    }
    .insight-table-filter-dropdown {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
        font-size: 14px;
        font-variant: tabular-nums;
        line-height: 1.5715;
        list-style: none;
        font-feature-settings: 'tnum';
        min-width: 120px;
        border-radius: 2px;
    }
`;
