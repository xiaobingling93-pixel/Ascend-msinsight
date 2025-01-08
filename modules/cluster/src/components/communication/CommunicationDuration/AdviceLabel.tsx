/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import React from 'react';
import { useTranslation } from 'react-i18next';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tooltip } from 'ascend-components';
import { HelpIcon } from 'ascend-icon';

export interface CommunicationAdvice {
    type: string;
    max: number;
    min: number;
    avg: number;
    diff: number;
    time: number;
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
            </CollapsiblePanel>
        </div>
    );
};

export default AdviceLabel;
