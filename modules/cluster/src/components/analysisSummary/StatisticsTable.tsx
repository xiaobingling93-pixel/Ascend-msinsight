/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { StringMap } from '../../utils/interface';
import { notNull, GetPageConfigWhithPageData, Loading } from '../Common';
import { queryCommunicationDetail, queryComputeDetail, querySummaryStatistics } from '../../utils/RequestUtils';
import ResizeTable from '../resize/ResizeTable';
import { Session } from '../../entity/session';

const computingStatisticsColumns = [
    {
        title: 'Accelerator Core',
        dataIndex: 'acceleratorCore',
        key: 'acceleratorCore',
        ellipsis: true,
    },
    {
        title: 'Accelerator Core Durations(μs)',
        dataIndex: 'duration',
        key: 'duration',
        ellipsis: true,
    },
];

const computingDetailColumns = [
    {
        title: 'Name',
        dataIndex: 'name',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Type',
        dataIndex: 'type',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Start Time(ms)',
        dataIndex: 'startTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Duration(μs)',
        dataIndex: 'duration',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Wait Time(μs)',
        dataIndex: 'waitTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Block Dim',
        dataIndex: 'blockDim',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Shapes',
        dataIndex: 'inputShapes',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Data Types',
        dataIndex: 'inputDataTypes',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Formats',
        dataIndex: 'inputFormats',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Shapes',
        dataIndex: 'outputShapes',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Data Types',
        dataIndex: 'outputDataTypes',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Formats',
        dataIndex: 'outputFormats',
        sorter: true,
        ellipsis: true,
    },
];

const communicationStatisticsColumns = [
    {
        title: 'Accelerator Core',
        dataIndex: 'acceleratorCore',
        key: 'acceleratorCore',
        ellipsis: true,
    },
    {
        title: 'Communication(Not Overlapped) Durations(μs)',
        dataIndex: 'NotOverlapped',
        key: 'NotOverlapped',
        ellipsis: true,
    },
    {
        title: 'Communication(Overlapped) Durations(μs)',
        dataIndex: 'Overlapped',
        key: 'Overlapped',
        ellipsis: true,
    },
];

const communicationDetailColumns = [
    {
        title: 'Name',
        dataIndex: 'name',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Type',
        dataIndex: 'type',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Start Time(ms)',
        dataIndex: 'startTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Duration(μs)',
        dataIndex: 'duration',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Wait Time(μs)',
        dataIndex: 'waitTime',
        sorter: true,
        ellipsis: true,
    },
];

const communicationOverlappedDetailColumns = [
    ...communicationDetailColumns,
    {
        title: 'Overlapped Durations(μs)',
        dataIndex: 'overlapDuration',
        key: 'overlapDuration',
        sorter: true,
        ellipsis: true,
    },
];

const communicationNotOverlappedDetailColumns = [
    ...communicationDetailColumns,
    {
        title: 'Not Overlapped Durations(μs)',
        dataIndex: 'notOverlapDuration',
        key: 'notOverlapDuration',
        sorter: true,
        ellipsis: true,
    },
];

const rowKeyMap: any = {
    compute: 'acceleratorCore',
    communication: 'acceleratorCore',
};
const colMap: any = {
    compute: computingStatisticsColumns,
    computeDetail: computingDetailColumns,
    communication: communicationStatisticsColumns,
    CommunicationDetail: communicationDetailColumns,
    'Communication(Overlapped)Detail': communicationOverlappedDetailColumns,
    'Communication(Not Overlapped)Detail': communicationNotOverlappedDetailColumns,
};

const getTableSet = (timeFlag: string, setExpandedKeys?: any): any => {
    const rowKey = rowKeyMap[timeFlag];
    const columns = notNull(colMap[timeFlag]) ? [...colMap[timeFlag]] : [];
    if ([ 'compute', 'communication' ].includes(timeFlag)) {
        const btnCol = {
            title: 'Details',
            ellipsis: true,
            minWidth: 85,
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
        data = res.communicationDetails;
        total = res.totalNum;
    }
    return { data, total };
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: 'descend' };
const DetailTable = ({ rankId, record, name, step }: any): JSX.Element => {
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
            expandedRowRender: (record: any) => <DetailTable record={record}
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
        const list: any[] = res.summaryStatisticsItemList ?? [ ];
        const data = {
            acceleratorCore: 'HCCL',
            Overlapped: list.find(item => item.overlapType === 'Communication(Overlapped)')?.duration,
            NotOverlapped: list.find(item => item.overlapType === 'Communication(Not Overlapped)')?.duration,
        };
        setDataSource([data]);
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DetailTable record={record}
                name={'CommunicationDetail' } rankId={ rankId} step={step}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
        size="small"
    />;
};

const StatisticsTable = (props: {step: string;rankId: string;session: Session}): JSX.Element => {
    const { rankId = '', step = '', session } = props;
    return notNull(rankId) && session.unitcount > 0
        ? (
            <div>
                <div style={{ marginBottom: '20px' }}>
                    <div className={'common-title-h2'}>
                        {getTitle('compute')} ( Rank {rankId} )
                    </div>
                    {session.parseCompleted
                        ? (<ComputeStatisticsTable rankId={rankId} step={step} session={session}/>)
                        : <Loading style={{ margin: '10px auto' }}/>
                    }
                </div>
                <div style={{ marginBottom: '20px' }}>
                    <div className={'common-title-h2'}>
                        {'Communication Detail'} ( Rank {rankId} )
                    </div>
                    {session.parseCompleted
                        ? <CommunicationStatisticsTable rankId={rankId} step={step} session={session}/>
                        : <Loading style={{ margin: '10px auto' }}/>
                    }
                </div>
            </div>)
        : <></>
    ;
};
export default StatisticsTable;
