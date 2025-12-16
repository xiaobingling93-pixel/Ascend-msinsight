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
const Operators = ({ returnHome, rankId, dbPath, operatorName, iterationId, stage, pgName, groupIdHash }: any): JSX.Element => {
    const { t } = useTranslation('communication');
    return (
        <FixedBox data-testid={'operators'}>
            <BreadcrumbBox>
                <div className={'btn-back'} onClick={returnHome}>
                    <ArrowLeftOutlined />
                    <div>{t('Back')}</div>
                </div>
                <div className="delimiter">|</div>
                <div>{operatorName}(RankId {rankId})</div>
            </BreadcrumbBox>
            <BandwidthAnalysis iterationId={iterationId} rankId={rankId} dbPath={dbPath} operatorName={operatorName} stage={stage} pgName={pgName} groupIdHash={groupIdHash}/>
        </FixedBox>
    );
};

export default Operators;
