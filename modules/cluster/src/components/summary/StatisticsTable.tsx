/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Button } from 'ascend-components';
import type { ColumnsType } from 'antd/es/table';
import { DownOutlined } from '@ant-design/icons';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { StringMap } from '../../utils/interface';
import { notNull, getPageConfigWithPageData } from '../Common';
import { queryCommunicationDetail, queryComputeDetail, querySummaryStatistics } from '../../utils/RequestUtils';
import { ResizeTable } from 'ascend-resize';
import type { Session } from '../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';

const useComputingStatisticsColumns = (): ColumnsType => {
    const { t } = useTranslation('summary');
    return [
        {
            title: t('AcceleratorCore'),
            dataIndex: 'acceleratorCore',
            key: 'acceleratorCore',
            ellipsis: true,
        },
        {
            title: `${t('AcceleratorCoreDurations')}(μs)`,
            dataIndex: 'duration',
            key: 'duration',
            ellipsis: true,
        },
    ];
};

const useComputingDetailColumns = (): ColumnsType => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        { title: t('Name'), dataIndex: 'name', sorter: true, ellipsis: true },
        { title: t('Type'), dataIndex: 'type', sorter: true, ellipsis: true },
        { title: `${t('StartTime')}(ms)`, dataIndex: 'startTime', sorter: true, ellipsis: true },
        { title: `${t('Duration')}(μs)`, dataIndex: 'duration', sorter: true, ellipsis: true },
        { title: `${t('WaitTime')}(μs)`, dataIndex: 'waitTime', sorter: true, ellipsis: true },
        { title: t('BlockDim'), dataIndex: 'blockDim', sorter: true, ellipsis: true },
        {
            title: t('InputShapes'),
            dataIndex: 'inputShapes',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('InputDataTypes'),
            dataIndex: 'inputDataTypes',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('InputFormats'),
            dataIndex: 'inputFormats',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputShapes'),
            dataIndex: 'outputShapes',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputDataTypes'),
            dataIndex: 'outputDataTypes',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('OutputFormats'),
            dataIndex: 'outputFormats',
            sorter: true,
            ellipsis: true,
        },
    ];
};

const useCommunicationStatisticsColumns = (): ColumnsType => {
    const { t } = useTranslation('summary');
    return [
        {
            title: t('AcceleratorCore'),
            dataIndex: 'acceleratorCore',
            key: 'acceleratorCore',
            ellipsis: true,
        },
        {
            title: `${t('CommunicationDurationsNotOverlapped')}(μs)`,
            dataIndex: 'notOverlapped',
            key: 'notOverlapped',
            ellipsis: true,
        },
        {
            title: `${t('CommunicationDurationsOverlapped')}(μs)`,
            dataIndex: 'overlapped',
            key: 'overlapped',
            ellipsis: true,
        },
    ];
};

const useCommunicationDetailColumns = (): ColumnsType => {
    const { t } = useTranslation('operator', { keyPrefix: 'tableHead' });
    return [
        {
            title: t('Name'),
            dataIndex: 'name',
            sorter: true,
            ellipsis: true,
        },
        {
            title: t('Type'),
            dataIndex: 'type',
            sorter: true,
            ellipsis: true,
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

const useCommunicationOverlappedDetailColumns = (): ColumnsType => {
    const { t } = useTranslation('summary');
    return [
        ...useCommunicationDetailColumns(),
        {
            title: `${t('Overlapped Durations')}(μs)`,
            dataIndex: 'overlapDuration',
            key: 'overlapDuration',
            sorter: true,
            ellipsis: true,
        },
    ];
};

const useCommunicationNotOverlappedDetailColumns = (): ColumnsType => {
    const { t } = useTranslation('summary');
    return [
        ...useCommunicationDetailColumns(),
        {
            title: `${t('Not Overlapped Durations')}(μs)`,
            dataIndex: 'notOverlapDuration',
            key: 'notOverlapDuration',
            sorter: true,
            ellipsis: true,
        },
    ];
};

const rowKeyMap: any = {
    compute: 'acceleratorCore',
    communication: 'acceleratorCore',
};
const useColMap = (): any => ({
    compute: useComputingStatisticsColumns(),
    computeDetail: useComputingDetailColumns(),
    communication: useCommunicationStatisticsColumns(),
    communicationDetail: useCommunicationDetailColumns(),
    communicationOverlappedDetail: useCommunicationOverlappedDetailColumns(),
    communicationNotOverlappedDetail: useCommunicationNotOverlappedDetailColumns(),
});

const useTableSet = (timeFlag: string, setExpandedKeys?: any): any => {
    const rowKey = rowKeyMap[timeFlag];
    const colMap = useColMap();
    const columns = notNull(colMap[timeFlag]) ? [...colMap[timeFlag]] : [];
    const { t } = useTranslation('summary');
    if (['compute', 'communication'].includes(timeFlag)) {
        const btnCol = {
            title: t('Details'),
            ellipsis: true,
            minWidth: 85,
            render: (_: any, record: any) => (<Button type="link"
                onClick={(): void => {
                    setExpandedKeys((pre: string[]) => {
                        const list = [...pre];
                        const keyIndex = list.indexOf(record[rowKey]);
                        if (keyIndex === -1) {
                            list.push(record[rowKey]);
                        } else {
                            list.splice(keyIndex, 1);
                        }
                        return list;
                    });
                }}>{t('Details')}<DownOutlined/></Button>),
        };
        columns.push(btnCol);
    }

    return { rowKey, columns };
};

const searchData = async({ rankId, dbPath, record, page, sorter, name, step, clusterPath }: any): Promise<{total: number;data: object[]}> => {
    let data;
    let total;
    const param = {
        rankId,
        dbPath,
        timeFlag: record.acceleratorCore,
        currentPage: page.current,
        pageSize: page.pageSize,
        orderBy: sorter.field,
        order: sorter.order,
        step: step === 'All' ? '' : step,
        clusterPath,
    };
    if (name === 'computeDetail') {
        try {
            const res = await queryComputeDetail(param);
            data = res?.computeDetails ?? [];
            total = res.totalNum;
        } catch (e) {
            data = [];
            total = 0;
        }

        data = data.map((item: any) => ({
            ...item,
            duration: Number(item.duration?.toFixed(4)),
            waitTime: Number(item.waitTime?.toFixed(4)),
        }));
    } else {
        try {
            const res = await queryCommunicationDetail(param);
            data = res?.communicationDetails ?? [];
            total = res?.totalNum ?? 0;
        } catch (e) {
            data = [];
            total = 0;
        }
    }
    return { data, total };
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: 'descend' };
const DetailTable = ({ rankId, dbPath, record, name, step, clusterPath }: any): JSX.Element => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const { columns, rowKey } = useTableSet(name);
    useEffect(() => {
        updateData(page, sorter);
    }, [page.current, page.pageSize, sorter.field, sorter.order, record.acceleratorCore, rankId]);
    const updateData = async(_page: any, _sorter: any): Promise<void> => {
        const { data, total } = await searchData({ rankId, dbPath, record, page: _page, sorter: _sorter, name, step, clusterPath });
        setDataSource(data);
        setPage({ ..._page, total });
    };

    return <div>
        <ResizeTable
            dataSource={dataSource}
            allowCopy
            columns={columns}
            rowKey={rowKey}
            size="small"
            pagination={getPageConfigWithPageData(page, setPage)}
            onChange={(pagination: any, filters: any, mySorter: any, extra: any): void => {
                if (extra.action === 'sort') {
                    setSorter(mySorter);
                }
            }
            }
        />
    </div>;
};

const timeFlagMap: StringMap = {
    compute: 'Computing',
    communicationNotOverLappedTime: 'Communication(Not OverLapped)',
    communicationOverLappedTime: 'Communication(OverLapped)',
    freeTime: 'Free',
};
function getTitle(timeFlag: string, t: TFunction): string {
    return t(`${timeFlagMap[timeFlag]}Detail`);
}

export const ComputeStatisticsTable = (props: any): JSX.Element => {
    const timeFlag = 'compute';
    const { rankId = '', dbPath = '', step = '', session } = props;
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const { columns, rowKey } = useTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId, props.step, session.selectedClusterPath]);
    const updateData = async (): Promise<void> => {
        try {
            const res = await querySummaryStatistics(
                { timeFlag, rankId, dbPath, stepId: step === 'All' ? '' : step, clusterPath: session.selectedClusterPath });
            let data = res?.summaryStatisticsItemList ?? [];
            data = data.map((item: any) => ({
                ...item,
                duration: Number(item.duration.toFixed(4)),
                utilization: Number(item.utilization.toFixed(4)),
            }));
            setDataSource(data);
        } catch (e) {
            setDataSource([]);
        }
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DetailTable record={record}
                name={`${timeFlag}Detail`} rankId={rankId} dbPath={dbPath} step={step} clusterPath={session.selectedClusterPath} />,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
    />;
};

export const CommunicationStatisticsTable = (props: any): JSX.Element => {
    const timeFlag = 'communication';
    const { rankId = '', dbPath = '', step = '', session } = props;
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const { columns, rowKey } = useTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId, props.step, session.selectedClusterPath]);
    const updateData = async (): Promise<void> => {
        let list: any[] = [];
        try {
            const res = await querySummaryStatistics(
                { timeFlag, rankId, dbPath, stepId: step === 'All' ? '' : step, clusterPath: session.selectedClusterPath });
            list = res?.summaryStatisticsItemList ?? [];
        } catch (e) {
            list = [];
        }

        const data = {
            acceleratorCore: 'Communication',
            overlapped: list.find(item => item.overlapType === 'Communication(Overlapped)')?.duration,
            notOverlapped: list.find(item => item.overlapType === 'Communication(Not Overlapped)')?.duration,
        };
        setDataSource([data]);
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DetailTable record={record}
                name={'communicationDetail' } rankId={rankId} dbPath={dbPath} step={step} clusterPath={session.selectedClusterPath} />,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
    />;
};

export const StatisticsTable = (props: {step: string; rankId: string; dbPath: string; session: Session }): JSX.Element => {
    const { rankId = '', dbPath = '', step = '', session } = props;
    const { t } = useTranslation('summary');
    return notNull(rankId) && session.unitcount > 0
        ? (
            <div data-testid="statistics-table-container">
                <div data-testid="computing-detail" style={{ marginBottom: '20px' }}>
                    <CollapsiblePanel
                        secondary
                        title={`${getTitle('compute', t)} ( Rank ${rankId} )`}
                        headerStyle={{ padding: 0 }}
                        contentStyle={{ paddingLeft: 0, paddingRight: 0 }}>
                        {session.parseCompleted
                            ? (<ComputeStatisticsTable rankId={rankId} dbPath={dbPath} step={step} session={session}/>)
                            : <div style={{ textAlign: 'center' }}>{ t('Timeline not fully parsed') }</div>
                        }
                    </CollapsiblePanel>
                </div>
                {
                    !session.isFullDb
                        ? (
                            <div data-testid="communication-detail" style={{ marginBottom: '20px' }}>
                                <CollapsiblePanel
                                    secondary
                                    title={`${t('CommunicationDetail')} ( Rank ${rankId} )`}
                                    headerStyle={{ padding: 0 }}
                                    contentStyle={{ paddingLeft: 0, paddingRight: 0 }}>
                                    {session.parseCompleted
                                        ? <CommunicationStatisticsTable rankId={rankId} dbPath={dbPath} step={step} session={session}/>
                                        : <div style={{ textAlign: 'center' }}>{ t('Timeline not fully parsed') }</div>
                                    }
                                </CollapsiblePanel>
                            </div>
                        )
                        : <></>
                }
            </div>)
        : <></>
    ;
};
