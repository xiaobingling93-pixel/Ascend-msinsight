/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { InputRef } from 'antd';
import { StyledInput } from '../../../components/base/StyledInput';
import { StyledSelect } from '../../../components/base/StyledSelect';
import React, { useRef, useState } from 'react';
import type { ChangeEvent } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { Session } from '../../../entity/session';
import styled from '@emotion/styled';
import { StyledTooltip } from '../../../components/base/StyledTooltip';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { useTheme } from '@emotion/react';
import { CardMetaData, ThreadTraceRequest } from '../../../entity/data';
import { getTimeOffset } from '../utils';
import { CustomButton } from '../../../components/base/StyledButton';
import type { SvgType } from '../../../components/base/rc-table/types';
import { ReactComponent as AlignStartIcon } from '../../../assets/images/timeline/ic_align_start.svg';
import { message } from 'antd';
const AlignIcon = AlignStartIcon as SvgType;

const defaultOffset = '0';
const minOffset = -Number.MAX_VALUE;
const maxOffset = Number.MAX_VALUE;
const defaultBorderColor = '#838383FF';
const inputBorderColor = '#1890ff';
const invalidBorderColor = '#C61E37FF';
const inputBorderShadow = '0 0 0 2px rgba(24, 144, 255, 0.2)';
const invalidBorderShadow = '0 0 0 2px rgba(255, 0, 0, 0.2)';

// Changing the border color of the input box when the input value is invalid
const onChange = ({ e, session, setOffset, setVisible, setTitle, t }: {
    e: ChangeEvent<HTMLInputElement>; session: Session;
    setOffset: React.Dispatch<React.SetStateAction<string>>;
    setVisible: React.Dispatch<React.SetStateAction<boolean>>;
    setTitle: React.Dispatch<React.SetStateAction<string>>;
    t: TFunction;
}): void => {
    runInAction(() => {
        setOffset(e.target.value);
        const inputValue = Number(e.target.value);
        if (!isNaN(inputValue) && inputValue >= minOffset && inputValue <= maxOffset) {
            e.target.style.borderColor = inputBorderColor;
            e.target.style.boxShadow = inputBorderShadow;
            setVisible(false);
        } else {
            e.target.style.borderColor = invalidBorderColor;
            e.target.style.boxShadow = invalidBorderShadow;
            setTitle(t('headerButtonTooltip:TimeStampOffset') ?? '');
            setVisible(true);
        }
    });
};

function checkValue(inputElement: HTMLInputElement, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>,
    setVisible: React.Dispatch<React.SetStateAction<boolean>>, metaData: any): void {
    const inputValue = Number(inputElement.value);
    if (!isNaN(inputValue) && (inputValue >= minOffset && inputValue <= maxOffset)) {
        const cardMetaData = (metaData as CardMetaData);
        // preValue = curValue, directly return
        const preTimestampOffset = getTimeOffset(session, cardMetaData.cardId);
        if (preTimestampOffset === inputValue) {
            return;
        }
        const prevObj = session.unitsConfig.offsetConfig.timestampOffset as Object;
        session.unitsConfig.offsetConfig.timestampOffset = { ...prevObj, [cardMetaData.cardId]: (inputValue) };
    } else {
        setValue(defaultOffset);
    }
    inputElement.style.borderColor = defaultBorderColor;
    inputElement.style.boxShadow = 'none';
    setVisible(false);
}

// The input value in the input box is changed to the default value when the focus is lost and the input value is invalid.
const onBlur = (e: ChangeEvent<HTMLInputElement>, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>,
    setVisible: React.Dispatch<React.SetStateAction<boolean>>, metaData: any): void => {
    runInAction(() => {
        checkValue(e.target, session, setValue, setVisible, metaData);
    });
};

const onFocus = (e: ChangeEvent<HTMLInputElement>): void => {
    runInAction(() => {
        e.target.style.borderColor = inputBorderColor;
        e.target.style.boxShadow = inputBorderShadow;
    });
};

const onPressEnter = (session: Session, setValue: React.Dispatch<React.SetStateAction<string>>,
    setVisible: React.Dispatch<React.SetStateAction<boolean>>,
    e: React.KeyboardEvent<HTMLInputElement> | React.MouseEvent<SVGSVGElement, MouseEvent>, metaData: any): void => {
    runInAction(() => {
        const input = (e.target as HTMLInputElement);
        checkValue(input, session, setValue, setVisible, metaData);
        session.renderTrigger = !session.renderTrigger;
    });
};

const InputContainer = styled.div`
    width: 380px;
    padding: 5px 10px 5px 10px;
    color: ${(props): string => props.theme.fontColor}
`;

const InputDiv = styled.div`
    align-items: center;
    justify-content: space-between;
    display: flex;
    .ant-input-disabled {
        background-color: ${(props): string => props.theme.templateBackgroundColor};
        border: none;
    }
`;

const InputSpan = styled.span`
    text-align: right;
    width: 170px;
`;

function handleAlignStart(inputRef: InputRef, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>): void {
    const alignStartTimestamp = session.selectedUnits[0]?.alignStartTimestamp;
    if (alignStartTimestamp === undefined) {
        message.warning('Please expand the card first');
        return;
    }
    setValue(alignStartTimestamp.toString());
    inputRef?.current?.focus();
}

const InputOption = observer(({ session, metaData }: { session: Session; metaData: any }): JSX.Element => {
    const cardId = (metaData as ThreadTraceRequest).cardId;
    const defaultOffset = (session.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[cardId] ?? 0;
    const [offset, setOffset] = useState(String(defaultOffset));
    const [visible, setVisible] = useState(false);
    const [title, setTitle] = useState('Please enter a proper value');
    const inputRef = useRef<InputRef>(null);
    const { t } = useTranslation();
    return <StyledSelect
        width={100}
        height={18}
        value={t('Offset', { ns: 'timeline' })}
        dropdownRender={(): React.ReactElement => <InputContainer>
            <StyledTooltip title={title} visible={visible} overlayInnerStyle={{ borderRadius: 8, color: useTheme().fontColor }}>
                <InputDiv>
                    <InputSpan>{t('Timestamp Offset', { ns: 'timeline' })}(ns):</InputSpan>
                    <StyledInput minwidth={20} height={18} width={155} isshow={1} value={offset} disabled={session.phase === 'analyzing'} ref={inputRef} maxLength={500}
                        onChange={(e): void => onChange({ e, session, setOffset, setVisible, setTitle, t })}
                        onBlur={(e): void => onBlur(e, session, setOffset, setVisible, metaData)}
                        onFocus={onFocus}
                        onPressEnter={(e): void => onPressEnter(session, setOffset, setVisible, e, metaData)}/>
                    <CustomButton tooltip={t('Align to Start', { ns: 'timeline' })} icon={AlignIcon} type="primary" onClick={(): void => handleAlignStart(inputRef, session, setOffset)} />
                </InputDiv>
            </StyledTooltip>
        </InputContainer>}>
    </StyledSelect>;
});

export const offsetConfig = (session: Session, metadata: any): JSX.Element => {
    return <InputOption session={session} metaData={metadata} />;
};
