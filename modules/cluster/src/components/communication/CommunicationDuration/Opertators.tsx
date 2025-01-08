/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import React from 'react';
import { useTranslation } from 'react-i18next';
import { ArrowLeftOutlined } from '@ant-design/icons';
import BandwidthAnalysis from './BandwidthAnalysis';
import styled from '@emotion/styled';

const FixedBox = styled.div`
    z-index: 10;
    position: fixed;
    top: ${(props): string => props.theme.pagePadding};
    left: ${(props): string => props.theme.pagePadding};
    right: ${(props): string => props.theme.pagePadding};
    bottom: ${(props): string => props.theme.pagePadding};
    background: ${(props): string => props.theme.bgColor};
    overflow: auto;
    border-radius: ${(props): string => props.theme.borderRadiusBase};
`;

const BreadcrumbBox = styled.div`
    display: flex;
    align-items: center;
    padding: 16px 24px;
    .btn-back{
        display: flex;
        align-items: center;
        gap: 4px;
        color: ${(props): string => props.theme.primaryColor};
        cursor: pointer;
    }
    .delimiter{
        padding: 0 8px;
    }
`;
const Operators = ({ returnHome, rankId, operatorName, iterationId, stage }: any): JSX.Element => {
    const { t } = useTranslation('communication');
    return (
        <FixedBox data-testid={'operators'}>
            <BreadcrumbBox>
                <div className={'btn-back'} onClick={returnHome}>
                    <ArrowLeftOutlined />
                    <div>{t('Back')}</div>
                </div>
                <div className="delimiter">|</div>
                <div data-testid={'operatorRankId'}>{operatorName}(RankId {rankId})</div>
            </BreadcrumbBox>
            <BandwidthAnalysis iterationId={iterationId} rankId={rankId} operatorName={operatorName} stage={stage}/>
        </FixedBox>
    );
};

export default Operators;
