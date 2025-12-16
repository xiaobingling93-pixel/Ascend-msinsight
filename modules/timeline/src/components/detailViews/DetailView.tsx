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

import React, { useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import { useEventBus } from '../../utils/eventBus';
import { ViewList } from './index';

export function getDetailViewItem(session: Session, bottomHeight: number): any {
    if (!session.isSimulation) {
        return {
            label: <ViewSelect/>,
            key: 'SystemView',
            children: <ViewContainer session={session} bottomHeight={bottomHeight} />,
        };
    } else {
        return {};
    }
}

const ViewSelect = observer((props: any) => {
    const { t } = useTranslation('timeline');
    return (<div className={'title'}>{<span>{t('System View')}</span>}</div>);
});

const ViewContainer = observer((props: any) => {
    const [view, setView] = useState(0);
    useEventBus('setView', (data) => setView(data as number));
    const View = useMemo(() => ViewList[view], [view]);
    return (<View session={props.session} bottomHeight={props.bottomHeight} />);
});
