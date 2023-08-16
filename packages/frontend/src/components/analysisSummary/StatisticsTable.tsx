/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button, Table } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { StringMap } from '../../utils/interface';
import { PaginationWhithPgaeData, notNull } from '../Common';
import { queryCommunicationDetail, queryComputeDetail, querySummaryStatistics } from '../../utils/RequestUtils';

const computingStatisticsColumns = [
    {
        title: 'Accelerator Core',
        dataIndex: 'acceleratorCore',
        key: 'acceleratorCore',
    },
    {
        title: 'Accelerator Core Durations(us)',
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
        dataIndex: 'wait_time',
    },
    {
        title: 'Block Dim',
        dataIndex: 'block_dim',
    },
    {
        title: 'Input Shapes',
        dataIndex: 'input_shapes',
    },
    {
        title: 'Input Data Types',
        dataIndex: 'input_data_types',
    },
    {
        title: 'Input Formats',
        dataIndex: 'input_formats',
    },
    {
        title: 'Output Shapes',
        dataIndex: 'output_shapes',
    },
    {
        title: 'Output Data Types',
        dataIndex: 'output_data_types',
    },
    {
        title: 'Output Formats',
        dataIndex: 'output_formats',
    },
];

const communicationStatisticsColumns = [
    {
        title: 'Transport Type',
        dataIndex: 'transportType',
        key: 'transportType',
    },
    {
        title: 'Total Durations(us)',
        dataIndex: 'duration',
        key: 'duration',
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
        compute: 'acceleratorCore',
        communicationOverLappedTime: 'transportType',
        communicationNotOverLappedTime: 'transportType',
    };
    const colMap: any = {
        compute: computingStatisticsColumns,
        computeDetail: computingDetailColumns,
        communicationOverLappedTime: communicationStatisticsColumns,
        communicationNotOverLappedTime: communicationStatisticsColumns,
        communicationOverLappedTimeDetail: communicationOverlappedColumns,
        communicationNotOverLappedTimeDetail: communicationNotOverlappedColumns,
    };
    const rowKey = rowKeyMap[timeFlag];
    const columns = notNull(colMap[timeFlag]) ? [...colMap[timeFlag]] : [];

    if ([ 'compute', 'communicationOverLappedTime', 'communicationNotOverLappedTime' ].includes(timeFlag)) {
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

const DtetailTable = ({ rankId, record, name }: any): JSX.Element => {
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const { columns, rowKey } = getTableSet(name);
    useEffect(() => {
        updateData({ current: 1, pageSize: 10 });
    }, []);
    const updateData = async(page: any): Promise<void> => {
        let data;
        let total;
        if (name === 'computeDetail') {
            const res = await queryComputeDetail({
                rankId,
                timeFlag: record.acceleratorCore,
                currentPage: page.current,
                pageSize: page.pageSize,
            });
            data = res.computeDetail;
            total = res.totalNum;
        } else {
            const res = await queryCommunicationDetail({
                rankId,
                timeFlag: record.transportType,
                currentPage: page.current,
                pageSize: page.pageSize,
            });
            data = res.communicationDetail;
            total = res.totalNum;
        }
        setDataSource(data);
        setPageInfo({ total });
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
    compute: 'Computing',
    communicationNotOverLappedTime: 'Communication(Not OverLapped)',
    communicationOverLappedTime: 'Communication(OverLapped)',
    freeTime: 'Free',
};
function getTitle(timeFlag: string): string {
    return (timeFlagMap[timeFlag]) + ' Detail';
}
const StatisticsTable = (props: any): JSX.Element => {
    const { timeFlag = '', rankId = '' } = props;
    const [ dataSource, setDataSource ] = useState<any>([]);
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const { columns, rowKey } = getTableSet(timeFlag, setExpandedKeys);

    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [ props.rankId, props.timeFlag ]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId });
        setDataSource(res.result ?? []);
    };

    return notNull(rankId) && notNull(timeFlag)
        ? (
            <div>
                <div className={'common-title'}>{getTitle(props.timeFlag)}</div>
                <Table
                    dataSource={dataSource}
                    columns={columns}
                    expandable={{
                        expandedRowRender: record => <DtetailTable record={record}
                            name={timeFlag + 'Detail' } rankId={rankId}/>,
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
