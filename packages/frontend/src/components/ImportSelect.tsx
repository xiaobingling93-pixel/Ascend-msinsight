import React from 'react';
import { Session } from '../entity/session';
import { messageSender } from '../connection/messageSender';
import { runInAction } from 'mobx';
import { CardUnit } from '../insight/units/AscendUnit';
import styled from '@emotion/styled';
import { StyledSelect } from './base/StyledSelect';
import { observer } from 'mobx-react';

const ChildrenContainer = styled.div`
    color: ${props => props.theme.fontColor};
    text-align: left;
    padding-left: 20px;
    user-select: none;
`;

type SelectProps = {
    session: Session;
};

const ImportModeSelect = observer((props: SelectProps) => {
    const { session } = props;

    function dropdownRender(): JSX.Element {
        return (
            <ChildrenContainer>
                <div key={'file'} onClick={() => selectFolders(session, true)} >Open File</div>
                <div key={'folder'} onClick={() => selectFolders(session, false)}>Open Folder</div>
            </ChildrenContainer>
        );
    }

    return (
        <StyledSelect
            defaultValue="Import Data"
            dropdownRender={dropdownRender}
            height={24} width={140} itemPaddingLeft={0}>
        </StyledSelect>
    );
});

type CardInfo = {
    cardName: string;
    rankId: string;
    result: boolean;
};

export const selectFolders = async (session: Session, isSelectFile: boolean): Promise<void> => {
    const selectedPath = isSelectFile ? await messageSender.selectFile() : await messageSender.selectFolder();
    const result = await window.request('import/action', { path: selectedPath });
    runInAction(() => {
        session.phase = 'download';
        session.endTimeAll = 1000000000;
        result.cards.forEach((item: CardInfo) => {
            const unit = new CardUnit({ cardId: item.rankId, cardName: item.cardName });
            if (item.result as boolean) {
                unit.phase = 'analyzing';
            } else {
                unit.phase = 'error';
            }
            session.units.push(unit);
        });
        session.allRankIds = result.cards.map((item: any) => item.rankId);
    });
};

export default ImportModeSelect;
