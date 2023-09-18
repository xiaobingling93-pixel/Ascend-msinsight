import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import * as React from 'react';
import { Session } from '../entity/session';
import { TimeMakerButton } from './TimeMakerButton';
import { UnitsFilter } from './UnitsFilter';
import { CategorySearch } from './CategorySearch';

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
    return (<Container>
        <TimeMakerButton session={session} />
        <UnitsFilter session={session} />
        <CategorySearch session={session} />
        {session.buttons.map((_Button, index) => <_Button session={session} key={`${session.id}-${index}`} />)}
        {unit?.buttons?.map((_Button, index) => <_Button session={session} key={`${unit.name}-${index}`} />)}
    </Container>);
});
