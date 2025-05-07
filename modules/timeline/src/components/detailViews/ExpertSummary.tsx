/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { message } from 'ascend-components';

const ExpertAnalysisContainer = styled.div`
    .expert-analysis-header {
        margin-bottom: 8px;
        font-weight: bold;
    }
    
    .expert-analysis-content {
        padding: 0 12px 20px;
    }
`;

export const ExpertSummary = observer((props: any): JSX.Element => {
    const { t } = useTranslation('timeline');
    const [hasProblem, setProblem] = useState(false);
    const [number, setNumber] = useState(0);
    const req = async (): Promise<void> => {
        const res = await props.request({ rankId: props.rankId });
        if (res.error !== undefined) {
            setProblem(false);
            message.error(t('expertSummary.AI Core Request Error'));
            return;
        }
        setProblem(res.hasProblem);
        setNumber(res.percent);
    };

    useEffect(() => {
        req();
    }, [props.rankId]);

    return <>
        {
            <ExpertAnalysisContainer className={'expert-summary'} >
                <div className="expert-analysis-header">{t('expertSummary.AI Core Frequency')}</div>
                {
                    hasProblem ? <div className="expert-analysis-content">{t('expertSummary.AI Core reduction', { percent: number })}</div> : <div className="expert-analysis-content">{t('expertSummary.AI Core unproblematic', { percent: number })}</div>
                }
            </ExpertAnalysisContainer>
        }
    </>;
});
