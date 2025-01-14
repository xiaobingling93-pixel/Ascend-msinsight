/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { ResizeTable } from 'ascend-resize';
import React, { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Button, Tooltip } from 'ascend-components';
import { DownOutlined } from '@ant-design/icons';
import { getPageConfigWithPageData } from '../Common';
import { type ConditionType, type FilterType } from './Filter';
import { queryOperators, queryOperatorsInStatic, queryOperatorStatic } from '../RequestUtils';
import { runInAction } from 'mobx';
import type { Session } from '../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { OperatorGroup, useColMap, useCompareSourceColumn } from '../TableColumnConfig';
import { HelpIcon } from 'ascend-icon';

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
};

interface CompInfo {
    level: string;
    detailData: any[];
}

const OperatorTable = ({ condition, filterType, opType, accCore, opName, inputShape, compInfo, session }:
{condition: ConditionType;filterType: FilterType;opType?: string;accCore?: string;opName?: string;
    inputShape?: string;compInfo?: CompInfo;session: Session;}): JSX.Element => {
    return <BaseTable
        condition={condition}
        filterType={filterType}
        opType={opType}
        accCore={accCore}
        opName={opName}
        inputShape={inputShape}
        compInfo={compInfo}
        session={session}
    />;
};

const OperatorTypeTable = ({ condition, filterType, session }: {condition: ConditionType;filterType: FilterType;session: Session}): JSX.Element => {
    return <BaseTable condition={condition} filterType={filterType} session={session} />;
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: '' };
const defaultFilters = { type: [], opType: [], name: [], opName: [], accCore: [] };

// 字符串替换映射
const replaceMap = new Map([
    ['_us_', '(us)'],
    ['_GB_s_', '(GB/s)'],
    ['_PCT_', '(%)'],
]);

function modifyTitle(item: string): string {
    let modifiedItem = item;
    replaceMap.forEach((value, key) => {
        if (modifiedItem.includes(key)) {
            modifiedItem = modifiedItem.replace(new RegExp(key, 'g'), value);
        }
    });
    return modifiedItem;
}

const getCols = ({ group, columnLevel, btnCol, colMap, condition, isExpend, pmuHeaders }:
{group: string;columnLevel: string;btnCol: any;colMap: any;condition: ConditionType;isExpend: boolean; pmuHeaders: any[]}): any[] => {
    const isCompare = condition.isCompare as boolean;
    let result = [];
    switch (group) {
        case OperatorGroup.OPERATOR: {
            if (isCompare && !isExpend) {
                result = [...colMap[group][columnLevel] ?? colMap[group].l2, btnCol];
                break;
            }
            const columns = colMap[group][columnLevel] ?? colMap[group].l2;
            // pmu数据的表头，后端返回时确定
            if (pmuHeaders === null || pmuHeaders === undefined) {
                result = columns;
                break;
            }
            pmuHeaders.forEach((item: any, index: number) => {
                columns.push({
                    title: modifyTitle(item),
                    dataIndex: index,
                    sorter: false,
                    ellipsis: true,
                });
            });
            result = columns;
            break;
        }
        case OperatorGroup.HCCL_OPERATOR:
            if (isCompare && !isExpend) {
                result = [...colMap[group], btnCol];
                break;
            }
            result = colMap[group];
            break;
        case OperatorGroup.HCCL_OPERATOR_TYPE:
            result = getHcclOperatorTypeCols({ group, columnLevel, btnCol, colMap, isCompare, isExpend });
            break;
        default:
            if (isCompare && isExpend) {
                result = [...colMap[group] ?? []];
                break;
            }
            result = [...colMap[group] ?? [], btnCol];
            break;
    }
    return result;
};

const getHcclOperatorTypeCols = ({ group, columnLevel, btnCol, colMap, isCompare, isExpend }:
{group: string;columnLevel: string;btnCol: any;colMap: any;isCompare: boolean;isExpend: boolean}): any[] => {
    if (columnLevel === undefined) {
        if (isCompare && isExpend) {
            return [...colMap[group].l0 ?? []];
        }
        return [...colMap[group].l0 ?? [], btnCol];
    } else {
        if (isCompare && !isExpend) {
            return [...colMap[group][columnLevel], btnCol];
        }
        return colMap[group][columnLevel];
    }
};

const setFilterTypes = (fullCondition: FullConditionType): string[] => {
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
    return filterTypes;
};

const queryOperatorData = async ({ condition, fullCondition, filterTypes }:
{condition: ConditionType;fullCondition: FullConditionType;filterTypes: string[]}): Promise<any> => {
    let res;
    if (condition.group === OperatorGroup.OPERATOR || condition.group === OperatorGroup.HCCL_OPERATOR) {
        const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes, isCompare: condition.isCompare };
        res = await queryOperators(param);
    } else {
        const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes, isCompare: condition.isCompare };
        res = await queryOperatorStatic(param);
    }
    return res;
};

const queryOperatorDetailData = async ({ fullCondition, filterTypes, opType, opName, accCore, inputShape }:
{fullCondition: FullConditionType;filterTypes: string[];opType?: string;opName?: string;accCore?: string;inputShape?: string}): Promise<any> => {
    const param = { ...fullCondition, orderBy: fullCondition.field, filters: filterTypes, opType: opType ?? '', opName, shape: inputShape ?? '', accCore: accCore ?? '' };
    return await queryOperatorsInStatic(param);
};

const handleOrginData = (group: string, pageSize: number, current: number, data: any[]): any[] => {
    const realData: any[] = [];
    data.forEach((item: any, index: number) => {
        if (item.compare !== null && item.compare !== undefined) {
            const diff = 'diff';
            item.compare.rowKey = group + String((pageSize * current) + index) + diff;
            realData.push(item.compare);
        } else {
            item.rowKey = group + String((pageSize * current) + index);
            realData.push(item);
        };
    });
    return realData;
};

const handleDiffData = (group: string, pageSize: number, current: number, data: any[], t: TFunction): any => {
    const realData: any[] = [];
    data.forEach((item: any, index: number) => {
        if (item.diff !== null && item.diff !== undefined) {
            item.diff.rowKey = group + String((pageSize * current) + index);
            item.diff.source = t('operator:Difference');
            item.diff.compInfo = [item.baseline, item.compare];
            realData.push(item.diff);
        };
    });
    return realData;
};

const handleCompareData = (data: any, t: TFunction): any[] => {
    if (data.length === 0) {
        return data;
    }
    data[0].source = t('operator:Baseline');
    data[1].source = t('operator:Comparison');
    return [data[0], data[1]];
};

// eslint-disable-next-line max-lines-per-function
const BaseTable = ({ condition, filterType, opType, accCore, opName, inputShape, compInfo, session }:
{condition: ConditionType;filterType: FilterType;opType?: string;accCore?: string;
    opName?: string;inputShape?: string;compInfo?: CompInfo;session: Session;}): JSX.Element => {
    const isCompare = condition.isCompare as boolean;
    const { t } = useTranslation();
    const [cols, setCols] = useState<any[]>(useColMap(isCompare)[OperatorGroup.OPERATOR].l0);
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
    const [compareColumnLevel, setCompareColumnLevel] = useState<string>();
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
    const colMap = useColMap(isCompare && compInfo === undefined);
    const compareSourceCol = useCompareSourceColumn();
    const updateData = async(): Promise<void> => {
        let isExpend = false;
        const filterTypes = setFilterTypes(fullCondition);
        let res;
        if (opType !== undefined || opName !== undefined || accCore !== undefined) {
            // 展开算子
            isExpend = true;
            if (isCompare) {
                res = { data: compInfo?.detailData ?? [], total: 2, level: compInfo?.level };
            } else {
                res = await queryOperatorDetailData({ fullCondition, filterTypes, opType, opName, accCore, inputShape });
            }
        } else {
            res = await queryOperatorData({ condition, fullCondition, filterTypes });
        };
        if (res === null || res === undefined) {
            return;
        }
        const { pmuHeaders, data, total, level } = res;
        let realData = [];
        if (isCompare) {
            if (isExpend) {
                realData = handleCompareData(data, t);
            } else {
                realData = handleDiffData(fullCondition.group, fullCondition.pageSize, fullCondition.current, data, t);
                setCompareColumnLevel(level);
            }
        } else {
            realData = handleOrginData(fullCondition.group, fullCondition.pageSize, fullCondition.current, data);
        }
        setTableData(realData);
        setPage({ ...page, total });
        let group = opType !== undefined && !isCompare ? OperatorGroup.OPERATOR : condition.group;
        let columnLevel = level;
        if (condition.group === OperatorGroup.HCCL_OPERATOR_TYPE && isExpend && !isCompare) {
            group = OperatorGroup.HCCL_OPERATOR_TYPE;
            columnLevel = 'l1';
        }
        const columns = getCols({ group, columnLevel, btnCol, colMap, condition, isExpend, pmuHeaders });
        if (isCompare) {
            columns.splice(1, 0, compareSourceCol[0]);
        }
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
    }, [JSON.stringify(fullCondition), condition.isCompare, t]);

    useEffect(() => {
        setSorter(defaultSorter);
        setPage(defaultPage);
        setFilters(defaultFilters);
        updateFullCondition({ ...defaultSorter, ...defaultPage, ...defaultFilters, ...condition });
    }, [condition.group]);

    useEffect(() => {
        if (page.current * page.pageSize > page.total) {
            page.current = parseInt((page.total / page.pageSize).toString()) + 1;
        }
        updateFullCondition({ ...sorter, ...page, ...filters, ...condition });
    }, [page.current, page.pageSize, page.total, sorter.field, sorter.order,
        filters.type, filters.opType, filters.name, filters.opName, filters.accCore,
        condition.rankId, condition.topK]);

    return <ResizeTable
        size="small"
        minThWidth={50}
        loading={loading}
        columns={cols}
        dataSource={tableData}
        scroll={{
            x: 100 * cols.length,
        }}
        pagination={isCompare && compInfo !== undefined ? false : getPageConfigWithPageData(page, setPage)}
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
        expandable={isCompare || (condition.group !== OperatorGroup.OPERATOR && condition.group !== OperatorGroup.HCCL_OPERATOR)
            ? {
                expandedRowRender: (record: any) => <OperatorTable
                    condition={condition}
                    filterType={filterType}
                    opName={record.opName}
                    opType={record.opType}
                    inputShape={record.inputShape}
                    accCore={record.accCore}
                    compInfo={
                        isCompare
                            ? {
                                level: compareColumnLevel,
                                detailData: record.compInfo ?? [],
                            } as CompInfo
                            : undefined
                    }
                    session={session}
                />,
                expandedRowKeys,
                expandIcon: () => (<></>),
            }
            : undefined}
    />;
};

export const useHit = (condition: ConditionType): React.ReactElement => {
    const { t } = useTranslation('operator');
    if (condition.isCompare || condition.group !== OperatorGroup.OPERATOR) {
        return (<></>);
    }
    const hit = t('Operator/ComputeOperatorDetailDescribe',
        { returnObjects: true }) as string[];
    return (<Tooltip
        overlayClassName={'width-auto'}
        title={
            (
                <div style={{ padding: '1rem' }}>
                    {hit?.map((item: string, index: number) => <div key={index}>{item}</div>)}
                </div>
            )
        }>
        <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20}/>
    </Tooltip>);
};

const DetailTable = ({ condition, filterType, session }: {condition: ConditionType;filterType: FilterType;session: Session}): JSX.Element => {
    const { t } = useTranslation('operator');
    let table;
    switch (condition.group) {
        case OperatorGroup.OPERATOR:
        case OperatorGroup.HCCL_OPERATOR:
            table = <OperatorTable condition={condition} filterType={filterType} session={session}/>;
            break;
        case OperatorGroup.OPERATOR_TYPE:
        case OperatorGroup.HCCL_OPERATOR_TYPE:
            table = <OperatorTypeTable condition={condition} filterType={filterType} session={session}/>;
            break;
        default:
            table = <BaseTable condition={condition} filterType={filterType} session={session}/>;
            break;
    }
    return <CollapsiblePanel title={<div className={'flex items-center'}>{t('sessionTitle.OperatorDetails')}{useHit(condition)}</div>} secondary>
        {table}
    </CollapsiblePanel>;
};

export default DetailTable;
