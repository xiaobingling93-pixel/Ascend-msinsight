/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useState } from 'react';
import type { Session } from '../entity/session';
import type { CardMetaData } from '../entity/data';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { useTheme } from '@emotion/react';
import { themeInstance } from 'ascend-theme';
import { Input, Button } from 'ascend-components';
import { runInAction } from 'mobx';
import { Modal, message } from 'antd';
import { setCardAliasReq } from '../api/request';

const SetAliasTitleDiv = styled.div`
    font-size: 16px;
    font-weight: 500;
    margin: 0 0 20px;
    box-shadow: inset 0 -1px 0 0 ${(props): string => `${props.theme.splitLineColor}`};
    padding: 0 0 10px 10px;
`;

const FlexDiv = styled.div`
    display: flex;
    align-items: center;
`;

const AliasInputBox = styled(FlexDiv)`
    margin: 0 0 20px;
    flex-wrap: wrap;
    gap: 24px;
    padding: 0 24px;
`;

const ButtonContainer = styled.div`
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 20px;
    padding: 36px 0 30px;
`;

const SetAliasDiv = styled.div`
    pointer-events: auto;
    padding-top: 13px;
    box-shadow: 0 10px 100px 0 rgba(0,0,0,0.50);
    border-radius: 4px;
`;

const handleCancelSetAlias = (session: Session): void => {
    runInAction(() => {
        Modal.destroyAll();
    });
};

const handleConfirmSetAlias = (session: Session, newCardAlias: string): void => {
    runInAction(async () => {
        try {
            (session.selectedUnits[0].metadata as CardMetaData).label = newCardAlias;
            const { cardId } = session.selectedUnits[0].metadata as CardMetaData;
            await setCardAliasReq({ rankId: cardId, cardAlias: newCardAlias });
        } catch (error) {
            message.error('Set card alias error!');
        }
        Modal.destroyAll();
    });
};

export const SetAlias = observer(({ session }: {session: Session}) => {
    const [cardAlias, setCardAlias] = useState<string>('');
    const { t } = useTranslation('timeline');
    const [theme, setTheme] = useState(useTheme());
    React.useEffect(() => {
        if (theme !== themeInstance.getThemeType()) {
            setTheme(themeInstance.getThemeType());
        }
    }, [themeInstance.getThemeType()]);

    const cancelSetAlias = (): void => { handleCancelSetAlias(session); };
    const confirmSetAlias = (): void => { handleConfirmSetAlias(session, cardAlias); };

    return (
        <SetAliasDiv style={{ color: theme.textColorPrimary, background: theme.bgColorLight }}>
            <SetAliasTitleDiv>
                <span>{t('contextMenu.Set alias')}</span>
            </SetAliasTitleDiv>
            <AliasInputBox>
                <FlexDiv>
                    <span>{`${t('contextMenu.Alias')}:`}</span>
                </FlexDiv>
                <FlexDiv>
                    <Input
                        style={{ color: theme.textColorPrimary, backgroundColor: theme.bgColorLight }}
                        value={cardAlias}
                        onChange={(e): void => { setCardAlias(e.target.value); }}
                    />
                </FlexDiv>
            </AliasInputBox>
            <ButtonContainer>
                <Button onClick={ cancelSetAlias }>{t('timelineMarker:cancelButton')}</Button>
                <Button type={'primary'} onClick={ confirmSetAlias }>{t('timelineMarker:confirmButton')}</Button>
            </ButtonContainer>
        </SetAliasDiv>
    );
});
