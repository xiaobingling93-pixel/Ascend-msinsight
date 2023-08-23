/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Table, Button } from 'antd';
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

const commonColumns: ColumnsType<DataType> = [
    {
        title: 'Elapse Time(ms)',
        dataIndex: 'elapse_time',
        sorter: (a: DataType, b: DataType) => a.elapse_time - b.elapse_time,
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'transit_time',
        sorter: (a: DataType, b: DataType) => a.transit_time - b.transit_time,
    },
    {
        title: 'Synchronization Time(ms)',
        dataIndex: 'synchronization_time',
        sorter: (a: DataType, b: DataType) => a.synchronization_time - b.synchronization_time,
    },
    {
        title: 'Wait Time(ms)',
        dataIndex: 'wait_time',
        sorter: (a: DataType, b: DataType) => a.wait_time - b.wait_time,
    },
    {
        title: 'Synchronization Time Ratio',
        dataIndex: 'synchronization_time_ratio',
        sorter: (a: DataType, b: DataType) => a.synchronization_time_ratio - b.synchronization_time_ratio,
    },
    {
        title: 'Wait Time Ratio',
        dataIndex: 'wait_time_ratio',
        sorter: (a: DataType, b: DataType) => a.wait_time_ratio - b.wait_time_ratio,
    },
    {
        title: 'Idle Time(ms)',
        dataIndex: 'idle_time',
        sorter: (a: DataType, b: DataType) => a.idle_time - b.idle_time,
    } ];

// Total HCCL Opertators表
const OperatorsTable = ({ record, conditions }: any): JSX.Element => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'elapse_time', order: 'descend' };
    const [ dataSource, setDataSource ] = useState<any[]>([]);
    const [ page, setPage ] = useState(defaultPage);
    const [ sorter, setSorter ] = useState(defaultSorter);

    useEffect(() => {
        updateData(page, sorter);
    }, [ page.current, page.pageSize, sorter.field, sorter.order, conditions.iterationId, record.rank_id ]);
    const updateData = async(page: any, sorter: {field: string;order: string}): Promise<void> => {
        const res = await queryOperatorDetails({
            iterationId: conditions.iterationId,
            rankId: record.rank_id,
            currentPage: page.current,
            pageSize: page.pageSize,
            orderBy: sorter.field,
            order: sorter.order,
        });
        setDataSource(res.allOperators);
        setPage({ ...page, total: res.count });
    };

    const columns: TableColumnsType<DataType> = [
        { title: 'Operator Name', dataIndex: 'op_name', key: 'op_name', sorter: true },
        ...commonColumns.map(item => {
            return { ...item, sorter: true };
        }),
    ];
    return <div>
        <Table columns={columns} dataSource={dataSource} size="small"
            pagination={GetPageConfigWhithPageData(page, setPage)}
            onChange={(pagination, filters, sorter: any, extra) => {
                if (extra.action === 'sort') {
                    setSorter(sorter);
                }
            }}
        />
    </div>;
};

const getRankColumns = (handleAction: VoidFunction[], conditions: any): any => {
    const [ showOperator, setExpandedKeys ] = handleAction;
    return [
        {
            title: 'Rank ID',
            dataIndex: rowKey,
            key: rowKey,
        },
        ...commonColumns,
        {
            title: 'Bandwidth Analysis',
            key: 'action1',
            render: (_: any, record: DataType) => (
                <Button type="link"
                    onClick={() => {
                        showOperator(record[rowKey]);
                    }}>see more</Button>),
        },
        {
            title: 'Communication Operators Details',
            key: 'action2',
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
const rowKey = 'rank_id';
const CommunicationTimeTable = observer(function (props:
{dataSource?: DataType[];showOperator: (rankid: string) => void;conditions: any;updateSort: VoidFunction}) {
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const columns = getRankColumns([ props.showOperator, setExpandedKeys ], props.conditions);
    const dataSource: DataType[] = props.dataSource ?? [];
    useEffect(() => {
        setExpandedKeys([]);
    }, [props.dataSource]);
    return (
        <Container
            title={'Data Analysis of Communication Time'}
            style={{ margin: '1rem 0' }}
            content={<Table
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
                onChange={(pagination, filters, sorter, extra) => {
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
