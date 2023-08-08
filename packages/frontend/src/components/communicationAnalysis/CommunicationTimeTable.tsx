import { observer } from 'mobx-react-lite';
import React, { useState } from 'react';
import { Table, Button } from 'antd';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import { Container } from './Common';

interface DataType {
    'Rank ID': string ;
    'Operator Name': string ;
    'Elapse Time(ms)': string | number;
    'Transit Time(ms)': string | number;
    'Synchronization Time(ms)': number;
    'Wait Time(ms)': string | number;
    'Synchronization Time Ratio': string | number;
    'Wait Time Ratio': string | number;
    'Idle Time(ms)': string | number;
    'Communication Bandwidth Info': any;
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
const expandedRowRender = (record: DataType): JSX.Element => {
    const columns: TableColumnsType<DataType> = [
        { title: 'Operators Name', dataIndex: 'OperatorsName', key: 'OperatorsName' },
        ...commonColumns,
    ];

    const data: any = [];
    for (let i = 0; i < 3; ++i) {
        data.push({
            rankId: i.toString(),
            OperatorsName: 'operator' + i.toString(),
            ElapseTime: 62.9322,
            TransitTime: 5,
            SynchronizationTime: 55,
            WaitTime: 60,
            SynchronizationTimeRatio: 0.95,
            WaitTimeRatio: 0.96,
            IdleTime: 0.08,
        });
    }
    return <Table columns={columns} dataSource={data} pagination={false} size="small"/>;
};

const CommunicationTimeTable = observer(function (props: {dataSource?: DataType[];showOperator: (rankid: string) => void}) {
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const columns = [
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
                        props.showOperator(record['Rank ID']);
                    }}>see more</Button>),
        },
        {
            title: 'Communication Operators Details',
            key: 'action2',
            render: (_: any, record: DataType) => (<Button type="link"
                onClick={() => {
                    setExpandedKeys(pre => {
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
    const dataSource = props.dataSource ?? defaultDataSource;
    return (
        <Container
            title={'DataAnalysis of Communication Time'}
            content={<Table
                dataSource={dataSource}
                columns={columns}
                expandable={{ expandedRowRender, expandedRowKeys, expandIcon: () => (<></>) }}
                rowKey={'Rank ID'}/>}
        />

    );
});

export default CommunicationTimeTable;
