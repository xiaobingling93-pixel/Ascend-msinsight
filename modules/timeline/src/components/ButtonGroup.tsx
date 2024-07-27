/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import * as React from 'react';
import type { Session } from '../entity/session';
import { TimeMakerButton } from './TimeMakerButton';
import { UnitsFilter } from './UnitsFilter';
import { CategorySearch } from './CategorySearch';
import { FilterLinkLine } from './FilterLinkLine';

const Container = styled.div`
    display: flex;
    align-items: center;
    text-align: center;
    font-size: 12px;
    margin-right: 1em;
    svg {
        cursor: pointer;
    }
`;

export const ButtonGroup = observer(({ session }: { session: Session }) => {
    const unit = session.selectedUnits[0];
    const isRenderLink = !(session.isSimulation && session.areFlagEventsHidden);
    return (<Container>
        <TimeMakerButton session={session} />
        <UnitsFilter session={session} />
        <CategorySearch session={session} />
        {isRenderLink && <FilterLinkLine session={session}/>}
        {session.buttons.map((_Button, index) => <_Button session={session} key={`${session.id}-${index}`} />)}
        {unit?.buttons?.map((_Button, index) => <_Button session={session} key={`${unit.name}-${index}`} />)}
    </Container>);
});
