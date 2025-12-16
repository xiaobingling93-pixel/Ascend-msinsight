/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { CollapsiblePanel, Tooltip } from '@insight/lib/components';
import { HelpIcon, BulbIcon } from '@insight/lib/icon';
import { Spin, type TableColumnsType } from 'antd';
import { ResizeTable } from '@insight/lib/resize';
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
    const [loading, setLoading] = useState(false);
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
        setLoading(true);
        const communicationExpertData = await queryCommunicationExpertAdvisor().finally(() => {
            setLoading(false);
        });
        setExpertAdviceData(communicationExpertData ?? {});
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
                    dataList.push({ title: obj.name, tableColumns, tableDataSource: [] });
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
                dataList.push({ title: obj.name, tableColumns, tableDataSource });
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
            <Spin spinning={loading}>
                <CollapsiblePanel title={<div>
                    <BulbIcon style={{ marginTop: '2px', marginRight: '8px' }}/>
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
            </Spin>
        </div>
    );
};

const FetchExpertIndex = (expertAdviceList: Array<{ title: string; tableColumns: object[]; tableDataSource: object[] }>): JSX.Element[] => {
    const { t } = useTranslation('communication');
    const results: JSX.Element[] = [];
    const getContent = (title: string): string => {
        switch (title) {
            case 'Byte Alignment Analysis':
                return `${t('title.Byte Alignment Analysis')} ${t('index.Byte Alignment Analysis')}`;
            case 'Bandwidth Contention Analysis':
                return `${t('title.Bandwidth Contention')} ${t('index.Bandwidth Contention')}`;
            case 'Communication Retransmission Analysis':
                return `${t('title.RDMA Transmission Time')} ${t('index.RDMA Transmission Time')}\n${t('title.Network Configuration')} ${t('index.Network Configuration')}`;
            case 'Packet Analysis':
                return `${t('title.Data Parallelism')} ${t('index.Data Parallelism')}\n${t('title.Memory Optimization')} ${t('index.Memory Optimization')}\n${t('title.Adopt')} ${t('index.Adopt')}`;
            default:
                return '';
        }
    };
    expertAdviceList.forEach(item => {
        const content = getContent(item.title);
        if (content.length === 0) {
            return;
        }
        results.push(
            <>
                <CommunicationAdviceHeader title={item.title} suffix={`${t('title.Data')}`} />
                {
                    item.tableDataSource.length === 0
                        ? <div className="communication-expert-index">{t('index.No problematic operators')}</div>
                        : (
                            <ResizeTable
                                columns={item.tableColumns}
                                dataSource={item.tableDataSource}
                                size="small"
                                pagination={getPageConfigWithAllData(item.tableDataSource.length)}
                            />
                        )
                }
                <div className="communication-advice-header">{`${t(item.title)}${t('title.Advice')}`}</div>
                <div className="communication-expert-index">{content}</div>
            </>,
        );
    });
    return results;
};

const CommunicationAdviceHeader = ({ title, suffix }: {title: string; suffix: string}): JSX.Element => {
    const { t } = useTranslation('communication');
    const tooltipContent = (
        <div style={{ padding: '10px' }}>
            {t(`tooltip.${title}`)}
        </div>
    );

    return (
        <div className="communication-advice-header">
            {`${t(title)}${t(suffix)}`}
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
