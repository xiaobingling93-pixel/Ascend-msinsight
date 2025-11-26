/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { InputRef } from 'antd';
import { StyledInput } from '../../../components/base/StyledInput';
import React, { RefObject, useRef, useState, useEffect } from 'react';
import type { ChangeEvent } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import type { Session } from '../../../entity/session';
import styled from '@emotion/styled';
import { Tooltip } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { ThreadTraceRequest } from '../../../entity/data';
import { getTimeOffset, getTimeOffsetKey } from '../utils';
import { CustomButton } from '../../../components/base/StyledButton';
import type { SvgType } from '../../../components/base/rc-table/types';
import { ReactComponent as AlignStartIcon } from '../../../assets/images/timeline/ic_align_start.svg';
const AlignIcon = AlignStartIcon as SvgType;
const defaultOffset = '0';
const MAX_OFFSET_TIME = 30 * 24 * 60 * 60 * 1000_000_000; // 30 天，单位 ns
const minOffset = -MAX_OFFSET_TIME;
const maxOffset = MAX_OFFSET_TIME;
const defaultBorderColor = '#838383FF';
const inputBorderColor = '#1890ff';
const invalidBorderColor = '#C61E37FF';
const inputBorderShadow = '0 0 0 2px rgba(24, 144, 255, 0.2)';
const invalidBorderShadow = '0 0 0 2px rgba(255, 0, 0, 0.2)';

const OffsetButton = styled.div`
    color: ${(props): string => props.theme.primaryColor};
    cursor: pointer;
`;

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

function hasStringValue(str: string = ''): boolean {
    return str !== '';
}

// 设置一级泳道偏移量时，同步其二级泳道的偏移量
export function handleTimestampOffsetReassignment(
    session: Session,
    cardMetaData: ThreadTraceRequest,
    inputValue: number,
): boolean {
    if (cardMetaData.processId === null || !hasStringValue(cardMetaData.processId) || session.isTimeAnalysisMode) {
        const cardId = cardMetaData.cardId;
        const timestampOffsetConfig = { ...session.unitsConfig.offsetConfig.timestampOffset };
        const offsetKeys = Object.keys(timestampOffsetConfig);
        let isReassigned = false;

        const timestampOffsetKey = getTimeOffsetKey(session, cardMetaData);
        timestampOffsetConfig[timestampOffsetKey] = inputValue;
        for (const key of offsetKeys) {
            if (key.startsWith(`${cardId}__`) && timestampOffsetConfig[key] !== inputValue) {
                timestampOffsetConfig[key] = inputValue;
                isReassigned = true;
            }
        }
        session.setTimestampOffsetAll(timestampOffsetConfig);
        return isReassigned;
    }
    return false;
}

function checkValue(inputElement: HTMLInputElement, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>,
    setVisible: React.Dispatch<React.SetStateAction<boolean>>, metaData: any): void {
    const inputValue = Number(inputElement.value);
    if (!isNaN(inputValue) && (inputValue >= minOffset && inputValue <= maxOffset)) {
        const cardMetaData = (metaData as ThreadTraceRequest);
        const timestampOffsetKey = getTimeOffsetKey(session, metaData as ThreadTraceRequest);
        // preValue = curValue, directly return
        const preTimestampOffset = getTimeOffset(session, cardMetaData);
        const isReassigned = handleTimestampOffsetReassignment(session, cardMetaData, inputValue);

        if (preTimestampOffset === inputValue && !isReassigned) {
            return;
        }
        session.setTimestampOffset(timestampOffsetKey, inputValue);
        session.setDomainWithoutHistory({ domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration });
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
    padding: 5px 10px;
    color: ${(props): string => props.theme.fontColor}
`;

const InputDiv = styled.div`
    align-items: flex-start;
    justify-content: space-between;
    display: flex;
    .ant-input-disabled {
        background-color: ${(props): string => props.theme.templateBackgroundColor};
        border: none;
    }
`;

function handleAlignStart(inputRef: RefObject<InputRef>, session: Session, setValue: React.Dispatch<React.SetStateAction<string>>): void {
    const alignStartTimestamp = session.selectedUnits[0]?.alignStartTimestamp ?? session.selectedUnits[0]?.parent?.alignStartTimestamp;
    if (alignStartTimestamp === undefined) {
        return;
    }
    setValue(alignStartTimestamp.toString());
    inputRef?.current?.focus();
}

export const InputOption = observer(({ session, metaData, onClick }: { session: Session; metaData: any; onClick?: () => void }): JSX.Element => {
    const timestampOffsetKey = getTimeOffsetKey(session, metaData as ThreadTraceRequest);
    const timestampOffset = (session.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[timestampOffsetKey] ?? 0;
    const [offset, setOffset] = useState(String(timestampOffset));
    const [visible, setVisible] = useState(false);
    const [title, setTitle] = useState('Please enter a proper value');
    const inputRef = useRef<InputRef>(null);
    const { t } = useTranslation();
    useEffect(() => {
        setOffset(String(timestampOffset));
    }, [timestampOffset]);
    useEffect(() => {
        if (session.isTimeAnalysisMode) {
            // 时间范围分析模式，需要将卡级别的偏移量同步到进程级别上做统一分析
            handleTimestampOffsetReassignment(session, metaData, timestampOffset);
        }
    }, [session.isTimeAnalysisMode]);
    return <Tooltip
        trigger={'click'}
        placement={'bottom'}
        title={
            <InputContainer>
                <InputDiv>
                    <div className="flex-none">{t('Timestamp Offset', { ns: 'timeline' })}(ns):</div>
                    <div>
                        <StyledInput minwidth={20} height={18} width={155} isshow={1} value={offset} disabled={session.phase === 'analyzing'} ref={inputRef} maxLength={500}
                            onChange={(e): void => onChange({ e, session, setOffset, setVisible, setTitle, t })}
                            onBlur={(e): void => onBlur(e, session, setOffset, setVisible, metaData)}
                            onFocus={onFocus}
                            onPressEnter={(e): void => onPressEnter(session, setOffset, setVisible, e, metaData)}
                        />
                        {visible && <div>{title}</div>}
                    </div>
                    <CustomButton aria-label="align to start" tooltip={t('Align to Start', { ns: 'timeline' })} icon={AlignIcon} type="primary" onClick={(): void => handleAlignStart(inputRef, session, setOffset)} />
                </InputDiv>
            </InputContainer>}
        overlayInnerStyle={{ borderRadius: 2 }}>
        <OffsetButton data-testid={'offset-btn'} onClick={onClick}>{t('Offset', { ns: 'timeline' })}</OffsetButton>
    </Tooltip>
    ;
});

export const offsetConfig = (session: Session, metadata: any, onClick?: () => void): JSX.Element => {
    return <InputOption session={session} metaData={metadata} onClick={onClick} />;
};
