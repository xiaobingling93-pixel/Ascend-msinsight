/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button, Table } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { StringMap } from '../../utils/interface';
import { notNull, GetPageConfigWhithPageData } from '../Common';
import { queryCommunicationDetail, queryComputeDetail, querySummaryStatistics } from '../../utils/RequestUtils';

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
        dataIndex: 'wait_time',
        sorter: true,
    },
    {
        title: 'Block Dim',
        dataIndex: 'block_dim',
        sorter: true,
    },
    {
        title: 'Input Shapes',
        dataIndex: 'input_shapes',
        sorter: true,
    },
    {
        title: 'Input Data Types',
        dataIndex: 'input_data_types',
        sorter: true,
    },
    {
        title: 'Input Formats',
        dataIndex: 'input_formats',
        sorter: true,
    },
    {
        title: 'Output Shapes',
        dataIndex: 'output_shapes',
        sorter: true,
    },
    {
        title: 'Output Data Types',
        dataIndex: 'output_data_types',
        sorter: true,
    },
    {
        title: 'Output Formats',
        dataIndex: 'output_formats',
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
        title: 'Start Time',
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
    },
];

const communicationNotOverlappedDetailColumns = [
    ...communicationDetailColumns,
    {
        title: 'Not Overlapped Durations(μs)',
        dataIndex: 'notOverlapDuration',
        key: 'notOverlapDuration',
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

const serachData = async({ rankId, record, page, sorter, name }: any): Promise<{total: number;data: object[]}> => {
    let data;
    let total;
    const param = {
        rankId,
        timeFlag: record.acceleratorCore,
        currentPage: page.current,
        pageSize: page.pageSize,
        orderBy: sorter.field,
        order: sorter.order,
    };
    if (name === 'computeDetail') {
        const res = await queryComputeDetail(param);
        data = res.computeDetail;
        data = data.map((item: any) => ({
            ...item,
            startTime: Number(item.startTime.toFixed(4)),
            duration: Number(item.duration.toFixed(4)),
            wait_time: Number(item.wait_time.toFixed(4)),
        }));
        total = res.totalNum;
    } else {
        const res = await queryCommunicationDetail(param);
        data = res.communicationDetail;
        total = res.totalNum;
    }
    return { data, total };
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: 'descend' };
const DtetailTable = ({ rankId, record, name }: any): JSX.Element => {
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ page, setPage ] = useState(defaultPage);
    const [ sorter, setSorter ] = useState(defaultSorter);
    const { columns, rowKey } = getTableSet(name);
    useEffect(() => {
        updateData(page, sorter);
    }, [ page.current, page.pageSize, sorter.field, sorter.order, record.acceleratorCore, rankId ]);
    const updateData = async(page: any, sorter: any): Promise<void> => {
        const { data, total } = await serachData({ rankId, record, page, sorter, name });
        setDataSource(data);
        setPage({ ...page, total });
    };

    return <div>
        <Table
            dataSource={dataSource}
            columns={columns}
            rowKey={rowKey}
            size="small"
            pagination={GetPageConfigWhithPageData(page, setPage)}
            onChange={(pagination, filters, sorter: any, extra) => {
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
    const { rankId = '' } = props;
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId });
        let data = res.result ?? [];
        data = data.map((item: any) => ({
            ...item,
            duration: Number(item.duration.toFixed(4)),
            utilization: Number(item.utilization.toFixed(4)),
        }));
        setDataSource(data);
    };

    return <Table
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: record => <DtetailTable record={record}
                name={timeFlag + 'Detail' } rankId={ rankId}/>,
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
    const { rankId = '' } = props;
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId });
        setDataSource(res.result ?? [ ]);
    };

    return <Table
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: record => <DtetailTable record={record}
                name={record.overlapType + 'Detail' } rankId={ rankId}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
        size="small"
    />;
};

const StatisticsTable = (props: any): JSX.Element => {
    const { rankId = '' } = props;
    return (
        <div style={{ display: notNull(rankId) ? 'block' : 'none' }}>
            <div style={{ marginBottom: '20px' }}>
                <div className={'common-title-h2'}>
                    {getTitle('compute')} ( Rank {rankId} )
                </div>
                <ComputeStatisticsTable rankId={rankId}/>
            </div>
            <div style={{ marginBottom: '20px' }}>
                <div className={'common-title-h2'}>
                    {'Communication Detail'} ( Rank {rankId} )
                </div>
                <CommunicationStatisticsTable rankId={rankId}/>
            </div>
        </div>)
    ;
};
export default StatisticsTable;
