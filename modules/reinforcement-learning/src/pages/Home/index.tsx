/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { observer } from 'mobx-react-lite';
import { useTranslation } from 'react-i18next';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { SearchForm } from '@/pages/Home/components/SearchForm';
import { TraceTimeline } from '@/pages/Home/components/TraceTimeline';
import { HeaderStyled } from '@/styles/styles';

export const Home: React.FC = observer(() => {
    const { t } = useTranslation('RL');

    return <>
        <HeaderStyled>
            <SearchForm></SearchForm>
        </HeaderStyled>
        <CollapsiblePanel id={'task-trace-timeline'} title={t('Task Trace Timeline')}>
            <TraceTimeline></TraceTimeline>
        </CollapsiblePanel>
    </>;
});
