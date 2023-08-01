import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import * as React from 'react';
import { Session } from '../entity/session';
import { TimeMakerButton } from './TimeMakerButton';
import { CustomButton } from './base/StyledButton';
import { ReactComponent as OpenIcon } from '../assets/images/ic_public_download.svg';
import { ReactComponent as ResetIcon } from '../assets/images/insights/ark_gc.svg';
import { runInAction } from 'mobx';
import { useState } from 'react';
import { CardUnit } from '../insight/units/AscendUnit';
import { processUnits } from '../entity/insight';
import { messageSender } from '../connection/messageSender';

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
    const [ isImporting, setIsImporting ] = useState(false);
    return (<Container>
        {<CustomButton icon={ OpenIcon } disabled={isImporting}
            onClick={() => selectFolders(isImporting, setIsImporting, session)}
        />}
        {<CustomButton icon={ ResetIcon }
            onClick={() => resetSession(session)}
        />}
        <TimeMakerButton session={session} />
        {session.buttons.map((_Button, index) => <_Button session={session} key={`${session.id}-${index}`} />)}
        {unit?.buttons?.map((_Button, index) => <_Button session={session} key={`${unit.name}-${index}`} />)}
    </Container>);
});

type CardInfo = {
    cardName: string;
    rankId: string;
};

const selectFolders = async (isImporting: boolean, setIsImporting: React.Dispatch<React.SetStateAction<boolean>>, session: Session): Promise<void> => {
    setIsImporting(true);
    const selectedPath = await messageSender.selectFolder();
    const result = await window.request('import/action', { path: selectedPath });
    runInAction(() => {
        session.phase = 'download';
        session.endTimeAll = 1000000000;
        result.result.forEach((item: CardInfo) => {
            session.units.push(new CardUnit({ cardId: item.rankId, cardName: item.cardName }));
        });
        processUnits(session.units, 'analyzing');
    });
    setIsImporting(false);
};

const resetSession = async (session: Session): Promise<void> => {
    runInAction(() => {
        location.reload();
    });
    await window.request('reset/window', {});
};
