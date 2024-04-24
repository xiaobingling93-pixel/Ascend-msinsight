/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
// Module Federation组件
// eslint-disable-next-line import/no-unresolved
import ResizeTable from 'lib/ResizeTable';
import React, { useState, useEffect } from 'react';
import { Button } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import { Container, GetPageConfigWhithPageData } from '../Common';
import { type ConditionType } from './Filter';
import { queryOperators, queryOperatorsInStatic, queryOperatorStatic } from '../RequestUtils';
import { runInAction } from 'mobx';
import { Session } from '../../entity/session';
import type { ColumnsType } from 'antd/es/table';

interface FullConditionType {
    rankId: string ;
    group: string;
    topK: number;
    current: number;
    pageSize: number;
    field: string;
    order: string;
};
const OPERATOR = 'Operator';
const OPERATOR_TYPE = 'Operator Type';
const opl0Columns: ColumnsType<any> = [
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
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
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
const opl2Columns = [
    ...opl0Columns,
    {
        title: 'Block Dim',
        dataIndex: 'blockDim',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Shapes',
        dataIndex: 'inputShape',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Data Types',
        dataIndex: 'inputType',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Input Formats',
        dataIndex: 'inputFormat',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Shapes',
        dataIndex: 'outputShape',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Data Types',
        dataIndex: 'outputType',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Output Formats',
        dataIndex: 'outputFormat',
        sorter: true,
        ellipsis: true,
    },
];
const opStaticColumns = [
    {
        title: 'Type',
        dataIndex: 'opType',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Count',
        dataIndex: 'count',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Total Time(μs)',
        dataIndex: 'totalTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Avg Time(μs)',
        dataIndex: 'avgTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Max Time(μs)',
        dataIndex: 'maxTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Min Time(μs)',
        dataIndex: 'minTime',
        sorter: true,
        ellipsis: true,
    },
];
const opShapeStaticColumns = [
    {
        title: 'Name',
        dataIndex: 'opName',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Shape',
        dataIndex: 'inputShape',
        key: 'Shape',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Accelerator Core',
        dataIndex: 'accCore',
        key: 'accCore',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Count',
        dataIndex: 'count',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Total Time(μs)',
        dataIndex: 'totalTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Avg Time(μs)',
        dataIndex: 'avgTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Max Time(μs)',
        dataIndex: 'maxTime',
        sorter: true,
        ellipsis: true,
    },
    {
        title: 'Min Time(μs)',
        dataIndex: 'minTime',
        sorter: true,
        ellipsis: true,
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

const OperatorTable = ({ condition, opType, accCore, opName, inputShape, session }:
{condition: ConditionType;opType?: string;accCore?: string;opName?: string;inputShape?: string;session: Session}): JSX.Element => {
    return <BaseTable
        condition={condition}
        opType={opType}
        accCore={accCore}
        opName={opName}
        inputShape={inputShape}
        session={session}
    />;
};

const OperatorTypeTable = ({ condition, session }: {condition: ConditionType;session: Session}): JSX.Element => {
    return <BaseTable condition={condition} session={session} />;
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: '' };

// eslint-disable-next-line max-lines-per-function
const BaseTable = ({ condition, opType, accCore, opName, inputShape, session }:
{condition: ConditionType;opType?: string;accCore?: string;opName?: string;inputShape?: string;session: Session}): JSX.Element => {
    const [cols, setCols] = useState<any[]>(opl0Columns);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [data, setData] = useState<any[]>([]);
    const rowKey = 'rowKey';
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const [loading, setLoading] = useState(false);
    const [fullCondition, setFullCondition] = useState<FullConditionType>({ current: 1, pageSize: 10, field: '', order: '', group: '', rankId: '', topK: 0 });
    const btnCol = {
        title: 'Details',
        width: 115,
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
            return [...colMap[group] ?? [], btnCol];
        }
    };
    const updateData = async(): Promise<void> => {
        let res;
        // 展开算子
        if (opType !== undefined || opName !== undefined || accCore !== undefined) {
            res = await queryOperatorsInStatic(
                { ...fullCondition, orderBy: fullCondition.field, opType: opType ?? '', opName, shape: inputShape ?? '', accCore: accCore ?? '' },
            );
        } else if (condition.group === OPERATOR) {
            res = await queryOperators(
                { ...fullCondition, orderBy: fullCondition.field });
        } else {
            res = await queryOperatorStatic(
                { ...fullCondition, orderBy: fullCondition.field });
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
        const columns = getCols({ group: opType !== undefined ? OPERATOR : condition.group, level });
        setCols(columns);
        runInAction(() => {
            session.total = total;
        });
    };

    const updateTable = async (): Promise<void> => {
        setLoading(true);
        try {
            await updateData();
        } finally {
            setExpandedKeys([]);
            setLoading(false);
        }
    };

    const updateFullCondition = (obj: FullConditionType): void => {
        setTimeout(() => {
            const newFullCondition = { ...fullCondition };
            const keys = ['group', 'rankId', 'topK', 'current', 'pageSize', 'field', 'order'];
            Object.keys(obj).forEach(key => {
                if (keys.includes(key)) {
                    Object.assign(newFullCondition, { [key]: obj[key as keyof FullConditionType] });
                }
            });
            setFullCondition(newFullCondition);
        });
    };

    useEffect(() => {
        if (condition.rankId === '') {
            setData([]);
            runInAction(() => {
                session.total = 0;
            });
            return;
        }
        updateTable();
    }, [JSON.stringify(fullCondition)]);

    useEffect(() => {
        setSorter(defaultSorter);
        setPage(defaultPage);
        updateFullCondition({ ...defaultSorter, ...defaultPage, ...condition });
    }, [condition.group]);

    useEffect(() => {
        updateFullCondition({ ...sorter, ...page, ...condition });
    }, [page.current, page.pageSize, sorter.field, sorter.order, condition.rankId, condition.topK]);
    return <ResizeTable
        size="small"
        minThWidth={50}
        loading={loading}
        columns={cols}
        dataSource={data}
        pagination={GetPageConfigWhithPageData(page, setPage)}
        onChange={(pagination: any, filters: any, sorter: any, extra: any) => {
            if (extra.action === 'sort') {
                setSorter(sorter.order === undefined ? { field: '', order: '' } : sorter);
            }
        }
        }
        rowKey={rowKey}
        expandable={condition.group !== OPERATOR
            ? {
                expandedRowRender: (record: any) => <OperatorTable
                    condition={condition}
                    opName={record.opName}
                    opType={record.opType}
                    inputShape={record.inputShape}
                    accCore={record.accCore}
                    session={session}
                />,
                expandedRowKeys,
                expandIcon: () => (<></>),
            }
            : false}
    />;
};

const DetailTable = ({ condition, session }: {condition: ConditionType;session: Session}): JSX.Element => {
    let table;
    switch (condition.group) {
        case OPERATOR:
            table = <OperatorTable condition={condition} session={session}/>;
            break;
        case OPERATOR_TYPE:
            table = <OperatorTypeTable condition={condition} session={session}/>;
            break;
        default:
            table = <BaseTable condition={condition} session={session}/>;
            break;
    }
    return <Container
        style={{ height: 'auto' }}
        title={'Operator Detail'}
        bodyStyle={{ overflow: 'visible' }}
        content={table}
    />;
};

export default DetailTable;
