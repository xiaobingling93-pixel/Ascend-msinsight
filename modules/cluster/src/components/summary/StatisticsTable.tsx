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
import { type AdviceInfo } from './ComputationCommunicationOverview';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Advice } from 'ascend-utils';

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

const serachData = async({ rankId, record, page, sorter, name, step }: any): Promise<{total: number;data: object[]}> => {
    let data;
    let total;
    const param = {
        rankId,
        timeFlag: record.acceleratorCore,
        currentPage: page.current,
        pageSize: page.pageSize,
        orderBy: sorter.field,
        order: sorter.order,
        step: step === 'All' ? '' : step,
    };
    if (name === 'computeDetail') {
        const res = await queryComputeDetail(param);
        data = res?.computeDetails ?? [];
        data = data.map((item: any) => ({
            ...item,
            duration: Number(item.duration?.toFixed(4)),
            waitTime: Number(item.waitTime?.toFixed(4)),
        }));
        total = res.totalNum;
    } else {
        const res = await queryCommunicationDetail(param);
        data = res?.communicationDetails ?? [];
        total = res?.totalNum ?? 0;
    }
    return { data, total };
};

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: '', order: 'descend' };
const DetailTable = ({ rankId, record, name, step }: any): JSX.Element => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const { columns, rowKey } = useTableSet(name);
    useEffect(() => {
        updateData(page, sorter);
    }, [page.current, page.pageSize, sorter.field, sorter.order, record.acceleratorCore, rankId]);
    const updateData = async(_page: any, _sorter: any): Promise<void> => {
        const { data, total } = await serachData({ rankId, record, page: _page, sorter: _sorter, name, step });
        setDataSource(data);
        setPage({ ..._page, total });
    };

    return <div>
        <ResizeTable
            dataSource={dataSource}
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
    const { rankId = '', step = '' } = props;
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const { columns, rowKey } = useTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId, props.step]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId, stepId: step === 'All' ? '' : step });
        let data = res?.summaryStatisticsItemList ?? [];
        data = data.map((item: any) => ({
            ...item,
            duration: Number(item.duration.toFixed(4)),
            utilization: Number(item.utilization.toFixed(4)),
        }));
        setDataSource(data);
    };

    return <ResizeTable
        dataSource={ dataSource}
        columns={columns}
        expandable={{
            expandedRowRender: (record: any) => <DetailTable record={record}
                name={`${timeFlag}Detail`} rankId={ rankId} step={step}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
    />;
};

export const CommunicationStatisticsTable = (props: any): JSX.Element => {
    const timeFlag = 'communication';
    const { rankId = '', step = '' } = props;
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const { columns, rowKey } = useTableSet(timeFlag, setExpandedKeys);
    useEffect(() => {
        updateData();
        setExpandedKeys([]);
    }, [props.rankId, props.step]);
    const updateData = async (): Promise<void> => {
        const res = await querySummaryStatistics({ timeFlag, rankId, stepId: step === 'All' ? '' : step });
        const list: any[] = res?.summaryStatisticsItemList ?? [];
        const data = {
            acceleratorCore: 'HCCL',
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
                name={'communicationDetail' } rankId={ rankId} step={step}/>,
            expandedRowKeys,
            expandIcon: () => (<></>),
        }}
        rowKey={rowKey}
        pagination={false}
    />;
};

const AdviceLabel = (props: {advice: AdviceInfo}): JSX.Element => {
    const { t } = useTranslation('summary');
    const { advice } = props;
    let adviceText = '';
    for (const [key, value] of Object.entries(advice)) {
        const capitalKey = key.charAt(0).toUpperCase() + key.slice(1);
        if (value !== 0) {
            adviceText += t('SummaryAdvice', { type: t(capitalKey), time: value });
        }
    }
    return (<Advice text={adviceText}/>);
};

const StatisticsTable = (props: {step: string; rankId: string;session: Session; advice: AdviceInfo}): JSX.Element => {
    const { rankId = '', step = '', session, advice } = props;
    const { t } = useTranslation('summary');
    return notNull(rankId) && session.unitcount > 0
        ? (
            <div>
                {advice.communication + advice.compute + advice.free !== 0 &&
                    <AdviceLabel advice={advice} />
                }
                <div style={{ marginBottom: '20px' }}>
                    <CollapsiblePanel
                        secondary
                        title={`${getTitle('compute', t)} ( Rank ${rankId} )`}
                        headerStyle={{ padding: 0 }}
                        contentStyle={{ paddingLeft: 0, paddingRight: 0 }}>
                        {session.parseCompleted
                            ? (<ComputeStatisticsTable rankId={rankId} step={step} session={session}/>)
                            : <div style={{ textAlign: 'center' }}>{ t('Timeline not fully parsed') }</div>
                        }
                    </CollapsiblePanel>
                </div>
                {
                    !session.isFullDb
                        ? (
                            <div style={{ marginBottom: '20px' }}>
                                <CollapsiblePanel
                                    secondary
                                    title={`${t('CommunicationDetail')} ( Rank ${rankId} )`}
                                    headerStyle={{ padding: 0 }}
                                    contentStyle={{ paddingLeft: 0, paddingRight: 0 }}>
                                    {session.parseCompleted
                                        ? <CommunicationStatisticsTable rankId={rankId} step={step} session={session}/>
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
export default StatisticsTable;
