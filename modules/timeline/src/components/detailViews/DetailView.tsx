/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

import React, { useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { Session } from '../../entity/session';
import { useEventBus } from '../../utils/eventBus';
import { ViewList } from './index';

export function getDetailViewItem(session: Session): any {
    return {
        label: <ViewSelect/>,
        key: 'SystemView',
        children: <ViewContainer session={session}/>,
    };
}

const ViewSelect = observer((props: any) => {
    return (<div className={'title'}>{<span>System View</span>}</div>);
});

const ViewContainer = observer((props: any) => {
    const [ view, setView ] = useState(0);
    useEventBus('setView', (data) => setView(data as number));
    const View = useMemo(() => ViewList[view], [view]);
    return (<View session={props.session}/>);
});
