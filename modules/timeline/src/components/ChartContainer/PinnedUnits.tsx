import * as React from 'react';
import { observer } from 'mobx-react';
import styled from '@emotion/styled';
// support utils/types
import { Session } from '../../entity/session';
import { getAutoKey } from '../../utils/dataAutoKey';
// same level infer
import { Unit } from './Units';
import { isPinned } from './unitPin';

const PinnedUnitsContainer = styled.div`
    box-shadow: 1px 2px 11px 1px ${props => props.theme.shadowBackgroundColor};
    z-index: 1;
`;

export const PinnedUnits = observer(({ session, laneInfoWidth }: { session: Session; laneInfoWidth: number }) => {
    return <PinnedUnitsContainer>
        {session.pinnedUnits.map((pinnedUnit) => <Unit
            key={getAutoKey(pinnedUnit)}
            unit={pinnedUnit}
            session={session}
            isVisible={true}
            hasExpandIcon={false}
            laneInfoWidth={laneInfoWidth}
            // homepage don't have pin button
            hasPinButton={true}
            isPinned={isPinned(pinnedUnit)}
        />)}
    </PinnedUnitsContainer>;
});
