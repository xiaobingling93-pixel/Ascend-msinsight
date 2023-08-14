/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Table, Button } from 'antd';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import { Container, GetPageConfigWhithAllData, PaginationWhithPgaeData } from './Common';
import { VoidFunction } from '../../utils/interface';
import { queryOperatorDetails } from '../../utils/RequestUtils';

export interface DataType {
    'Rank ID': string ;
    'Operator Name': string ;
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

const commonColumns = [
    {
        title: 'Elapse Time(ms)',
        dataIndex: 'Elapse Time(ms)',
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'Transit Time(ms)',
    },
    {
        title: 'Synchronization Time(ms)',
        dataIndex: 'Synchronization Time(ms)',
        sorter: (a: DataType, b: DataType) => a.SynchronizationTime - b.SynchronizationTime,
    },
    {
        title: 'Wait Time(ms)',
        dataIndex: 'Wait Time(ms)',
    },
    {
        title: 'Synchronization Time Ratio',
        dataIndex: 'Synchronization Time Ratio',
    },
    {
        title: 'Wait Time Ratio',
        dataIndex: 'Wait Time Ratio',
    },
    {
        title: 'Idle Time(ms)',
        dataIndex: 'Idle Time(ms)',
    } ];

const defaultDataSource: DataType[] = [];

// Total HCCL Opertators表
const OperatorsTable = (record: any): JSX.Element => {
    useEffect(() => {
        updateData();
    }, [ ]);
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const { rankId } = record;
    const updateData = async(page?: any): Promise<void> => {
        const data = await queryOperatorDetails({ rankId, ...page });
        setDataSource(data);
        setPageInfo({ total: data.length });
    };

    const [ pageInfo, setPageInfo ] = useState({ total: 0 });

    const columns: TableColumnsType<DataType> = [
        { title: 'Operator Name', dataIndex: 'Operator Name', key: 'Operator Name' },
        ...commonColumns,
    ];
    return <div>
        <Table columns={columns} dataSource={dataSource} pagination={false} size="small"/>
        <PaginationWhithPgaeData handlePageChange={updateData} total={pageInfo.total}/>
    </div>;
};

const getRankColumns = (handleAction: VoidFunction[]): any => {
    const [ showOperator, setExpandedKeys ] = handleAction;
    return [
        {
            title: 'Rank ID',
            dataIndex: 'Rank ID',
            key: 'Rank ID',
        },
        ...commonColumns,
        {
            title: 'Bandwidth Analysis',
            key: 'action1',
            render: (_: any, record: DataType) => (
                <Button type="link"
                    onClick={() => {
                        showOperator(record['Rank ID']);
                    }}>see more</Button>),
        },
        {
            title: 'Communication Operators Details',
            key: 'action2',
            render: (_: any, record: DataType) => (<Button type="link"
                onClick={() => {
                    setExpandedKeys((pre: any) => {
                        const list = [...pre];
                        const keyIndex = list.indexOf(record['Rank ID']);
                        if (keyIndex === -1) {
                            list.push(record['Rank ID']);
                        } else {
                            list.splice(keyIndex, 1);
                        }
                        return list;
                    });
                }}>see more<DownOutlined/></Button>),
        },
    ];
};
const CommunicationTimeTable = observer(function (props: {dataSource?: DataType[];showOperator: (rankid: string) => void}) {
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const columns = getRankColumns([ props.showOperator, setExpandedKeys ]);
    const dataSource: DataType[] = props.dataSource ?? defaultDataSource;
    return (
        <Container
            title={'DataAnalysis of Communication Time'}
            content={<Table
                dataSource={dataSource}
                columns={columns}
                expandable={{
                    expandedRowRender: (record: DataType) => <div style={{ marginLeft: '30px' }}><OperatorsTable record={record}/></div>,
                    expandedRowKeys,
                    expandIcon: () => (<></>),
                }}
                rowKey={'Rank ID'}
                pagination={GetPageConfigWhithAllData(dataSource.length)}
                size="small"
            />
            }
        />

    );
});

export default CommunicationTimeTable;
