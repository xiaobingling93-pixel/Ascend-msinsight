/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tooltip } from 'ascend-components';
import { HelpIcon } from 'ascend-icon';
import { type TableColumnsType } from 'antd';
import { ResizeTable } from 'ascend-resize';
import { DataType } from '../CommunicationTimeTable';
import { queryCommunicationExpertAdvisor } from '../../../utils/RequestUtils';
import { getPageConfigWithAllData } from '../../Common';

export interface CommunicationAdvice {
    type: string;
    max: number;
    min: number;
    avg: number;
    diff: number;
    time: number;
}

export interface CommunicationExpertAdvice {
    name: string;
    statistics: object;
    suggestions: string[];
}

const AdviceLabel = (props: {adviceData: CommunicationAdvice[]}): JSX.Element => {
    const { t } = useTranslation('communication');
    const { adviceData } = props;
    let overAllText = '';
    const issueList: Array<{title: string; content: string }> = [];
    const sdmaData = adviceData.find(item => item.type === 'SDMA');
    const rdmaData = adviceData.find(item => item.type === 'RDMA');
    adviceData.forEach(data => {
        overAllText += t('OverallDuration', { type: data.type, time: data.time });
        // 比较经验带宽（最大带宽的0.8）与平均带宽
        const isBandwidthIssue = data.avg >= data.max * 0.8;
        issueList.push({
            title: data.type,
            content: t('CommunicationAdvice', { ...data, issue: isBandwidthIssue ? t('BandwidthIssue') : t('CommunicationIssue') }),
        });
    });
    const [expertAdviceData, setExpertAdviceData] = useState<CommunicationExpertAdvice>({} as CommunicationExpertAdvice);
    const getCommunicationExpertAdvisorData = async (): Promise<void> => {
        setExpertAdviceData(await queryCommunicationExpertAdvisor() ?? {});
    };
    const [expertAdviceList, setDataList] = useState<Array<{title: string; tableColumns: object[]; tableDataSource: object[] }>>([]);
    const getInTableHead = (key: string): string => {
        const result = `tableHead.${key}`;
        return result;
    };
    const fetchDataAndSetData = (): void => {
        const dataList: any[] = [];
        Object.values(expertAdviceData).forEach(data => {
            data.forEach((obj: { name: string; statistics: any; suggestions: string[] }) => {
                const tableColumns: TableColumnsType<DataType> = Object.keys(obj.statistics)?.map((key) => ({
                    title: t(getInTableHead(key)),
                    dataIndex: t(getInTableHead(key)),
                    key: t(getInTableHead(key)),
                }));
                if (Object.keys(obj.statistics).length === 0) {
                    dataList.push({
                        title: obj.name,
                        tableColumns: tableColumns,
                        tableDataSource: [],
                    });
                    return;
                }
                const firstKey = Object.keys(obj.statistics)[0];
                const tableDataSource: any[] = obj.statistics[firstKey].map((_: any, index: string | number) => {
                    const rowData: Record<string, string | number> = { key: index };
                    Object.keys(obj.statistics).forEach((key) => {
                        rowData[t(getInTableHead(key))] = obj.statistics[key][index]; // 按索引填充数据
                    });
                    return rowData;
                });
                dataList.push({
                    title: obj.name,
                    tableColumns: tableColumns,
                    tableDataSource: tableDataSource,
                });
            });
        });
        setDataList(dataList);
    };
    useEffect(() => {
        fetchDataAndSetData();
    }, [JSON.stringify(expertAdviceData), t]);
    useEffect(() => {
        getCommunicationExpertAdvisorData();
    }, []);
    if (sdmaData && rdmaData) {
        overAllText += t('MoreFocus', { type: sdmaData.time >= rdmaData.time ? sdmaData.type : rdmaData.type });
    }
    return (
        <div style={{ marginBottom: '20px' }} data-testid={'communicationAdvice'}>
            <CollapsiblePanel title={<div>
                {t('Advice')}
                <Tooltip title={
                    (
                        <div style={{ padding: '1rem' }}>
                            {t('AdviceTip')}
                        </div>
                    )
                }>
                    <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20}/>
                </Tooltip>
            </div>}>
                <div className="communication-advice-header">{t('Overall')}</div>
                <div className="communication-advice-content">{overAllText}</div>
                {
                    issueList.map(item => {
                        return (
                            <>
                                <div className="communication-advice-header">{item.title}</div>
                                <div className="communication-advice-content">{item.content}</div>
                            </>
                        );
                    })
                }
                {
                    FetchExpertIndex(expertAdviceList)
                }
            </CollapsiblePanel>
        </div>
    );
};

const FetchExpertIndex = (expertAdviceList: Array<{ title: string; tableColumns: object[]; tableDataSource: object[] }>): JSX.Element[] => {
    const { t } = useTranslation('communication');
    const results: JSX.Element[] = [];
    expertAdviceList.forEach(item => {
        let content: JSX.Element;
        if (item.title === 'Packet Analysis') {
            const dataParallelism = `${t('title.Data Parallelism')} ${t('index.Data Parallelism')}\n\n`;
            const memoryOptimization = `${t('title.Memory Optimization')} ${t('index.Memory Optimization')}\n\n`;
            const adopt = `${t('title.Adopt')} ${t('index.Adopt')}\n`;
            content = (
                <>
                    <CommunicationAdviceHeader title={item.title}/>
                    <div className="communication-expert-index">{dataParallelism + memoryOptimization + adopt}</div>
                    <ResizeTable columns={item.tableColumns} dataSource={item.tableDataSource} size="small" pagination={getPageConfigWithAllData(item.tableDataSource.length)}></ResizeTable>
                </>
            );
        } else if (item.title === 'Byte Alignment Analysis') {
            const byteAlignmentAnalysis = `${t('title.Byte Alignment Analysis')} ${t('index.Byte Alignment Analysis')}\n`;
            content = (
                <>
                    <CommunicationAdviceHeader title={item.title}/>
                    <div className="communication-expert-index">{byteAlignmentAnalysis}</div>
                    <ResizeTable columns={item.tableColumns} dataSource={item.tableDataSource} size="small" pagination={getPageConfigWithAllData(item.tableDataSource.length)}></ResizeTable>
                </>
            );
        } else if (item.title === 'Bandwidth Contention Analysis') {
            const bandwidthContention = `${t('title.Bandwidth Contention')} ${t('index.Bandwidth Contention')}\n`;
            content = (
                <>
                    <CommunicationAdviceHeader title={item.title}/>
                    <div className="communication-expert-index">{bandwidthContention}</div>
                    <ResizeTable columns={item.tableColumns} dataSource={item.tableDataSource} size="small"></ResizeTable>
                </>
            );
        } else if (item.title === 'Communication Retransmission Analysis') {
            const transmissionTime = `${t('title.RDMA Transmission Time')} ${t('index.RDMA Transmission Time')}`;
            const networkConfiguration = `${t('title.Network Configuration')} ${t('index.Network Configuration')}\n`;
            content = (
                <>
                    <CommunicationAdviceHeader title={item.title}/>
                    <div className="communication-expert-index">{transmissionTime}</div>
                    <div className="communication-expert-index">{networkConfiguration}</div>
                    <ResizeTable columns={item.tableColumns} dataSource={item.tableDataSource} size="small"></ResizeTable>
                </>
            );
        } else {
            return;
        }
        results.push(content);
    });
    return results;
};

const CommunicationAdviceHeader = ({ title }: {title: string}): JSX.Element => {
    const { t } = useTranslation('communication');
    const tooltipContent = (
        <div style={{ padding: '10px' }}>
            {t(`tooltip.${title}`)}
        </div>
    );

    return (
        <div className="communication-advice-header">
            {t(title)}
            <Tooltip
                placement="left"
                title={tooltipContent}
            >
                <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20} />
            </Tooltip>
        </div>
    );
};

export default AdviceLabel;
