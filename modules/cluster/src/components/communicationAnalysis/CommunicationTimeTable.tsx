/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Button } from 'antd';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import type { ColumnsType } from 'antd/es/table';
import {
    Container,
    GetPageConfigWhithAllData,
    GetPageConfigWhithPageData,
} from '../Common';
import { VoidFunction } from '../../utils/interface';
import { queryOperatorDetails } from '../../utils/RequestUtils';
import { totalOperator } from './Filter';
import ResizeTable from '../resize/ResizeTable';
import type { Session } from '../../entity/session';

export interface DataType {
    'Rank ID': string ;
    'Operator Name': string ;
    'Start Timestamp(us)': string | number;
    'Elapse Time(ms)': string | number;
    'Transit Time(ms)': string | number;
    'Synchronization Time(ms)': number;
    'Wait Time(ms)': string | number;
    'Synchronization Time Ratio': string | number;
    'Wait Time Ratio': string | number;
    'Idle Time(ms)': string | number;
    'Communication Bandwidth Info'?: any;
    [prop: string]: any;
}

const commonColumns: ColumnsType<DataType> = [
    {
        title: 'Start Time(ms)',
        dataIndex: 'startTime',
        sorter: (a: DataType, b: DataType) => a.startTime - b.startTime,
        ellipsis: true,
    },
    {
        title: 'Elapse Time(ms)',
        dataIndex: 'elapseTime',
        sorter: (a: DataType, b: DataType) => a.elapseTime - b.elapseTime,
        ellipsis: true,
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'transitTime',
        sorter: (a: DataType, b: DataType) => a.transitTime - b.transitTime,
        ellipsis: true,
    },
    {
        title: 'Synchronization Time(ms)',
        dataIndex: 'synchronizationTime',
        sorter: (a: DataType, b: DataType) => a.synchronizationTime - b.synchronizationTime,
        ellipsis: true,
    },
    {
        title: 'Wait Time(ms)',
        dataIndex: 'waitTime',
        sorter: (a: DataType, b: DataType) => a.waitTime - b.waitTime,
        ellipsis: true,
    },
    {
        title: 'Synchronization Time Ratio',
        dataIndex: 'synchronizationTimeRatio',
        sorter: (a: DataType, b: DataType) => a.synchronizationTimeRatio - b.synchronizationTimeRatio,
        ellipsis: true,
    },
    {
        title: 'Wait Time Ratio',
        dataIndex: 'waitTimeRatio',
        sorter: (a: DataType, b: DataType) => a.waitTimeRatio - b.waitTimeRatio,
        ellipsis: true,
    },
    {
        title: 'Idle Time(ms)',
        dataIndex: 'idleTime',
        sorter: (a: DataType, b: DataType) => a.idleTime - b.idleTime,
        ellipsis: true,
    }];

// Total HCCL Opertators表
const OperatorsTable = ({ record, conditions }: any): JSX.Element => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'elapseTime', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);

    useEffect(() => {
        updateData(page, sorter);
    }, [page.current, page.pageSize, sorter.field, sorter.order, conditions.iterationId, record.rankId]);
    const updateData = async(page: any, sorter: {field: string;order: string}): Promise<void> => {
        const res = await queryOperatorDetails({
            iterationId: conditions.iterationId,
            rankId: record.rankId,
            currentPage: page.current,
            pageSize: page.pageSize,
            orderBy: sorter.field,
            order: sorter.order,
            stage: conditions.stage,
        });
        setDataSource(res.allOperators);
        setPage({ ...page, total: res.count });
    };

    const columns: TableColumnsType<DataType> = [
        { title: 'Operator Name', dataIndex: 'operatorName', key: 'operatorName', sorter: true, ellipsis: true },
        ...commonColumns.map(item => {
            return { ...item, sorter: true };
        }),
    ];
    return <div>
        <ResizeTable columns={columns} dataSource={dataSource} size="small"
            pagination={GetPageConfigWhithPageData(page, setPage)}
            onChange={(pagination: any, filters: any, sorter: any, extra: any) => {
                if (extra.action === 'sort') {
                    setSorter(sorter);
                }
            }}
        />
    </div>;
};

const getRankColumns = (handleAction: VoidFunction[], conditions: any): any => {
    const [showOperator, setExpandedKeys] = handleAction;
    return [
        {
            title: 'Rank ID',
            dataIndex: 'rankId',
            key: 'rankId',
            ellipsis: true,
            width: 70,
        },
        ...commonColumns,
        {
            title: 'Bandwidth Analysis',
            key: 'action1',
            ellipsis: true,
            width: 110,
            minWidth: 100,
            render: (_: any, record: DataType) => (
                <Button type="link"
                    onClick={() => {
                        showOperator(record.rankId);
                    }}>see more</Button>),
        },
        {
            title: 'Communication Operators Details',
            key: 'action2',
            ellipsis: true,
            width: 110,
            minWidth: 110,
            render: (_: any, record: DataType) => (<Button type="link"
                onClick={() => {
                    setExpandedKeys((pre: any) => {
                        const list = [...pre];
                        const keyIndex = list.indexOf(record[rowKey]);
                        if (keyIndex === -1) {
                            list.push(record[rowKey]);
                        } else {
                            list.splice(keyIndex, 1);
                        }
                        return list;
                    });
                }}>see more<DownOutlined/></Button>),
            display: conditions.operatorName === totalOperator,
        },
    ].filter((item: any) => item.display !== false);
};
const rowKey = 'index';
const CommunicationTimeTable = observer(function (props:
{dataSource?: DataType[];showOperator: (rankid: string) => void;conditions: any;updateSort: VoidFunction; session: Session}) {
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const columns = getRankColumns([props.showOperator, setExpandedKeys], props.conditions);
    const dataSource: DataType[] = props.dataSource ?? [];
    useEffect(() => {
        setExpandedKeys([]);
    }, [props.dataSource]);
    return (
        <Container
            title={'Data Analysis of Communication Time'}
            style={{ margin: '1rem 0' }}
            content={<ResizeTable
                loading={!props.session.durationFileCompleted}
                dataSource={dataSource}
                columns={columns}
                expandable={{
                    expandedRowRender: (record: DataType) => <div style={{ marginLeft: '0' }}>
                        <OperatorsTable record={record} conditions={props.conditions}/>
                    </div>,
                    expandedRowKeys,
                    expandIcon: () => (<></>),
                }}
                rowKey={rowKey}
                pagination={GetPageConfigWhithAllData(dataSource.length)}
                size="small"
                onChange={(pagination: any, filters: any, sorter: any, extra: any) => {
                    if (extra.action === 'sort') {
                        setExpandedKeys([]);
                        props.updateSort(extra.currentDataSource);
                    }
                }
                }
            />
            }
        />

    );
});

export default CommunicationTimeTable;
