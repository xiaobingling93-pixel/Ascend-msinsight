/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import { useTranslation } from 'react-i18next';
import { HelpIcon } from 'lib/Icon';
import { StyledTooltip } from './StyledTooltip';

export const Label = (props: {name: React.ReactNode;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name}{' :'} </span>;
};

export const useHit = (): React.ReactElement => {
    const { t } = useTranslation('memory');
    return <StyledTooltip title={
        (
            <div style={{ padding: '1rem' }}>
                <div>{safeStr(t('searchCriteria.Overall'))}: {safeStr(t('searchCriteria.OverallDescribe'))}</div>
                <div style={{ marginTop: '2rem' }}>{safeStr(t('searchCriteria.Stream'))}: {safeStr(t('searchCriteria.StreamDescribe'))}</div>
            </div>
        )
    }>
        <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20}/>
    </StyledTooltip>;
};

export const useChartCharacter = (): React.ReactElement => {
    const { t } = useTranslation('memory');
    const hit = t('searchCriteria.CurveDescribe', { returnObjects: true }) as [];
    return <StyledTooltip title={
        <div style={{ padding: '1rem' }}>
            {hit?.map((item: string, index: number) =>
                <div style={{ padding: '3px 0' }} key={index}>{safeStr(item)}</div>)
            }
        </div>
    }>
        <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20}/>
    </StyledTooltip>;
};

export const safeStr = (str: string, ignore?: string): string => {
    if (str === undefined || str === null) {
        return str;
    }
    if (ignore !== undefined && ignore !== null && ignore !== '') {
        const list = str.split(ignore);
        const safelist = list.map(item => item.replace(/</g, '&lt;').replace(/>/g, '&gt;'));
        return safelist.join(ignore);
    }
    return str?.replace(/</g, '&lt;').replace(/>/g, '&gt;');
};
