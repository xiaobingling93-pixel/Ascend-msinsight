import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import * as React from 'react';
import { Session } from '../entity/session';
import { TimeMakerButton } from './TimeMakerButton';
import { CustomButton } from './base/StyledButton';
import { ReactComponent as AntdResetIcon } from '../assets/images/insights/ark_gc.svg';
import { runInAction } from 'mobx';
import { SvgType } from './base/rc-table/types';
import { store } from '../store';

const ResetIcon = AntdResetIcon as SvgType;

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
        {<CustomButton icon={ ResetIcon }
            onClick={() => resetSession(session)}
        />}
        <TimeMakerButton session={session} />
        {session.buttons.map((_Button, index) => <_Button session={session} key={`${session.id}-${index}`} />)}
        {unit?.buttons?.map((_Button, index) => <_Button session={session} key={`${unit.name}-${index}`} />)}
    </Container>);
});

const resetSession = async (session: Session): Promise<void> => {
    runInAction(async () => {
        const sessionConf: Partial<Session> = {
            name: 'entry',
            isNsMode: true,
        };
        store.sessionStore.activeSession = await store.sessionStore.newSession(sessionConf);
    });
    await window.request('reset/window', {});
};
