import { StyledInput } from '../../../components/base/StyledInput';
import { StyledSelect } from '../../../components/base/StyledSelect';
import React, { ChangeEvent, useState } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { Session } from '../../../entity/session';
import styled from '@emotion/styled';
import { StyledTooltip } from '../../../components/base/StyledTooltip';
import i18n from '../../../i18n';
import { useTheme } from '@emotion/react';
import { CardMetaData, ThreadTraceRequest } from '../../../entity/data';

const defaultOffset = '0';
const minOffset = -Number.MAX_VALUE;
const maxOffset = Number.MAX_VALUE;
const defaultBorderColor = '#838383FF';
const inputBorderColor = '#1890ff';
const invalidBorderColor = '#C61E37FF';
const inputBorderShadow = '0 0 0 2px rgba(24, 144, 255, 0.2)';
const invalidBorderShadow = '0 0 0 2px rgba(255, 0, 0, 0.2)';

// Changing the border color of the input box when the input value is invalid
const onChange = (e: ChangeEvent<HTMLInputElement>, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>,
    setVisible: React.Dispatch<React.SetStateAction<boolean>>,
    setTitle: React.Dispatch<React.SetStateAction<string>>): void => {
    runInAction(() => {
        setValue(e.target.value);
        const inputValue = Number(e.target.value);
        if (!isNaN(inputValue) && inputValue >= minOffset && inputValue <= maxOffset) {
            e.target.style.borderColor = inputBorderColor;
            e.target.style.boxShadow = inputBorderShadow;
            setVisible(false);
        } else {
            e.target.style.borderColor = invalidBorderColor;
            e.target.style.boxShadow = invalidBorderShadow;
            setTitle(i18n.t('headerButtonTooltip:TimeStampOffset') ?? '');
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
        const preTimestampOffset = cardMetaData.cardId !== undefined
            ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[cardMetaData.cardId] ?? 0
            : 0;
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
    });
};

const InputContainer = styled.div`
    width: 340px;
    padding: 5px 10px 5px 10px;
    color: ${props => props.theme.fontColor}
`;

const InputDiv = styled.div`
    align-items: center;
    justify-content: space-between;
    display: flex;
    .ant-input-disabled {
        background-color: ${props => props.theme.templateBackgroundColor};
        border: none;
    }
`;

const InputSpan = styled.span`
    text-align: right;
    width: 170px;
`;

const InputOption = observer(({ session, metaData }: { session: Session; metaData: any }): JSX.Element => {
    const cardId = (metaData as ThreadTraceRequest).cardId;
    const defaultOffset = (session.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[cardId] ?? 0;
    const [ offset, setOffset ] = useState(String(defaultOffset));
    const [ visible, setVisible ] = useState(false);
    const [ title, setTitle ] = useState('Please enter a proper value');
    return <StyledSelect
        width={100}
        height={18}
        value={'Offset'}
        dropdownRender={() => <InputContainer>
            <StyledTooltip title={title} visible={visible} overlayInnerStyle={{ borderRadius: 8, color: useTheme().fontColor }}>
                <InputDiv>
                    <InputSpan>Timestamp Offset(ns):</InputSpan>
                    <StyledInput minwidth={20} height={18} width={155} isshow={1} value={offset} disabled={session.phase === 'analyzing'}
                        onChange={(e) => onChange(e, session, setOffset, setVisible, setTitle)}
                        onBlur={(e) => onBlur(e, session, setOffset, setVisible, metaData)}
                        onFocus={onFocus}
                        onPressEnter={(e) => onPressEnter(session, setOffset, setVisible, e, metaData)}/>
                </InputDiv>
            </StyledTooltip>
        </InputContainer>}>
    </StyledSelect>;
});

export const offsetConfig = (session: Session, metadata: any): JSX.Element => {
    return <InputOption session={session} metaData={metadata} />;
};
