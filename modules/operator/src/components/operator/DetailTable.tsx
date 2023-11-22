/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import ResizeTable from '../resize/ResizeTable';
import React, { useState, useEffect } from 'react';
import { Button } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import { Container, GetPageConfigWhithPageData } from '../Common';
import { type ConditionType } from './Filter';
import { queryOperators, queryOperatorsInStatic, queryOperatorStatic } from '../RequestUtils';
import { runInAction } from 'mobx';
import { Session } from '../../entity/session';

const OPERATOR = 'Operator';

const opl0Columns = [
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
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
        sorter: true,
    },
    {
        title: 'Start Time(ms)',
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
];
const opl2Columns = [
    ...opl0Columns,
    {
        title: 'Block Dim',
        dataIndex: 'blockDim',
        sorter: true,
    },
    {
        title: 'Input Shapes',
        dataIndex: 'inputShape',
        sorter: true,
    },
    {
        title: 'Input Data Types',
        dataIndex: 'inputType',
        sorter: true,
    },
    {
        title: 'Input Formats',
        dataIndex: 'inputFormat',
        sorter: true,
    },
    {
        title: 'Output Shapes',
        dataIndex: 'outputShape',
        sorter: true,
    },
    {
        title: 'Output Data Types',
        dataIndex: 'outputType',
        sorter: true,
    },
    {
        title: 'Output Formats',
        dataIndex: 'outputFormat',
        sorter: true,
    },
];
const opStaticColumns = [
    {
        title: 'Type',
        dataIndex: 'opType',
        sorter: true,
    },
    {
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
        sorter: true,
    },
    {
        title: 'Count',
        dataIndex: 'count',
        sorter: true,
    },
    {
        title: 'Total Time(μs)',
        dataIndex: 'totalTime',
        sorter: true,
    },
    {
        title: 'Avg Time(μs)',
        dataIndex: 'avgTime',
        sorter: true,
    },
    {
        title: 'Max Time(μs)',
        dataIndex: 'maxTime',
        sorter: true,
    },
    {
        title: 'Min Time(μs)',
        dataIndex: 'minTime',
        sorter: true,
    },
];
const opShapeStaticColumns = [
    {
        title: 'Type',
        dataIndex: 'opType',
        sorter: true,
    },
    {
        title: 'Shape',
        dataIndex: 'inputShape',
        key: 'Shape',
    },
    {
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
        sorter: true,
    },
    {
        title: 'Count',
        dataIndex: 'count',
        sorter: true,
    },
    {
        title: 'Total Time(μs)',
        dataIndex: 'totalTime',
        sorter: true,
    },
    {
        title: 'Avg Time(μs)',
        dataIndex: 'avgTime',
        sorter: true,
    },
    {
        title: 'Max Time(μs)',
        dataIndex: 'maxTime',
        sorter: true,
    },
    {
        title: 'Min Time(μs)',
        dataIndex: 'minTime',
        sorter: true,
    },
];
const colMap: any = {
    Operator: {
        l0: opl0Columns,
        l1: opl2Columns,
        l2: opl2Columns,
    },
    'Operator Type': opStaticColumns,
    'Input Shape': opShapeStaticColumns,
};

const OperatorTable = ({ condition, opType, inputShape, session }:
{condition: ConditionType;opType?: string;inputShape?: string;session: Session}): JSX.Element => {
    return <BaseTable
        condition={{ ...condition, group: OPERATOR }}
        opType={opType}
        inputShape={inputShape}
        session={session}
    />;
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: '' };
// eslint-disable-next-line max-lines-per-function
const BaseTable = ({ condition, opType, inputShape, session }:
{condition: ConditionType;opType?: string;inputShape?: string;session: Session}): JSX.Element => {
    const [ cols, setCols ] = useState<any[]>(opl0Columns);
    const [ page, setPage ] = useState(defaultPage);
    const [ sorter, setSorter ] = useState(defaultSorter);
    const [ data, setData ] = useState<any[]>([]);
    const rowKey = 'rowKey';
    const [ expandedRowKeys, setExpandedKeys ] = useState<string[]>([]);
    const btnCol = {
        title: 'Details',
        key: 'action',
        render: (_: any, record: any) => (<Button type="link"
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
    };

    const getCols = ({ group, level }: any): any[] => {
        if (group === OPERATOR) {
            return colMap[group][level] ?? colMap[group].l2;
        } else {
            return [ ...colMap[group] ?? [], btnCol ];
        }
    };
    const updateData = async(page: any, sorter: any): Promise<void> => {
        let res;
        // 展开算子
        if (opType !== undefined && inputShape !== undefined) {
            res = await queryOperatorsInStatic(
                { ...condition, ...page, order: sorter.order, orderBy: sorter.field, opType, shape: inputShape },
            );
        } else if (condition.group === OPERATOR) {
            res = await queryOperators(
                { ...condition, ...page, order: sorter.order, orderBy: sorter.field });
        } else {
            res = await queryOperatorStatic(
                { ...condition, ...page, order: sorter.order, orderBy: sorter.field });
        }
        if (res === null || res === undefined) {
            return;
        }
        const { data, total, level } = res;
        data.forEach((item: any, index: number) => {
            item.rowKey = condition.group + String(page.pageSize * page.current + index);
        });

        setData(data);
        setPage({ ...page, total });
        const columns = getCols({ group: condition.group, level });
        setCols(columns);
        runInAction(() => {
            session.total = total;
        });
    };

    useEffect(() => {
        updateData(page, sorter);
        setExpandedKeys([]);
    }, [ page.current, page.pageSize, sorter.field, sorter.order, condition ]);
    return <ResizeTable
        size="small"
        minThWidth={50}
        columns={cols}
        dataSource={data}
        pagination={GetPageConfigWhithPageData(page, setPage)}
        onChange={(pagination: any, filters: any, sorter: any, extra: any) => {
            if (extra.action === 'sort') {
                setSorter(sorter);
            }
        }
        }
        rowKey={rowKey}
        expandable={condition.group !== OPERATOR
            ? {
                expandedRowRender: (record: any) => <OperatorTable
                    condition={condition}
                    opType={record.opType}
                    inputShape={record.inputShape}
                    session={session}
                />,
                expandedRowKeys,
                expandIcon: () => (<></>),
            }
            : false}
    />;
};

const DetailTable = ({ condition, session }: {condition: ConditionType;session: Session}): JSX.Element => {
    return <Container
        style={{ height: 'auto' }}
        title={'Detail'}
        content={<BaseTable condition={condition} session={session}/>}
    />;
};

export default DetailTable;
