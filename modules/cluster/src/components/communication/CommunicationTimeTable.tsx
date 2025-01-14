/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useMemo, useState } from 'react';
import type { CSSProperties } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Button } from 'ascend-components';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import type { ColumnsType } from 'antd/es/table';
import { getPageConfigWithAllData, getPageConfigWithPageData } from '../Common';
import type { VoidFunction } from '../../utils/interface';
import { queryOperatorDetails } from '../../utils/RequestUtils';
import { type ConditionDataType, totalOperator } from './Filter';
import { ResizeTable } from 'ascend-resize';
import type { Session } from '../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { CaretDownIcon, CaretRightIcon } from 'ascend-icon';
import { CompareNumber } from 'ascend-utils';
import i18n from 'ascend-i18n';
import type { DataItem, Duration } from './CommunicationTimeChart';

export type DataType = Duration & {
    rankId: string;
    expanded: boolean;
    source: Source;
    index: number;
};

type TableDataItem = DataItem & {
    expanded: boolean;
    source: Source;
};

const useCommonColumns = (): ColumnsType<DataType> => {
    const { t } = useTranslation('communication');
    return [
        { title: `${t('tableHead.Start Time')}(ms)`, dataIndex: 'startTime', sorter: (a: DataType, b: DataType) => a.startTime - b.startTime, ellipsis: true },
        { title: `${t('tableHead.Elapse Time')}(ms)`, dataIndex: 'elapseTime', sorter: (a: DataType, b: DataType) => a.elapseTime - b.elapseTime, ellipsis: true, showSorterTooltip: { title: `${t('tableHeadTooltip.Elapse Time')}` } },
        {
            title: `${t('tableHead.Transit Time')}(ms)`,
            dataIndex: 'transitTime',
            sorter: (a: DataType, b: DataType) => a.transitTime - b.transitTime,
            ellipsis: true,
            showSorterTooltip: { title: `${t('tableHeadTooltip.Transit Time')}` },
        },
        {
            title: `${t('tableHead.Synchronization Time')}(ms)`,
            dataIndex: 'synchronizationTime',
            sorter: (a: DataType, b: DataType) => a.synchronizationTime - b.synchronizationTime,
            ellipsis: true,
            showSorterTooltip: { title: `${t('tableHeadTooltip.Synchronization Time')}` },
        },
        {
            title: `${t('tableHead.Wait Time')}(ms)`,
            dataIndex: 'waitTime',
            sorter: (a: DataType, b: DataType) => a.waitTime - b.waitTime,
            ellipsis: true,
            showSorterTooltip: { title: `${t('tableHeadTooltip.Wait Time')}` },
        },
        {
            title: `${t('tableHead.Synchronization Time Ratio')}`,
            dataIndex: 'synchronizationTimeRatio',
            sorter: (a: DataType, b: DataType) => a.synchronizationTimeRatio - b.synchronizationTimeRatio,
            ellipsis: true,
            showSorterTooltip: { title: `${t('tableHeadTooltip.Synchronization Time Ratio')}` },
        },
        {
            title: `${t('tableHead.Wait Time Ratio')}`,
            dataIndex: 'waitTimeRatio',
            sorter: (a: DataType, b: DataType) => a.waitTimeRatio - b.waitTimeRatio,
            ellipsis: true,
            showSorterTooltip: { title: `${t('tableHeadTooltip.Wait Time Ratio')}` },
        },
        {
            title: `${t('tableHead.Idle Time')}(ms)`, dataIndex: 'idleTime', sorter: (a: DataType, b: DataType) => a.idleTime - b.idleTime, ellipsis: true, showSorterTooltip: { title: `${t('tableHeadTooltip.Idle Time')}` },
        },
        {
            title: `${t('tableHead.SDMABW')}(GB)`, dataIndex: 'sdmaBw', sorter: (a: DataType, b: DataType) => a.sdmaBw - b.sdmaBw, ellipsis: true,
        },
        {
            title: `${t('tableHead.RDMABW')}(GB)`, dataIndex: 'rdmaBw', sorter: (a: DataType, b: DataType) => a.rdmaBw - b.rdmaBw, ellipsis: true,
        }];
};
// Total HCCL Opertators表
const OperatorsTable = ({ record, conditions }: any): JSX.Element => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'elapseTime', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const { t } = useTranslation('communication');

    useEffect(() => {
        updateData(page, sorter);
    }, [page.current, page.pageSize, sorter.field, sorter.order, conditions.iterationId, record.rankId]);
    const updateData = async(_page: any, _sorter: {field: string;order: string}): Promise<void> => {
        const res = await queryOperatorDetails({
            iterationId: record.source === Source.COMPARISON ? conditions.iterationId : conditions.baselineIterationId,
            rankId: record.rankId,
            currentPage: _page.current,
            pageSize: _page.pageSize,
            orderBy: _sorter.field,
            order: _sorter.order,
            stage: conditions.stage,
            queryType: record.source,
        });
        setDataSource(res?.allOperators ?? []);
        setPage({ ..._page, total: res?.count ?? 0 });
    };

    const columns: TableColumnsType<DataType> = [
        { title: t('tableHead.Operator Name'), dataIndex: 'operatorName', key: 'operatorName', sorter: true, ellipsis: true },
        ...useCommonColumns().map(item => {
            return { ...item, sorter: true };
        }),
    ];
    return <div>
        <ResizeTable columns={columns} dataSource={dataSource} size="small"
            pagination={getPageConfigWithPageData(page, setPage)}
            onChange={(pagination: any, filters: any, newSorter: any, extra: any): void => {
                if (extra.action === 'sort') {
                    setSorter(newSorter);
                }
            }}
        />
    </div>;
};

const ExpandIcon = ({ expanded, onClick }: {expanded: boolean;onClick: VoidFunction}): JSX.Element => {
    const iconProps = {
        onClick,
        style: { cursor: 'pointer', float: 'left', marginRight: '5px' } as CSSProperties,
    };
    return expanded
        ? <CaretDownIcon {...iconProps}/>
        : <CaretRightIcon {...iconProps}/>;
};
const commonColumnConfig = {
    ellipsis: true,
    width: 110,
    minWidth: 100,
};
const useRankColumns = (handleAction: VoidFunction[], conditions: ConditionDataType, t: TFunction, tableLevel: TableLevel): any => {
    const [showOperator, handleExpand] = handleAction;
    return [
        {
            title: t('tableHead.Rank ID'),
            ellipsis: true,
            width: 70,
            dataIndex: 'rankId',
            key: 'rankId',
            sorter: (a: DataType, b: DataType) => Number(a.rankId) - Number(b.rankId),
            render: (_: any, record: DataType): React.ReactNode => (<div>
                {conditions.operatorName === totalOperator && <ExpandIcon expanded={record.expanded} onClick={(): void => { handleExpand(record); }}/>}
                {record.rankId}</div>),
            display: tableLevel !== TableLevel.DIFF_SOURCE,
        },
        {
            title: t('tableHead.Source'),
            ...commonColumnConfig,
            render: (data: DataType): React.ReactNode => (<div>
                {tableLevel === TableLevel.DIFF_SOURCE && <ExpandIcon expanded={data.expanded} onClick={(): void => { handleExpand(data); }}/>}
                {i18n.t(data.source)} </div>),
            display: [TableLevel.DIFF, TableLevel.DIFF_SOURCE].includes(tableLevel),
        },
        ...useCommonColumns().map(commonCol => ({
            ...commonCol,
            render: (data: string | number) => tableLevel === TableLevel.DIFF ? <CompareNumber data={data}/> : data,
        })),
        {
            title: t('tableHead.Bandwidth Analysis'),
            ...commonColumnConfig,
            render: (_: any, record: DataType) => (<Button type="link" onClick={(): void => { showOperator(record.rankId); }}>{t('tableHead.see more')}</Button>),
            display: tableLevel === TableLevel.RANK,
        },
        {
            title: t('tableHead.Communication Operators Details'),
            ...commonColumnConfig,
            render: (_: any, record: DataType) => (<Button type="link" onClick={(): void => { handleExpand(record); }}>{t('tableHead.see more')}<DownOutlined/></Button>),
            display: conditions.operatorName === totalOperator && tableLevel === TableLevel.RANK,
        },
        {
            title: t('tableHead.Details'),
            ...commonColumnConfig,
            render: (_: any, record: DataType) => (<Button type="link" onClick={(): void => { handleExpand(record); }}>{t('tableHead.see more')}<DownOutlined/></Button>),
            display: [TableLevel.DIFF, TableLevel.DIFF_SOURCE].includes(tableLevel),
        },
    ].filter((item: any) => item.display !== false);
};

enum TableLevel {
    RANK = 0,
    OPERATOR = 1,
    DIFF = 2,
    DIFF_SOURCE = 3,
}
enum Source {
    DIFFERENCE = 'Difference',
    COMPARISON = 'Comparison',
    BASELINE = 'Baseline',
}

interface IProps {
    dataSource: DataItem[];
    showOperator: (rankid: string) => void;
    conditions: ConditionDataType;
    updateSort?: VoidFunction;
    session: Session;
    level?: TableLevel;
}
const rowKey = 'index';
const CommunicationTimeTable = observer(({ dataSource, showOperator, conditions, session, updateSort, level }: IProps) => {
    const { t } = useTranslation('communication');
    const [expandedRowKeys, setExpandedKeys] = useState<React.Key[]>([]);
    const handleExpand = (record: DataType): void => {
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
    };

    // 表类型：卡、对比差值、对比详情
    const tableLevel = useMemo(() => {
        if (level !== undefined && level !== null) {
            return level;
        }
        return session.isCompare ? TableLevel.DIFF : TableLevel.RANK;
    }, [session.isCompare, level]);

    const tableData: TableDataItem[] = useMemo(() => {
        if (tableLevel === TableLevel.DIFF_SOURCE) {
            return dataSource.reduce<TableDataItem[]>((pre, cur) => {
                const { compareData } = cur;
                pre.push({ ...cur, ...compareData.compare, source: Source.COMPARISON, index: 0, expanded: expandedRowKeys.includes(0) },
                    { ...cur, ...compareData.baseline, source: Source.BASELINE, index: 1, expanded: expandedRowKeys.includes(1) });
                return pre;
            }, []);
        }
        return dataSource.map((data, index) => {
            const { compareData } = data;
            const record = tableLevel === TableLevel.DIFF ? (compareData.diff ?? {}) : compareData.compare;
            return ({
                ...data,
                ...record,
                index,
                source: tableLevel === TableLevel.DIFF ? Source.DIFFERENCE : Source.COMPARISON,
                expanded: expandedRowKeys.includes(index),
            });
        });
    }, [dataSource, expandedRowKeys, session.isCompare, tableLevel]);

    const columns = useMemo(() => useRankColumns([showOperator, handleExpand], conditions, t, tableLevel),
        [handleExpand, tableLevel, conditions, t]);

    const nextLevelTable = (record: TableDataItem): JSX.Element => (<div style={{ marginLeft: '0' }}>{
        tableLevel === TableLevel.DIFF
            ? <CommunicationTimeTable level={TableLevel.DIFF_SOURCE} dataSource={[record]} {...{ showOperator, conditions, session }} />
            : <OperatorsTable record={record} conditions={conditions}/>
    }</div>);

    // 数据变动，清理折叠展开
    useEffect(() => {
        setExpandedKeys([]);
    }, [dataSource]);

    return <ResizeTable
        data-testid={'dataAnalysisTable'}
        loading={!session.durationFileCompleted}
        dataSource={tableData}
        columns={columns}
        expandable={{
            expandedRowRender: nextLevelTable,
            expandedRowKeys,
            showExpandColumn: false,
        }}
        rowKey={rowKey}
        pagination={getPageConfigWithAllData(dataSource.length)}
        onChange={(pagination: any, filters: any, sorter: any, extra: any): void => {
            if (extra.action === 'sort') {
                setExpandedKeys([]);
                if (typeof updateSort === 'function') {
                    updateSort(extra.currentDataSource);
                }
            }
        } }
    />;
});

const CommunicationTime = observer((props: IProps) => {
    const { t } = useTranslation('communication');
    return <CollapsiblePanel title={t('sessionTitle.DataAnalysisCommunicationTime')}>
        <CommunicationTimeTable {...props}/>
    </CollapsiblePanel>;
});

export default CommunicationTime;
