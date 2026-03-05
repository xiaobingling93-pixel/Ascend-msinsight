/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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
import React, { useEffect } from 'react';
import { CollapsiblePanel } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { getBarNewData } from './dataHandler';
import { MemoryBlockDiagram } from './leaks/MemoryBlockDiagram';
import { MemoryStateDiagram } from '@/components/leaks/MemoryStateDiagram';

const MemoryStack = observer(({ session }: { session: any }): React.ReactElement => {
    const { t } = useTranslation('triton');

    useEffect(() => {
        if (session.tritonParsed) {
            getBarNewData(session);
        }
    }, [session.renderId, session.tritonParsed]);

    return (
        <>
            <CollapsiblePanel title={t('MemoryBlocks')} style={{ minWidth: 1000 }}>
                <div id="barContent" style={{ overflow: 'hidden', padding: 0, position: 'relative' }}>
                    <MemoryBlockDiagram session={session} />
                </div>
            </CollapsiblePanel>
            <CollapsiblePanel title={t('stateDiagram')} style={{ minWidth: 1000 }}>
                <MemoryStateDiagram session={session} />
            </CollapsiblePanel>
        </>
    );
});

export default MemoryStack;
