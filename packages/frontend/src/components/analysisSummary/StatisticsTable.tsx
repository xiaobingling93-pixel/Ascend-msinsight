/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button, Table } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { StringMap } from '../../utils/interface';
import { PaginationWhithPgaeData, notNull } from '../communicationAnalysis/Common';
import { querySummaryDetail, querySummaryStatistics } from '../../utils/RequestUtils';

const computingStatisticsColumns = [
    {
        title: 'Accelerator Core',
        dataIndex: 'Accelerator Core',
        key: 'Accelerator Core',
    },
    {
        title: 'Accelerator Core Durations(us)',
        dataIndex: 'Accelerator Core Durations(us)',
        key: 'Accelerator Core Durations(us)',
    },

    {
        title: 'Accelerator Core Utilization',
        dataIndex: 'Accelerator Core Utilization',
        key: 'Accelerator Core Utilization',
    },
];

const computingDetailColumns = [
    {
        title: 'Name',
        dataIndex: 'name',
    },
    {
        title: 'Type',
        dataIndex: 'type',
    },
    {
        title: 'Start Time',
        dataIndex: 'startTime',
    },
    {
        title: 'Duration(us)',
        dataIndex: 'duration',
    },
    {
        title: 'Wait Time(us)',
        dataIndex: 'waitTime',
    },
    {
        title: 'Block Dim',
        dataIndex: 'blockDim',
    },
    {
        title: 'Input Shapes',
        dataIndex: 'inputShapes',
    },
    {
        title: 'Input Data Types',
        dataIndex: 'inputDataTypes',
    },
    {
        title: 'Input Formats',
        dataIndex: 'inputFormats',
    },
    {
        title: 'Output Shapes',
        dataIndex: 'outputShapes',
    },
    {
        title: 'Output Data Types',
        dataIndex: 'outputDataTypes',
    },
    {
        title: 'Output Formats',
        dataIndex: 'outputFormats',
    },
];

const communicationStatisticsColumns = [
    {
        title: 'Transport Type',
        dataIndex: 'Transport Type',
        key: 'Transport Type',
    },
    {
        title: 'Total Durations(us)',
        dataIndex: 'Total Durations(us)',
        key: 'Total Durations(us)',
    },
];

const communicationColumns = [
    {
        title: 'Communication Kernel',
        dataIndex: 'Communication Kernel',
        key: 'Communication Kernel',
    },
    {
        title: 'Start Time',
        dataIndex: 'Start Time',
        key: 'Start Time',
    },
    {
        title: 'Total Durations(us)',
        dataIndex: 'Total Durations(us)',
        key: 'Total Durations(us)',
    },
];

const communicationOverlappedColumns = [
    ...communicationColumns,
    {
        title: 'Overlapped Durations(us)',
        dataIndex: 'Overlapped Durations(us)',
        key: 'Overlapped Durations(us)',
    },
];

const communicationNotOverlappedColumns = [
    ...communicationColumns,
    {
        title: 'Not Overlapped Durations(us)',
        dataIndex: 'Not Overlapped Durations(us)',
        key: 'Not Overlapped Durations(us)',
    },
];

const getTableSet = (timeFlag: string, setExpandedKeys?: any): any => {
    const rowKeyMap: any = {
        Computing: 'Accelerator Core',
        ComputingDetail: 'Name',
        'Communication(OverLapped)': 'Transport Type',
        'Communication(Not OverLapped)': 'Transport Type',
    };
    const colMap: any = {
        Computing: computingStatisticsColumns,
        ComputingDetail: computingDetailColumns,
        'Communication(OverLapped)': communicationStatisticsColumns,
        'Communication(Not OverLapped)': communicationStatisticsColumns,
        'Communication(OverLapped)Detail': communicationOverlappedColumns,
        'Communication(Not OverLapped)Detail': communicationNotOverlappedColumns,
    };
    const rowKey = rowKeyMap[timeFlag];
    const columns = notNull(colMap[timeFlag]) ? [...colMap[timeFlag]] : [];

    if ([ 'Computing', 'Communication(OverLapped)', 'Communication(Not OverLapped)' ].includes(timeFlag)) {
        const btnCol = {
            title: 'See Details',
            key: 'action',
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
                }}>see details<DownOutlined/></Button>),
        };
        columns.push(btnCol);
    }

    return { rowKey, columns };
};

const DtetailTable = ({ rankId, timeFlag }: any): JSX.Element => {
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag);
    useEffect(() => {
        updateData();
    }, []);
    const updateData = async(page?: any): Promise<void> => {
        const data = await querySummaryDetail({ rankId, timeFlag, ...page });
        setDataSource(data);
        setPageInfo({ total: data.length });
    };

    const [ pageInfo, setPageInfo ] = useState({ total: 0 });
    const handlePageChange = (page: any): void => {
        updateData(page);
    };

    return <>
        <Table
            dataSource={dataSource}
            columns={columns}
            rowKey={rowKey}
            pagination={ false}
            size="small"
        />
        <PaginationWhithPgaeData handlePageChange={handlePageChange} total={pageInfo.total}/>
    </>;
};

const timeFlagMap: StringMap = {
    totalComputeTime: 'Computing',
    totalCommunicationNotOverLapTime: 'Communication(Not OverLapped)',
    totalCommunicationTime: 'Communication(OverLapped)',
    totalFreeTime: 'Free',
};
function getTitle(timeFlag: string): string {
    return (timeFlagMap[timeFlag] || '') + ' Detail';
}
const StatisticsTable = (props: any): JSX.Element => {
    let { timeFlag = '', rankId = '' } = props;
    timeFlag = timeFlagMap[timeFlag];
    useEffect(() => {
        updateData();
    }, [ props.rankId, props.timeFlag ]);
    const updateData = async (): Promise<void> => {
        const data = await querySummaryStatistics({ timeFlag, rankId });
        setDataSource(data);
    };
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);

    return notNull(rankId) && notNull(timeFlag)
        ? (
            <div>
                <div className={'common-title'}>{getTitle(props.timeFlag)}</div>
                <Table
                    dataSource={dataSource}
                    columns={columns}
                    expandable={{
                        expandedRowRender: record => <DtetailTable timeFlag={timeFlag + 'Detail' } rankId={rankId}/>,
                        expandedRowKeys,
                        expandIcon: () => (<></>),
                    }}
                    rowKey={rowKey}
                    pagination={false}
                    size="small"
                />
            </div>)
        : <></>
    ;
};
export default StatisticsTable;
