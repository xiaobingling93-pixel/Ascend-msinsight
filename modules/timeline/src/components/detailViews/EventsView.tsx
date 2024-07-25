/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { eventViewData, getDefaultColumData, getPageData } from './Common';
import ResizeTable from 'lib/ResizeTable';
import { getDetailTimeDisplay, ThreadUnit } from '../../insight/units/AscendUnit';
import type { ThreadMetaData } from '../../entity/data';
import { Button } from 'lib/components';
import type { InsightUnit } from '../../entity/insight';
import { colorPalette, getTimeOffset } from '../../insight/units/utils';
import { calculateDomainRange } from '../CategorySearch';
import { hashToNumber } from '../../utils/colorUtils';
import { runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import i18n from 'lib/i18n';
import { DETAIL_HEADER_HEIGHT_ETC_PX } from './SystemView';

export interface EventTableData {
    eventDetails: EventDetails[];
    columnList: EventTableColumn[];
    count: number;
}

export interface EventDetails {
    start: number;
    name: string;
    duration: number;
    startTime: string;
    tid?: string;
    pid?: string;
    threadName?: string;
    rankId?: string;
    analysisType?: string;
}

export interface EventTableColumn {
    name: string;
    type: string;
    key: string;
}

interface UpdateDatas {
    pages: { current: number; pageSize: number; total: number };
    sorters: { field: string; order: string };
    props: any;
    setLoading: React.Dispatch<React.SetStateAction<boolean>>;
    setDataSource: React.Dispatch<React.SetStateAction<any[]>>;
    setPage: React.Dispatch<React.SetStateAction<any>>;
    setEventColum: React.Dispatch<React.SetStateAction<string[]>>;
}

const getColumns = (tableColumns: EventTableColumn[]): any => {
    const result = [];
    for (const tableColumn of tableColumns) {
        if (tableColumn.key === 'rankId') {
            result.push({ title: i18n.t(`timeline:tableHead.${tableColumn.name}`), dataIndex: tableColumn.key });
        } else if (tableColumn.key === 'start') {
            result.push({ title: i18n.t(`timeline:tableHead.${tableColumn.name}`), dataIndex: 'startTime', ...getDefaultColumData('startTime') });
        } else {
            result.push({ title: i18n.t(`timeline:tableHead.${tableColumn.name}`), dataIndex: tableColumn.key, ...getDefaultColumData(tableColumn.key) });
        }
    }
    return result;
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: 'duration', order: 'descend' };
export const EventDetail = observer((props: any) => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const [eventColum, setEventColum] = useState<string[]>([]);
    const [rowData, setRowData] = useState<any>({});
    const [allCondition, setAllCondition] = useState({ showEvent: props.session.showEvent, page, sorter });
    const { t } = useTranslation();

    useEffect(() => {
        setAllCondition({ ...allCondition, page, sorter });
    }, [sorter, page.current, page.pageSize]);
    useEffect(() => {
        setAllCondition({ ...allCondition, showEvent: props.session.showEvent, page: defaultPage, sorter: defaultSorter });
    }, [props.session.showEvent]);

    useEffect(() => {
        if (props.session.eventUnits === undefined || props.session.eventUnits.length === 0) {
            setDataSource([]);
            setPage(defaultPage);
            setSorter(defaultSorter);
            setEventColum([]);
            setAllCondition({ ...allCondition, page: defaultPage, sorter: defaultSorter });
            return;
        }
        updateData({ pages: allCondition.page, sorters: allCondition.sorter, props, setLoading, setDataSource, setPage, setEventColum });
    }, [allCondition.showEvent, allCondition.page.current, allCondition.page.pageSize,
        allCondition.sorter.field, allCondition.sorter.order, props.session.doReset, t]);

    useEffect(() => {
        if (rowData.name !== null && rowData.name !== undefined) {
            handleSelected(rowData, props);
        }
    }, [rowData]);

    const eventColumns = generateEventColumns(eventColum, setRowData);

    return (
        <div style={{ height: '100%', padding: '5px 5px 15px 5px' }}>
            <ResizeTable
                onChange={(pagination: unknown, filters: unknown, newsorter: unknown, extra: {action: string}): void => {
                    if (extra.action === 'sort') {
                        setSorter(newsorter as typeof sorter);
                    }
                }}
                pagination={getPageData(page, setPage)} dataSource={dataSource} columns={eventColumns} size="small" loading={isLoading} rowClassName={'click-able'}
                scroll={{ y: props.bottomHeight - DETAIL_HEADER_HEIGHT_ETC_PX }}
            />
        </div>
    );
});

const updateData = async ({
    pages,
    sorters,
    props,
    setLoading,
    setDataSource,
    setPage,
    setEventColum,
}: UpdateDatas): Promise<void> => {
    setLoading(true);
    const res = await searchData(pages, sorters, props).finally(() => setLoading(false));
    const requestData = props.session.eventUnits?.[0]?.metadata as ThreadMetaData;
    const timestampoffset = getTimeOffset(props.session, requestData.cardId as string);
    const data = res.eventDetails.map((item: any) => {
        item.startTime = getDetailTimeDisplay(item.start - timestampoffset);
        return item;
    });
    setEventColum(getColumns(res.columnList));
    setDataSource(data);
    setPage((prevPage: any) => ({ ...pages, total: res.count }));
};

const generateEventColumns = (
    eventColum: string[],
    setRowData: React.Dispatch<React.SetStateAction<any>>,
): any[] => [
    ...eventColum,
    {
        title: i18n.t('timeline:tableHead.Click To Timeline'),
        dataIndex: 'click',
        key: 'click',
        ellipsis: true,
        render: (_: any, record: any) => (
            <Button type="link" onClick={(): void => setRowData(record)}>{i18n.t('timeline:tableHead.Click')}</Button>
        ),
    },
];

const handleSelected = async(rowData: any, props: any): Promise<void> => {
    const rankId = props.session.eventUnits[0].metadata.cardId;
    runInAction(() => {
        props.session.locateUnit = {
            target: (unit: any): boolean => {
                // 能否跳转到某个算子：判断是否是ThreadUnit、卡号能否对上、线程号、进程号是否一致，如果都能对上，说明在timeline上找到了
                return unit instanceof ThreadUnit && unit.metadata.cardId === rankId &&
                    unit.metadata.threadId === rowData.threadId && unit.metadata.processId === rowData.processId;
            },
            onSuccess: (unit: InsightUnit): void => {
                const startTime = rowData.start - getTimeOffset(props.session, (unit.metadata as ThreadMetaData).cardId);
                const [rangeStart, rangeEnd] = calculateDomainRange(props.session, startTime, rowData.duration);
                props.session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                props.session.selectedData = {
                    startTime,
                    name: rowData.name,
                    color: colorPalette[hashToNumber(rowData.name, colorPalette.length)],
                    duration: rowData.duration,
                    depth: rowData.depth,
                    threadId: rowData.threadId,
                    startRecordTime: props.session.startRecordTime,
                    showDetail: false,
                };
            },
        };
    });
};

const searchData = async(pages: any, sorters: {field: string;order: string}, prop: any): Promise<EventTableData> => {
    const requestData = prop.session.eventUnits?.[0]?.metadata as ThreadMetaData;
    return await eventViewData({
        rankId: requestData.cardId as string,
        pageSize: pages.pageSize,
        currentPage: pages.current,
        orderBy: sorters.field === 'startTime' ? 'start' : sorters.field ?? defaultSorter.field,
        order: sorters.order ?? defaultSorter.order,
        pid: requestData.processId as string,
        tid: requestData.threadId as string,
        threadName: requestData.threadName,
        processName: requestData.processName as string,
        metaType: requestData.metaType as string,
    });
};
