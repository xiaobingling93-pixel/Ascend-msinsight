import { observer } from 'mobx-react-lite';
import React, { useState } from 'react';
import { Table, Button } from 'antd';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';

interface DataType {
    RankID: string ;
    OperatorsName?: string ;
    ElapseTime: string | number;
    TransitTime: string | number;
    SynchronizationTime: number;
    WaitTime: string | number;
    SynchronizationTimeRatio: string | number;
    WaitTimeRatio: string | number;
    IdleTime: string | number;
}

const commonColumns = [
    {
        title: 'Elapse Time(ms)',
        dataIndex: 'ElapseTime',
        key: 'ElapseTime',
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'TransitTime',
        key: 'TransitTime',
    },
    {
        title: 'Synchronization Time(ms)',
        dataIndex: 'SynchronizationTime',
        key: 'SynchronizationTime',
        sorter: (a: DataType, b: DataType) => a.SynchronizationTime - b.SynchronizationTime,
    },
    {
        title: 'Wait Time(ms)',
        dataIndex: 'WaitTime',
        key: 'WaitTime',
    },
    {
        title: 'Synchronization Time Ratio',
        dataIndex: 'SynchronizationTimeRatio',
        key: 'SynchronizationTimeRatio',
    },
    {
        title: 'Wait Time Ratio',
        dataIndex: 'WaitTimeRatio',
        key: 'WaitTimeRatio',
    },
    {
        title: 'Idle Time(ms)',
        dataIndex: 'IdleTime',
        key: 'IdleTime',
    } ];

const defaultDataSource = [
    {
        RankID: '1',
        ElapseTime: 62.9322,
        TransitTime: 5,
        SynchronizationTime: 55,
        WaitTime: 60,
        SynchronizationTimeRatio: 0.95,
        WaitTimeRatio: 0.96,
        IdleTime: 0.08,
    },
    {
        RankID: '2',
        ElapseTime: 62.8256,
        TransitTime: 7,
        SynchronizationTime: 52,
        WaitTime: 55,
        SynchronizationTimeRatio: 1.2,
        WaitTimeRatio: 0.91,
        IdleTime: 0.1,
    },
];

// Total HCCL Opertators表
const expandedRowRender = (): JSX.Element => {
    const columns: TableColumnsType<DataType> = [
        { title: 'Operators Name', dataIndex: 'OperatorsName', key: 'OperatorsName' },
        ...commonColumns,
    ];

    const data = [];
    for (let i = 0; i < 3; ++i) {
        data.push({
            RankID: i.toString(),
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
            dataIndex: 'RankID',
            key: 'RankID',
        },
        ...commonColumns,
        {
            title: 'Bandwidth Analysis',
            key: 'action1',
            render: (_: any, record: DataType) => (
                <Button type="link"
                    onClick={() => {
                        props.showOperator(record.RankID);
                    }}>see more</Button>),
        },
        {
            title: 'Communication Operators Details',
            key: 'action2',
            render: (_: any, record: DataType) => (<Button type="link"
                onClick={() => {
                    setExpandedKeys(pre => {
                        const list = [...pre];
                        const keyIndex = list.indexOf(record.RankID);
                        if (keyIndex === -1) {
                            list.push(record.RankID);
                        } else {
                            list.splice(keyIndex, 1);
                        }
                        return list;
                    });
                }}>see more<DownOutlined/></Button>),
        },
    ];
    const dataSource = props.dataSource ?? defaultDataSource;
    return (<Table
        dataSource={dataSource}
        columns={columns}
        expandable={{ expandedRowRender, expandedRowKeys, expandIcon: () => (<></>) }}
        rowKey={'RankID'}/>);
});

export default CommunicationTimeTable;
