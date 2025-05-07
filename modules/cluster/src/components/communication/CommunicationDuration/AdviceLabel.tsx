/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import React, { useState } from 'react';
import { useTranslation } from 'react-i18next';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tooltip } from 'ascend-components';
import { HelpIcon } from 'ascend-icon';
import { type TableColumnsType } from 'antd';
import { ResizeTable } from 'ascend-resize';
import { DataType } from '../CommunicationTimeTable';

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

const AdviceLabel = (props: {adviceData: CommunicationAdvice[]; expertAdviceData: CommunicationExpertAdvice}): JSX.Element => {
    const { t } = useTranslation('communication');
    const { adviceData, expertAdviceData } = props;
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [columns, setColumns] = useState<TableColumnsType<DataType>>([]);
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
    const expertAdviceList: Array<{title: string;content: string[] }> = [];
    Object.values(expertAdviceData).forEach(data => {
        data.forEach((obj: { name: string; statistics: any; suggestions: string[] }) => {
            const tableColumns: TableColumnsType<DataType> = Object.keys(obj.statistics).map((key) => ({
                title: key,
                dataIndex: key,
                key,
            }));
            const tableDataSource: any[] = obj.statistics.Category.map((_: any, index: string | number) => {
                const rowData: Record<string, string | number> = { key: index };
                Object.keys(obj.statistics).forEach((key) => {
                    rowData[key] = obj.statistics[key][index]; // 按索引填充数据
                });
                return rowData;
            });
            setColumns(tableColumns);
            setDataSource(tableDataSource);
            expertAdviceList.push({
                title: obj.name,
                content: obj.suggestions,
            });
        });
    });

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
                    expertAdviceList.map(item => {
                        return (
                            <>
                                <div className="communication-advice-header">{item.title}</div>
                                <div className="communication-advice-content">{item.content}</div>
                                <ResizeTable columns={columns} dataSource={dataSource} size="small"></ResizeTable>
                            </>
                        );
                    })
                }
            </CollapsiblePanel>
        </div>
    );
};

export default AdviceLabel;
