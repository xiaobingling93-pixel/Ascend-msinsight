/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

import { observer } from 'mobx-react';
import React, { useState, useEffect } from 'react';
import { DETAIL_HEADER_HEIGHT_ETC_PX } from './SystemView';
import { ResizeTable } from 'ascend-resize';
import { getDefaultColumData, getPageData, queryTableDataDetails } from './Common';
interface TableData {
    columnAttr: any[];
    tableData: any[];
    totalNum: number;
}
// eslint-disable-next-line camelcase
export const TableDataDetail = observer((props: any) => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const [page, setPage] = useState(defaultPage);
    const defaultSorter = { field: '', order: '' };
    const [sorter, setSorter] = useState(defaultSorter);
    const [column, setColumn] = useState<any[]>([]);
    const [isLoading, setLoading] = useState(false);
    const [condition, setCondition] = useState({ page, sorter });
    useEffect(() => {
        setCondition({ page, sorter });
    }, [sorter, page.current, page.pageSize]);
    useEffect(() => {
        const param = {
            rankId: props.rankId,
            pageSize: condition.page.pageSize,
            currentPage: condition.page.current,
            selectKey: props.selectKey as number,
            order: condition.sorter.order,
            orderBy: condition.sorter.field,
        };
        setLoading(true);
        queryTableDataDetails(param).then((res) => {
            const datas = res as TableData;
            const cols = datas.columnAttr.map((item) => {
                return { title: item.key as string, dataIndex: item.key, ...getDefaultColumData(item.key) };
            });
            setColumn(cols);
            setDataSource(datas.tableData);
            setPage((prevPage: any) => ({ ...prevPage, total: res.totalNum }));
            setLoading(false);
        });
    }, [condition.page.current, condition.page.pageSize,
        condition.sorter.field, condition.sorter.order, props.selectKey, props.rankId]);
    return (
        <div style={{ height: '100%' }}>
            <ResizeTable
                onChange={(pagination: unknown, filters: unknown, newsorter: unknown, extra: {action: string}): void => {
                    if (extra.action === 'sort') {
                        setSorter(newsorter as typeof sorter);
                    }
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
// eslint-disable-next-line camelcase
export const TableDataView = observer((props: any) => {
    return (
        <TableDataDetail {...props} />
    );
});
