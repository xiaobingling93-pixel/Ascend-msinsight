/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { StringMap } from '../../utils/interface';
import { notNull, GetPageConfigWhithPageData } from '../Common';
import { queryCommunicationDetail, queryComputeDetail, querySummaryStatistics } from '../../utils/RequestUtils';
import ResizeTable from '../resize/ResizeTable';

const computingStatisticsColumns = [
    {
        title: 'Accelerator Core',
        dataIndex: 'acceleratorCore',
        key: 'acceleratorCore',
    },
    {
        title: 'Accelerator Core Durations(μs)',
        dataIndex: 'duration',
        key: 'duration',
    },

    {
        title: 'Accelerator Core Utilization',
        dataIndex: 'utilization',
        key: 'utilization',
    },
];

const computingDetailColumns = [
    {
        title: 'Name',
        dataIndex: 'name',
        sorter: true,
    },
    {
        title: 'Type',
        dataIndex: 'type',
        sorter: true,
    },
    {
        title: 'Start Time',
        dataIndex: 'startTime',
        sorter: true,
    },
    {
        title: 'Duration(μs)',
        dataIndex: 'duration',
        sorter: true,
    },
    {
        title: 'Wait Time(μs)',
        dataIndex: 'waitTime',
        sorter: true,
    },
    {
        title: 'Block Dim',
        dataIndex: 'blockDim',
        sorter: true,
    },
    {
        title: 'Input Shapes',
        dataIndex: 'inputShapes',
        sorter: true,
    },
    {
        title: 'Input Data Types',
        dataIndex: 'inputDataTypes',
        sorter: true,
    },
    {
        title: 'Input Formats',
        dataIndex: 'inputFormats',
        sorter: true,
    },
    {
        title: 'Output Shapes',
        dataIndex: 'outputShapes',
        sorter: true,
    },
    {
        title: 'Output Data Types',
        dataIndex: 'outputDataTypes',
        sorter: true,
    },
    {
        title: 'Output Formats',
        dataIndex: 'outputFormats',
        sorter: true,
    },
];

const communicationStatisticsColumns = [
    {
        title: 'Overlapped Type',
        dataIndex: 'overlapType',
        key: 'overlapType',
    },
    {
        title: 'Total Durations(μs)',
        dataIndex: 'duration',
        key: 'duration',
    },
];

const communicationDetailColumns = [
    {
        title: 'Communication Kernel',
        dataIndex: 'communicationKernel',
        key: 'communicationKernel',
        sorter: true,
    },
    {
        title: 'Start Time(ms)',
        dataIndex: 'startTime',
        key: 'startTime',
        sorter: true,
    },
    {
        title: 'Communication Durations(μs)',
        dataIndex: 'totalDuration',
        key: 'totalDuration',
        sorter: true,
    },
];

const communicationOverlappedDetailColumns = [
    ...communicationDetailColumns,
    {
        title: 'Overlapped Durations(μs)',
        dataIndex: 'overlapDuration',
        key: 'overlapDuration',
        sorter: true,
    },
];

const communicationNotOverlappedDetailColumns = [
    ...communicationDetailColumns,
    {
        title: 'Not Overlapped Durations(μs)',
        dataIndex: 'notOverlapDuration',
        key: 'notOverlapDuration',
        sorter: true,
    },
];

const rowKeyMap: any = {
    compute: 'acceleratorCore',
    communication: 'overlapType',
};
const colMap: any = {
    compute: computingStatisticsColumns,
    computeDetail: computingDetailColumns,
    communication: communicationStatisticsColumns,
    'Communication(Overlapped)Detail': communicationOverlappedDetailColumns,
    'Communication(Not Overlapped)Detail': communicationNotOverlappedDetailColumns,
};

const getTableSet = (timeFlag: string, setExpandedKeys?: any): any => {
    const rowKey = rowKeyMap[timeFlag];
    const columns = notNull(colMap[timeFlag]) ? [...colMap[timeFlag]] : [];
    if ([ 'compute', 'communication' ].includes(timeFlag)) {
        const btnCol = {
            title: 'Details',
            render: (_: any, record: any) => (<Button type="link"
                onClick={() => {
                    setExpandedKeys((pre: string[]) => {
                        const list = [...pre];
                        const keyIndex = list.indexOf(record[rowKey]);
                        if (keyIndex === -1) {
                            list.push(record[rowKey]);
                        } else {
                            list.splice(keyIndex, 1);
                        }
                        return list;
                    });
                }}>details<DownOutlined/></Button>),
        };
        columns.push(btnCol);
    }

    return { rowKey, columns };
};

const serachData = async({ rankId, record, page, sorter, name, step }: any): Promise<{total: number;data: object[]}> => {
    let data;
    let total;
    const param = {
        rankId,
        timeFlag: record.acceleratorCore,
        currentPage: page.current,
        pageSize: page.pageSize,
        orderBy: sorter.field,
        order: sorter.order,
        step: step === 'All' ? '' : step,
    };
    if (name === 'computeDetail') {
        const res = await queryComputeDetail(param);
        data = res.computeDetails;
        data = data.map((item: any) => ({
            ...item,
            startTime: Number(item.startTime?.toFixed(4)),
            duration: Number(item.duration?.toFixed(4)),
            waitTime: Number(item.waitTime?.toFixed(4)),
        }));
        total = res.totalNum;
    } else {
        const res = await queryCommunicationDetail(param);
        data = res.communicationDetail;
        data = data.map((item: any) => ({
            ...item,
            startTime: Number((item.startTime / 1000000).toFixed(4)),
            totalDuration: Number((item.totalDuration / 1000).toFixed(4)),
            overlapDuration: Number((item.overlapDuration / 1000).toFixed(4)),
            notOverlapDuration: Number((item.notOverlapDuration / 1000)?.toFixed(4)),
        }));
        total = res.totalNum;
    }
    return { data, total };
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: 'descend' };
const DtetailTable = ({ rankId, record, name, step }: any): JSX.Element => {
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ page, setPage ] = useState(defaultPage);
    const [ sorter, setSorter ] = useState(defaultSorter);
    const { columns, rowKey } = getTableSet(name);
    useEffect(() => {
        updateData(page, sorter);
    }, [ page.current, page.pageSize, sorter.field, sorter.order, record.acceleratorCore, rankId ]);
    const updateData = async(page: any, sorter: any): Promise<void> => {
        const { data, total } = await serachData({ rankId, record, page, sorter, name, step });
        setDataSource(data);
        setPage({ ...page, total });
    };

    return <div>
        <ResizeTable
            dataSource={dataSource}
            columns={columns}
            rowKey={rowKey}
            size="small"
            pagination={GetPageConfigWhithPageData(page, setPage)}
            onChange={(pagination: any, filters: any, sorter: any, extra: any) => {
                if (extra.action === 'sort') {
                    setSorter(sorter);
                }
            }
            }
        />
    </div>;
};

const timeFlagMap: StringMap = {
    compute: 'Computing',
    communicationNotOverLappedTime: 'Communication(Not OverLapped)',
    communicationOverLappedTime: 'Communication(OverLapped)',
    freeTime: 'Free',
};
function getTitle(timeFlag: string): string {
    return (timeFlagMap[timeFlag]) + ' Detail';
}

export const ComputeStatisticsTable = (props: any): JSX.Element => {
    const timeFlag = 'compute';
    const { rankId = '', step = '' } = props;
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [ props.rankId, props.step ]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId, stepId: step === 'All' ? '' : step });
        let data = res.summaryStatisticsItemList ?? [];
        data = data.map((item: any) => ({
            ...item,
            duration: Number(item.duration.toFixed(4)),
            utilization: Number(item.utilization.toFixed(4)),
        }));
        setDataSource(data);
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DtetailTable record={record}
                name={timeFlag + 'Detail' } rankId={ rankId} step={step}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
        size="small"
    />;
};

export const CommunicationStatisticsTable = (props: any): JSX.Element => {
    const timeFlag = 'communication';
    const { rankId = '', step = '' } = props;
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [ props.rankId, props.step ]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId, stepId: step === 'All' ? '' : step });
        setDataSource(res.summaryStatisticsItemList ?? [ ]);
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DtetailTable record={record}
                name={record.overlapType + 'Detail' } rankId={ rankId} step={step}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
        size="small"
    />;
};

const StatisticsTable = (props: any): JSX.Element => {
    const { rankId = '', step = '' } = props;
    return notNull(rankId)
        ? (
            <div>
                <div style={{ marginBottom: '20px' }}>
                    <div className={'common-title-h2'}>
                        {getTitle('compute')} ( Rank {rankId} )
                    </div>
                    <ComputeStatisticsTable rankId={rankId} step={step}/>
                </div>
                <div style={{ marginBottom: '20px' }}>
                    <div className={'common-title-h2'}>
                        {'Communication Detail'} ( Rank {rankId} )
                    </div>
                    <CommunicationStatisticsTable rankId={rankId} step={step}/>
                </div>
            </div>)
        : <></>
    ;
};
export default StatisticsTable;
