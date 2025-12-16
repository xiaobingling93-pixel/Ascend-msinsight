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

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { Button } from '@insight/lib/components';
import type { SelectContentViewProps } from './SystemView';
import jumpToCorrespondingLane from '../../utils/jumpToCorrespondingLane';

const ExpertAnalysisContainer = styled.div`
    .expert-analysis-header {
        margin-bottom: 8px;
        font-weight: bold;
    }

    .expert-analysis-content {
        padding: 0 12px 20px;
    }
`;

interface ResType {
    cardId: string;
    pid: string;
    propsCardId: string;
}

export const ExpertSummary = observer((props: SelectContentViewProps & { request: (...rest: any[]) => any }): JSX.Element => {
    const { t } = useTranslation('timeline');
    const [hasProblem, setProblem] = useState(false);
    const [number, setNumber] = useState(0);
    const [res, setRes] = useState<ResType>({ cardId: '', pid: '', propsCardId: '' });
    const req = async (): Promise<void> => {
        try {
            const res = await props.request({ rankId: props.card.cardId, dbPath: props.card.dbPath });
            setProblem(res.hasProblem);
            setNumber(res.percent);
            setRes({ cardId: res.rankId, pid: res.pid, propsCardId: props.card.cardId });
        } catch (e) {
            setProblem(false);
        }
    };

    useEffect(() => {
        req();
    }, [props.card.cardId]);

    return <>
        {
            <ExpertAnalysisContainer className={'expert-summary'} >
                <div className="expert-analysis-header">{t('expertSummary.AI Core Frequency')}</div>
                {
                    hasProblem
                        ? <div className="expert-analysis-content">
                            {t('expertSummary.AI Core reduction', { percent: number })}
                            <Button type="link"
                                onClick={() =>
                                    jumpToCorrespondingLane({
                                        cardId: res.cardId,
                                        pid: res.pid,
                                        propsCardId: props.card.cardId,
                                    })
                                }
                            >
                                {t('expertSummary.Click To AI Core Frequency')}
                            </Button>
                        </div>
                        : <div className="expert-analysis-content">
                            {t('expertSummary.AI Core unproblematic', { percent: number })}
                        </div>
                }
            </ExpertAnalysisContainer>
        }
    </>;
});
