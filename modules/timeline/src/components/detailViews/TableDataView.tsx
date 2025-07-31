/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

import { observer } from 'mobx-react';
import React, { useState, useEffect } from 'react';
import { DETAIL_HEADER_HEIGHT_ETC_PX, type SelectContentViewProps } from './SystemView';
import { fetchColumnFilterProps, ResizeTable } from 'ascend-resize';
import { getDefaultColumData, getPageData, queryTableDataDetails } from './Common';
interface TableData {
    columnAttr: any[];
    tableData: any[];
    totalNum: number;
}

interface FilterCondition {
    col: string;
    content: string;
}
export const TableDataDetail = observer((props: SelectContentViewProps & { selectKey: number }) => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const [page, setPage] = useState(defaultPage);
    const defaultSorter = { field: '', order: '' };
    const [sorter, setSorter] = useState(defaultSorter);
    const [column, setColumn] = useState<any[]>([]);
    const [isLoading, setLoading] = useState(false);
    const [condition, setCondition] = useState({ page, sorter });
    const [filters, setFilters] = useState<any>();
    useEffect(() => {
        setCondition({ page, sorter });
    }, [sorter, page.current, page.pageSize]);
    useEffect(() => {
        const filterconditions: FilterCondition[] = [];
        if (filters !== undefined) {
            Object.keys(filters).forEach(key => {
                const filterValue = filters[key];
                if (Array.isArray((filterValue)) && filterValue.length > 0) {
                    const filterCondition = { col: key, content: filterValue[0] };
                    filterconditions.push(filterCondition);
                }
            });
        }
        const param = {
            rankId: props.card.cardId,
            dbPath: props.card.dbPath,
            pageSize: condition.page.pageSize,
            currentPage: condition.page.current,
            selectKey: props.selectKey as number,
            order: condition.sorter.order,
            orderBy: condition.sorter.field,
            filterconditions,
        };
        setLoading(true);
        queryTableDataDetails(param).then((res) => {
            const datas = res as TableData;
            const cols = datas.columnAttr.map((item) => {
                return { title: item.key as string, dataIndex: item.key, ...getDefaultColumData(item.key), ...fetchColumnFilterProps(item.key, item.key) };
            });
            setColumn(cols);
            setDataSource(datas.tableData);
            setPage((prevPage: any) => ({ ...prevPage, total: res.totalNum }));
            setLoading(false);
        });
    }, [condition.page.current, condition.page.pageSize,
        condition.sorter.field, condition.sorter.order, props.selectKey, props.card, filters]);
    return (
        <div style={{ height: '100%' }}>
            <ResizeTable
                onChange={(pagination: unknown, filters: any, newsorter: unknown, extra: {action: string}): void => {
                    if (extra.action === 'sort') {
                        setSorter(newsorter as typeof sorter);
                    }
                    setFilters(filters);
                }}
                rowClassName={(record: any): string => {
                    return 'click-able';
                }}
                pagination={getPageData(page, setPage)} dataSource={dataSource} columns={column} size="small" loading={isLoading}
                scroll={{ y: props.bottomHeight - DETAIL_HEADER_HEIGHT_ETC_PX }}
            />
        </div>
    );
});
export const TableDataView = observer((props: SelectContentViewProps & { selectKey: number }) => {
    return (
        <TableDataDetail {...props} />
    );
});
