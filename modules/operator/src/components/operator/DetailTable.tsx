/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import ResizeTable from 'lib/ResizeTable';
import { fetchColumnFilterProps } from 'lib/ColumnFilter';
import React, { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { Button } from 'lib/components';
import { DownOutlined } from '@ant-design/icons';
import { getPageConfigWithPageData } from '../Common';
import { type ConditionType, type FilterType } from './Filter';
import { queryOperators, queryOperatorsInStatic, queryOperatorStatic } from '../RequestUtils';
import { runInAction } from 'mobx';
import type { Session } from '../../entity/session';
import type { ColumnsType } from 'antd/es/table';
import i18n from 'lib/i18n';
import CollapsiblePanel from 'lib/CollapsiblePanel';

interface FullConditionType {
    rankId: string ;
    group: string;
    topK: number;
    current: number;
    pageSize: number;
    field: string;
    order: string;
    type: string[];
    opType: string[];
    name: string[];
    opName: string[];
    accCore: string[];
}
const OPERATOR = 'Operator';
const OPERATOR_TYPE = 'Operator Type';
const INPUT_SHAPE = 'Input Shape';
const HCCL_OPERATOR = 'HCCL Operator';
const HCCL_OPERATOR_TYPE = 'HCCL Operator Type';
const useOpl0Columns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        {
            title: t('Name'),
            dataIndex: 'name',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('name', 'Name', i18n.t),
        },
        {
            title: t('Type'),
            dataIndex: 'type',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('type', 'Type', i18n.t),
        },
        {
            title: t('AcceleratorCore'),
            dataIndex: 'accCore',
            key: 'accCore',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('accCore', 'AcceleratorCore', i18n.t),
        },
        {
            title: `${t('StartTime')}(ms)`,
            dataIndex: 'startTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('Duration')}(μs)`,
            dataIndex: 'duration',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('WaitTime')}(μs)`,
            dataIndex: 'waitTime',
            sorter: true,
            ellipsis: true,
        },
    ];
};
const useOpl2Columns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        ...useOpl0Columns(),
        {
            title: t('BlockDim'),
            dataIndex: 'blockDim',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('InputShapes'),
            dataIndex: 'inputShape',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('InputDataTypes'),
            dataIndex: 'inputType',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('InputFormats'),
            dataIndex: 'inputFormat',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputShapes'),
            dataIndex: 'outputShape',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputDataTypes'),
            dataIndex: 'outputType',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputFormats'),
            dataIndex: 'outputFormat',
            sorter: true,
            ellipsis: true,
        },
    ];
};
const useOpStaticColumns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        {
            title: t('Type'),
            dataIndex: 'opType',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('opType', 'Type'),
        },
        {
            title: t('AcceleratorCore'),
            dataIndex: 'accCore',
            key: 'accCore',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('accCore', 'AcceleratorCore'),
        },
        {
            title: t('Count'),
            dataIndex: 'count',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('TotalTime')}(μs)`,
            dataIndex: 'totalTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('AvgTime')}(μs)`,
            dataIndex: 'avgTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MaxTime')}(μs)`,
            dataIndex: 'maxTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MinTime')}(μs)`,
            dataIndex: 'minTime',
            sorter: true,
            ellipsis: true,
        },
    ];
};
const useOpShapeStaticColumns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        { title: t('Name'), dataIndex: 'opName', sorter: true, ellipsis: true, ...fetchColumnFilterProps('opName', 'Name') },
        { title: t('Shape'), dataIndex: 'inputShape', key: 'Shape', sorter: true, ellipsis: true },
        {
            title: t('AcceleratorCore'),
            dataIndex: 'accCore',
            key: 'accCore',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('accCore', 'AcceleratorCore'),
        },
        {
            title: t('Count'),
            dataIndex: 'count',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('TotalTime')}(μs)`,
            dataIndex: 'totalTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('AvgTime')}(μs)`,
            dataIndex: 'avgTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MaxTime')}(μs)`,
            dataIndex: 'maxTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MinTime')}(μs)`,
            dataIndex: 'minTime',
            sorter: true,
            ellipsis: true,
        },
    ];
};
const useHcclOpColumns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        {
            title: t('Name'),
            dataIndex: 'name',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('name', 'Name'),
        },
        {
            title: t('Type'),
            dataIndex: 'type',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('type', 'Type'),
        },
        {
            title: `${t('StartTime')}(ms)`,
            dataIndex: 'startTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('Duration')}(μs)`,
            dataIndex: 'duration',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('WaitTime')}(μs)`,
            dataIndex: 'waitTime',
            sorter: true,
            ellipsis: true,
        },
    ];
};

const useHcclOpTypeColumns = (): ColumnsType<any> => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        {
            title: t('Type'),
            dataIndex: 'opType',
            sorter: true,
            ellipsis: true,
            ...fetchColumnFilterProps('opType', 'Type'),
        },
        {
            title: t('Count'),
            dataIndex: 'count',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('TotalTime')}(μs)`,
            dataIndex: 'totalTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('AvgTime')}(μs)`,
            dataIndex: 'avgTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MaxTime')}(μs)`,
            dataIndex: 'maxTime',
            sorter: true,
            ellipsis: true,
        },
        {
            title: `${t('MinTime')}(μs)`,
            dataIndex: 'minTime',
            sorter: true,
            ellipsis: true,
        },
    ];
};

const useColMap = (): any => {
    const opl0Columns = useOpl0Columns();
    const opl2Columns = useOpl2Columns();
    const opStaticColumns = useOpStaticColumns();
    const opShapeStaticColumns = useOpShapeStaticColumns();
    const hcclOpColumns = useHcclOpColumns();
    const hcclOpTypeColumns = useHcclOpTypeColumns();

    return {
        [OPERATOR]: {
            l0: opl0Columns,
            l1: opl2Columns,
            l2: opl2Columns,
        },
        [OPERATOR_TYPE]: opStaticColumns,
        [INPUT_SHAPE]: opShapeStaticColumns,
        [HCCL_OPERATOR]: hcclOpColumns,
        [HCCL_OPERATOR_TYPE]: {
            l0: hcclOpTypeColumns,
            l1: hcclOpColumns,
        },
    };
};

const OperatorTable = ({ condition, filterType, opType, accCore, opName, inputShape, session }:
{condition: ConditionType;filterType: FilterType;opType?: string;accCore?: string;opName?: string;inputShape?: string;session: Session}): JSX.Element => {
    return <BaseTable
        condition={condition}
        filterType={filterType}
        opType={opType}
        accCore={accCore}
        opName={opName}
        inputShape={inputShape}
        session={session}
    />;
};

const OperatorTypeTable = ({ condition, filterType, session }: {condition: ConditionType;filterType: FilterType;session: Session}): JSX.Element => {
    return <BaseTable condition={condition} filterType={filterType} session={session} />;
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: '' };
const defaultFilters = { type: [], opType: [], name: [], opName: [], accCore: [] };

// eslint-disable-next-line max-lines-per-function
const BaseTable = ({ condition, filterType, opType, accCore, opName, inputShape, session }:
{condition: ConditionType;filterType: FilterType;opType?: string;accCore?: string;opName?: string;inputShape?: string;session: Session}): JSX.Element => {
    const { t } = useTranslation();
    const [cols, setCols] = useState<any[]>(useOpl0Columns());
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [filters, setFilters] = useState(defaultFilters);
    const [tableData, setTableData] = useState<any[]>([]);
    const rowKey = 'rowKey';
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const [loading, setLoading] = useState(false);
    const [fullCondition, setFullCondition] = useState<FullConditionType>({
        current: 1, pageSize: 10, field: '', order: '', group: '', rankId: '', topK: 0, type: [], opType: [], name: [], opName: [], accCore: [],
    });
    const btnCol = {
        title: t('Details'),
        width: 115,
        key: 'action',
        render: (_: any, record: any) => (<Button type="link"
            onClick={(): void => {
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
            }}>{t('SeeMore', { ns: 'buttonText' })}<DownOutlined/></Button>),
    };
    const colMap = useColMap();

    const getCols = ({ group, columnLevel }: any): any[] => {
        switch (group) {
            case OPERATOR:
                return colMap[group][columnLevel] ?? colMap[group].l2;
            case HCCL_OPERATOR:
                return colMap[group];
            case HCCL_OPERATOR_TYPE:
                if (columnLevel === undefined) {
                    return [...colMap[group].l0 ?? [], btnCol];
                }
                return colMap[group][columnLevel];
            default:
                return [...colMap[group] ?? [], btnCol];
        }
    };
    const updateData = async(): Promise<void> => {
        let res;
        let isExpend = false;
        const filterColumn = ['type', 'opType', 'name', 'opName', 'accCore'];
        const filterTypes: string[] = [];
        Object.keys(fullCondition).forEach(key => {
            const filterValue = fullCondition[key as keyof FullConditionType];
            if (filterColumn.includes(key) && filterValue != null) {
                if (Array.isArray((filterValue)) && filterValue.length > 0) {
                    filterTypes.push(JSON.stringify({ columnName: key, value: filterValue[0] }));
                }
            }
        });
        // 展开算子
        if (opType !== undefined || opName !== undefined || accCore !== undefined) {
            const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes, opType: opType ?? '', opName, shape: inputShape ?? '', accCore: accCore ?? '' };
            isExpend = true;
            res = await queryOperatorsInStatic(param);
        } else if (condition.group === OPERATOR || condition.group === HCCL_OPERATOR) {
            const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes };
            res = await queryOperators(param);
        } else {
            const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes };
            res = await queryOperatorStatic(param);
        }
        if (res === null || res === undefined) {
            return;
        }
        const { data, total, level } = res;
        data.forEach((item: any, index: number) => {
            item.rowKey = condition.group + String((page.pageSize * page.current) + index);
        });
        setTableData(data);
        setPage({ ...page, total });
        let group = opType !== undefined ? OPERATOR : condition.group;
        let columnLevel = level;
        if (condition.group === HCCL_OPERATOR_TYPE && isExpend) {
            group = HCCL_OPERATOR_TYPE;
            columnLevel = 'l1';
        }
        const columns = getCols({ group, columnLevel });
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
            const keys = ['group', 'rankId', 'topK', 'current', 'pageSize',
                'field', 'order', 'type', 'opType', 'name', 'opName', 'accCore'];
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
            setTableData([]);
            runInAction(() => {
                session.total = 0;
            });
            return;
        }
        updateTable();
    }, [JSON.stringify(fullCondition), t]);

    useEffect(() => {
        setSorter(defaultSorter);
        setPage(defaultPage);
        setFilters(defaultFilters);
        updateFullCondition({ ...defaultSorter, ...defaultPage, ...defaultFilters, ...condition });
    }, [condition.group]);

    useEffect(() => {
        updateFullCondition({ ...sorter, ...page, ...filters, ...condition });
    }, [page.current, page.pageSize, sorter.field, sorter.order,
        filters.type, filters.opType, filters.name, filters.opName, filters.accCore,
        condition.rankId, condition.topK]);
    return <ResizeTable
        size="small"
        minThWidth={50}
        loading={loading}
        columns={cols}
        dataSource={tableData}
        pagination={getPageConfigWithPageData(page, setPage)}
        onChange={(pagination: any, newFilters: any, newSorter: any, extra: any): void => {
            if (extra.action === 'sort') {
                setSorter(newSorter.order === undefined ? { field: '', order: '' } : newSorter);
            }
            if (extra.action === 'filter') {
                setFilters(newFilters === undefined ? {} : newFilters);
            }
        }
        }
        rowKey={rowKey}
        expandable={condition.group !== OPERATOR && condition.group !== HCCL_OPERATOR
            ? {
                expandedRowRender: (record: any) => <OperatorTable
                    condition={condition}
                    filterType={filterType}
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

const DetailTable = ({ condition, filterType, session }: {condition: ConditionType;filterType: FilterType;session: Session}): JSX.Element => {
    const { t } = useTranslation('operator');
    let table;
    switch (condition.group) {
        case OPERATOR:
        case HCCL_OPERATOR:
            table = <OperatorTable condition={condition} filterType={filterType} session={session}/>;
            break;
        case OPERATOR_TYPE:
        case HCCL_OPERATOR_TYPE:
            table = <OperatorTypeTable condition={condition} filterType={filterType} session={session}/>;
            break;
        default:
            table = <BaseTable condition={condition} filterType={filterType} session={session}/>;
            break;
    }
    return <CollapsiblePanel title={t('sessionTitle.OperatorDetails')} secondary>
        {table}
    </CollapsiblePanel>;
};

export default DetailTable;
