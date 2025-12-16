/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import type { TableProps } from 'antd/es/table';
import type { SorterResult } from 'antd/lib/table/interface';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import i18n from '@insight/lib/i18n';
import type { CurveTable, DataDetail, RenderExpandRecord, TableColumn } from '../entity/curve';
import { ResizeTable } from '@insight/lib/resize';
import type { MenuProps, TableColumnsType } from 'antd';
import { Dropdown } from 'antd';
import connector from '../connection';

interface IProps {
    tableData: CurveTable;
    onRowSelected?: (record?: DataDetail, rowIndex?: number) => void;
    current: number;
    pageSize: number;
    onPageChange: (newCurrent: number, newPageSize: number) => void;
    onOrderChange: (order: string | undefined) => void;
    onOrderByChange: (orderBy: string) => void;
    total: number;
    rankId: string;
    groupName: string;
}

const getTableColumns = function (
    columns: TableColumn[],
    t: TFunction,
): TableColumnsType<RenderExpandRecord> {
    return columns.map((col: TableColumn) =>
        ({
            dataIndex: col.key,
            key: col.key,
            title: t(col.name, { defaultValue: col.name, keyPrefix: 'tableHead' }),
            sorter: true,
            ellipsis: true,
            showSorterTooltip: t(col.name, { keyPrefix: 'tableHeadTooltip', defaultValue: '' }) === ''
                ? true
                : { title: t(col.name, { keyPrefix: 'tableHeadTooltip' }) },
            render: (data, _record) => data,
        }),
    );
};
let selectedRecord: DataDetail | undefined;
function redirectToTimeline(time: string, rankId: string, groupName: string, duration: string): void {
    const last = groupName.lastIndexOf('_');
    if (last === -1) return;
    const secondLast = groupName.lastIndexOf('_', last - 1);
    if (secondLast === -1) return; // 没有第二个下划线
    let name = groupName.substring(0, secondLast);
    let startTime = parseInt(time);
    if (name.endsWith('_bubble')) {
        const last = name.lastIndexOf('_');
        name = name.substring(0, last);
        startTime += parseInt(duration);
    }
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'timeline',
            toModuleEvent: 'findBlock',
            params: {
                startTime,
                rankId,
                name,
            },
        },
    });
}
export const AntTableChart: React.FC<IProps> = (props) => {
    // 开发环境防止antd4 table组件报ResizeObserver loop错误，但会在没有数据时也显示有1条，生产环境不会报错也会正常显示
    const defaultDataSource = (process.env.NODE_ENV === 'development' ? [{}] : []) as DataDetail[];
    const { t } = useTranslation('statistic');
    const {
        tableData, onRowSelected, current, pageSize,
        onPageChange, total, onOrderChange, onOrderByChange, rankId, groupName,
    } = props;
    const [shouldBlockMouseLeave, setShouldBlockMouseLeave] = useState<boolean>(false);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const [open, setOpen] = useState<boolean>(false);
    const columns = useMemo(
        () => getTableColumns(tableData.columns, t),
        [tableData.columns, t],
    );

    const useMenuItems = (): MenuProps['items'] => {
        if (selectedRecord === undefined || !open) {
            return [];
        }
        return [
            {
                label: t('Find in Timeline'),
                key: 'findInTimeline',
                onClick: (): void => {
                    if (selectedRecord !== undefined) {
                        redirectToTimeline(selectedRecord.startTime as string, rankId, groupName, selectedRecord.duration as string);
                    }
                },
            },
        ];
    };
    const items = useMenuItems();

    // key is used to reset the Table state (page and sort) if the columns change
    const key = useMemo(() => `${Math.random()}`, [tableData.columns]);

    const onChange = (newCurrent: number, size: number): void => {
        onPageChange(newCurrent, size);
    };

    const onTableChange: TableProps<DataDetail>['onChange'] = (pagination, filter,
        sorter: SorterResult<DataDetail> | Array<SorterResult<DataDetail>>) => {
        if ((sorter as SorterResult<DataDetail>).order) {
            const orderByCol = `${(sorter as SorterResult<DataDetail>).field}`;
            onOrderChange((sorter as SorterResult<DataDetail>).order as string);
            onOrderByChange(orderByCol);
        } else {
            onOrderChange(undefined);
        }
    };

    const onRow = (record: DataDetail, rowIndex?: number): React.HTMLAttributes<any> => {
        return {
            onMouseEnter: (event: any): void => {
                onRowSelected?.(record, rowIndex);
            },
            onMouseLeave: (event: any): void => {
                if (shouldBlockMouseLeave) {
                    setShouldBlockMouseLeave(false);
                    return;
                }
                setOpen(false);
                onRowSelected?.(undefined, undefined);
            },
            onContextMenu: (event: any): void => {
                event.preventDefault(); // 阻止默认的右键菜单
                selectedRecord = record;
                if (selectedRecord.startTime !== undefined && selectedRecord.duration !== undefined) {
                    setOpen(true);
                    setShouldBlockMouseLeave(true);
                }
            },
        };
    };

    useEffect(() => {
        setExpandedKeys([]);
    }, [JSON.stringify(tableData), current, pageSize]);
    return (
        <Dropdown menu={{ items }} trigger={['contextMenu']}>
            <div>
                <ResizeTable
                    columns={columns as TableColumnsType<DataDetail>}
                    dataSource={tableData.rows.length === 0 ? defaultDataSource : tableData.rows.map((item, index) => ({ ...item, key: `${item.name}_${index}` }))}
                    onChange={onTableChange}
                    scroll={{
                        x: 150 * columns.length,
                    }}
                    pagination={{
                        current,
                        pageSize,
                        pageSizeOptions: ['10', '20', '30', '50', '100'],
                        onChange,
                        total,
                        showTotal: (totalNum: number): string => i18n.t('PaginationTotal', { total: totalNum }),
                        showQuickJumper: true,
                    }}
                    rowClassName="curve-ant-table-row"
                    key={key}
                    onRow={onRow}
                    expandable={{
                        expandIcon: () => <></>,
                        expandedRowKeys,
                    }}
                />
            </div>
        </Dropdown>
    );
};
