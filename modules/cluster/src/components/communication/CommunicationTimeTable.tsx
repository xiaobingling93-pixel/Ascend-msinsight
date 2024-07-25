/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Button } from 'lib/components';
import type { TableColumnsType } from 'antd';
import { DownOutlined } from '@ant-design/icons';
import type { ColumnsType } from 'antd/es/table';
import { getPageConfigWithAllData, getPageConfigWithPageData } from '../Common';
import type { VoidFunction } from '../../utils/interface';
import { queryOperatorDetails } from '../../utils/RequestUtils';
import { totalOperator } from './Filter';
import ResizeTable from 'lib/ResizeTable';
import type { Session } from '../../entity/session';
import CollapsiblePanel from 'lib/CollapsiblePanel';

export interface DataType {
    [prop: string]: any;
}

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

    useEffect(() => {
        updateData(page, sorter);
    }, [page.current, page.pageSize, sorter.field, sorter.order, conditions.iterationId, record.rankId]);
    const updateData = async(_page: any, _sorter: {field: string;order: string}): Promise<void> => {
        const res = await queryOperatorDetails({
            iterationId: conditions.iterationId,
            rankId: record.rankId,
            currentPage: _page.current,
            pageSize: _page.pageSize,
            orderBy: _sorter.field,
            order: _sorter.order,
            stage: conditions.stage,
        });
        setDataSource(res?.allOperators ?? []);
        setPage({ ..._page, total: res?.count ?? 0 });
    };

    const columns: TableColumnsType<DataType> = [
        { title: 'Operator Name', dataIndex: 'operatorName', key: 'operatorName', sorter: true, ellipsis: true },
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

const useRankColumns = (handleAction: VoidFunction[], conditions: any, t: TFunction): any => {
    const [showOperator, setExpandedKeys] = handleAction;
    return [
        {
            title: t('tableHead.Rank ID'),
            dataIndex: 'rankId',
            key: 'rankId',
            sorter: (a: DataType, b: DataType) => Number(a.rankId) - Number(b.rankId),
            ellipsis: true,
            width: 70,
        },
        ...useCommonColumns(),
        {
            title: t('tableHead.Bandwidth Analysis'),
            key: 'action1',
            ellipsis: true,
            width: 110,
            minWidth: 100,
            render: (_: any, record: DataType) => (
                <Button type="link"
                    onClick={(): void => {
                        showOperator(record.rankId);
                    }}>{t('tableHead.see more')}</Button>),
        },
        {
            title: t('tableHead.Communication Operators Details'),
            key: 'action2',
            ellipsis: true,
            width: 110,
            minWidth: 110,
            render: (_: any, record: DataType) => (<Button type="link"
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
                }}>{t('tableHead.see more')}<DownOutlined/></Button>),
            display: conditions.operatorName === totalOperator,
        },
    ].filter((item: any) => item.display !== false);
};
const rowKey = 'index';
const CommunicationTimeTable = observer((props:
{dataSource?: DataType[];showOperator: (rankid: string) => void;conditions: any;updateSort: VoidFunction; session: Session}) => {
    const { t } = useTranslation('communication');
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const columns = useRankColumns([props.showOperator, setExpandedKeys], props.conditions, t);
    const dataSource: DataType[] = props.dataSource ?? [];
    useEffect(() => {
        setExpandedKeys([]);
    }, [props.dataSource]);
    return (
        <CollapsiblePanel title={t('sessionTitle.DataAnalysisCommunicationTime')}>
            <ResizeTable
                loading={!props.session.durationFileCompleted}
                dataSource={dataSource}
                columns={columns}
                expandable={{
                    expandedRowRender: (record: DataType): JSX.Element => <div style={{ marginLeft: '0' }}>
                        <OperatorsTable record={record} conditions={props.conditions}/>
                    </div>,
                    expandedRowKeys,
                    expandIcon: (): JSX.Element => (<></>),
                }}
                rowKey={rowKey}
                pagination={getPageConfigWithAllData(dataSource.length)}
                onChange={(pagination: any, filters: any, sorter: any, extra: any): void => {
                    if (extra.action === 'sort') {
                        setExpandedKeys([]);
                        props.updateSort(extra.currentDataSource);
                    }
                }
                }
            />
        </CollapsiblePanel>
    );
});

export default CommunicationTimeTable;
