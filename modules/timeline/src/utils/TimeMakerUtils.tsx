/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { Tooltip } from 'lib/components';
import { Modal } from 'antd';
import React, { useState, type MouseEvent } from 'react';
import type { TimelineAxisFlag } from '../entity/timeMaker';
import { ReactComponent as CloseIcon } from '../assets/images/insights/UIicon_closeFlagList.svg';
import { ReactComponent as TimeDiffIcon } from '../assets/images/ic_public_timeDiff.svg';
import { ReactComponent as DeleteIcon } from '../assets/images/ic_public_delete.svg';
import { ReactComponent as JumpingIcon } from '../assets/images/rectangle.svg';
import type { Session } from '../entity/session';
import { ColorEditor } from '../components/TimelineMarker';
import { getTimeDifference } from '../components/charts/ChartInteractor';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { ThemeProvider, useTheme } from '@emotion/react';
import type { Theme } from '@emotion/react';
import { themeInstance } from '../theme/theme';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { StyledButton } from '../components/base/StyledButton';
import { runInAction } from 'mobx';
import { getTimestamp } from './humanReadable';
import { StyledTooltip } from '../components/base/StyledTooltip';

export interface TimeLineMakerProps {
    session: Session;
    item?: TimelineAxisFlag;
    setFlagColor?: (flagColor: string) => void;
    index?: number;
    onToolTipVisibleChange?: (open: boolean) => void;
}

// 调色窗口和功能
const handleSelectColor = (item: TimelineAxisFlag, props: TimeLineMakerProps, setFlagColor: (flagColor: string) => void): void => {
    Modal.confirm({
        modalRender: () => <ThemeProvider theme={themeInstance.getThemeType()}>
            <ColorEditor session={props.session} setFlagColor={setFlagColor} item={item}/>
        </ThemeProvider>,
        centered: true,
    });
};

// 删除列表全部按钮
const deleteAll = (props: TimeLineMakerProps): void => {
    const flagList = props.session.timelineMaker.timelineFlagList;
    if (flagList.length === 0) {
        return;
    }
    Modal.confirm({
        modalRender: () => <ThemeProvider theme={themeInstance.getThemeType()}>
            <DeleteALLConfirm session={props.session}/>
        </ThemeProvider>,
        maskClosable: true,
        centered: true,
    });
};

export const handleTimeMakerAction = (props: TimeLineMakerProps): void => {
    const onToolTipVisibleChange = props.onToolTipVisibleChange;
    if (onToolTipVisibleChange === undefined) {
        return;
    }
    onToolTipVisibleChange(true);
    Modal.confirm({
        modalRender: () => <ThemeProvider theme={themeInstance.getThemeType()}>
            <TimeMakerListElement session={props.session} onToolTipVisibleChange={onToolTipVisibleChange}/>
        </ThemeProvider>,
        maskClosable: true,
        closable: true,
        onCancel: () => {
            onToolTipVisibleChange(false);
        },
        afterClose: () => {
            // 关闭列表时，如果标记选中的时间轴标记的selectedFlag选中的是框选标记，则置为undefined
            if (props.session.timelineMaker.selectedFlag?.anotherTimeStamp !== undefined) {
                runInAction(() => {
                    props.session.timelineMaker.selectedFlag = undefined;
                });
            }
        },
    });
};

const getTitleDisplay = (props: TimeLineMakerProps, t: TFunction): string => {
    const selectedFlag = props.session.timelineMaker.selectedFlag;
    if (selectedFlag === undefined) {
        return t('timelineMarker:unselected');
    } else {
        return selectedFlag.description;
    }
};

interface HandleDeleteParams {
    event: MouseEvent;
    deleteSignal: number;
    uid: string;
    props: TimeLineMakerProps;
    setDeleteSignal: (deleteSignal: number) => void;
    setTimeDiff: (timeDiff: string) => void;
    t: TFunction;
}

// 单击删除行
const handleDelete = ({ event, deleteSignal, uid, props, setDeleteSignal, setTimeDiff, t }: HandleDeleteParams): void => {
    event?.stopPropagation();
    const flagList = props.session.timelineMaker.timelineFlagList;
    const selectedFlag = props.session.timelineMaker.selectedFlag;
    const newDeleteSignal = (deleteSignal + 1) % 10;
    if (selectedFlag?.uid === uid) {
        runInAction(() => {
            props.session.timelineMaker.selectedFlag = undefined;
        });
        setTimeDiff(t('timelineMarker:unselected'));
    }
    for (let i = 0; i < flagList.length; i++) {
        if (flagList[i].uid !== uid) {
            continue;
        }
        const markedRange = props.session.timelineMaker.oldMarkedRange;
        const isFlagList = markedRange !== undefined && flagList[i].anotherTimeStamp !== undefined &&
            flagList[i].timeStamp === markedRange[0] && flagList[i].anotherTimeStamp === markedRange[1];
        if (isFlagList) {
            runInAction(() => {
                props.session.timelineMaker.oldMarkedRange = undefined;
            });
        }
        flagList.splice(i, 1);
    }
    setDeleteSignal(newDeleteSignal);
    runInAction(() => {
        props.session.timelineMaker.refreshTrigger += 1;
    });
};

// 选中某行
const handleSelected = (item: TimelineAxisFlag, props: TimeLineMakerProps, theme: Theme, setSelectFlag: (item: TimelineAxisFlag) => void): void => {
    runInAction(() => {
        props.session.timelineMaker.selectedFlag = item;
        if (item.anotherTimeStamp !== undefined) {
            props.session.selectedRange = [item.timeStamp, item.anotherTimeStamp];
            props.session.timelineMaker.oldMarkedRange = [item.timeStamp, item.anotherTimeStamp];
        }
    });
    const flagTableElements = document.getElementById('flagTable')?.children;
    if (flagTableElements === undefined) {
        return;
    }
    for (let i = 0; i < flagTableElements.length; i++) {
        const currentRow = flagTableElements[i] as HTMLElement;
        currentRow.style.backgroundColor = theme.tooltipBGColor;
        if (currentRow.id === item.uid) {
            currentRow.style.backgroundColor = theme.flagListSelectedColor;
        }
    }
    setSelectFlag(item);
};

const mouseMoveListener = (event: MouseEvent, props: TimeLineMakerProps, theme: Theme, setTimeDiff: (timeDiff: string) => void): void => {
    const target = event.currentTarget;
    const flagList = props.session.timelineMaker.timelineFlagList;
    const selectedFlag = props.session.timelineMaker.selectedFlag;
    if (target.className !== 'singleTimeMakerRow' || !selectedFlag) {
        return;
    }
    const selectedTimeStamp = selectedFlag.timeStamp;
    const hoverTimeStamp = flagList.find(timelineFlag => timelineFlag.uid === target.id)?.timeStamp;
    if (hoverTimeStamp === undefined) {
        return;
    }
    const flagTableElements = document.getElementById('flagTable')?.children;
    if (flagTableElements === undefined) {
        return;
    }
    for (let i = 0; i < flagTableElements.length; i++) {
        const currentRow = flagTableElements[i] as HTMLElement;
        currentRow.style.backgroundColor = theme.tooltipBGColor;
        if (currentRow.id === selectedFlag.uid) {
            currentRow.style.backgroundColor = theme.flagListSelectedColor;
        }
        if (currentRow.id === target.id && currentRow.id !== selectedFlag.uid) {
            currentRow.style.backgroundColor = theme.flagListHoverColor;
        }
    }
    const timeStampRange = [selectedTimeStamp, hoverTimeStamp];
    timeStampRange.sort((a, b) => a - b);
    const timeDifference = getTimeDifference(Math.floor(timeStampRange[0]), Math.floor(timeStampRange[1]), props.session.isNsMode);
    setTimeDiff(timeDifference);
};

const showDescTooTip = (e: React.MouseEvent<SVGTextElement>, setShouldShowDescToolTip: (shouldShowDescToolTip: boolean) => void): void => {
    const descTitleElement = document.getElementById('selectedMarkerDec');
    if (descTitleElement?.scrollWidth === undefined || descTitleElement.clientWidth === undefined ||
        descTitleElement.scrollWidth <= descTitleElement.clientWidth) {
        setShouldShowDescToolTip(false);
        return;
    }
    setShouldShowDescToolTip(true);
};

const MarkerListBody = styled.div`
    user-select: none;
    pointer-events: auto;
    padding-top: 11px;
    box-shadow: 0 10px 100px 0 rgba(0,0,0,0.50);
    border-radius: 16px;
`;

const TitleText = styled.text`
    height: 17px;
    width: 90%;
    flex-Grow: 1;
    font-Weight: bold;
    overflow: hidden;
    white-Space: nowrap;
    text-Overflow: ellipsis;
`;
const MarkerListText = styled.div`
    padding-left: 4%;
    font-size: 12px;
    height: 22px;
    line-height: 19px;
`;
const MarkerListTr = styled.tr`
    padding-left: 4%;
    font-size: 12px;
    height: 19px;
    line-height: 19px;
    padding-right: 6%;
`;
const ListTitleText = styled.div`
    padding-left: 7%;
    margin-top: 20px;
    font-size: 16px;
    height: 19px;
    line-height: 19px;
    text-align: center;
`;

const ConfirmButton = styled.button`
    margin-left: 12%;
    margin-top: 50px;
    height: 28px;
    width: 88px;
    background: #5291FF;
    border: 0px;
    border-radius: 20px;
    text-align: center;
    font-size: 12px;
    line-height: 28px;
`;

const CancelButton = styled.button`
    margin-left: 8%;
    margin-top: 50px;
    height: 28px;
    width: 88px;
    border: 0px;
    border-radius: 20px;
    text-align: center;
    font-size: 12px;
    line-height: 28px;
`;

const MarkerTable = styled.div`
    padding-left: 4%;
    font-size: 12px;
    padding-right: 4%;
    height: 130px;
    overflow-Y: scroll;
    .tableIcon {
        cursor: pointer;
        margin: 3px 0px 0 5px;
    }
`;

interface MarkerBodyProps {
    theme: Theme;
    timeLineMarkerProps: TimeLineMakerProps;
    list: TimelineAxisFlag[];
    setTimeDiff: React.Dispatch<React.SetStateAction<string>>;
    setSelectFlag: React.Dispatch<React.SetStateAction<TimelineAxisFlag | undefined>>;
    setFlagColor: React.Dispatch<React.SetStateAction<string>>;
    setDeleteSignal: React.Dispatch<React.SetStateAction<number>>;
    deleteSignal: number;
}
const MarkerBody = (props: MarkerBodyProps): JSX.Element => {
    const { theme, list, timeLineMarkerProps, setTimeDiff, setSelectFlag, setFlagColor, setDeleteSignal, deleteSignal } = props;
    const { t } = useTranslation();
    return <MarkerTable id={'flagTable'} style={{ color: theme.fontColor }} >
        {
            list.map((item) => (
                <div key ={item.uid} id={item.uid} onMouseMove = {(event): void => mouseMoveListener(event, timeLineMarkerProps, theme, setTimeDiff)}
                    className = {'singleTimeMakerRow'} style={{ backgroundColor: item.uid === timeLineMarkerProps.session.timelineMaker.selectedFlag?.uid ? theme.flagListSelectedColor : theme.tooltipBGColor, borderBottom: '1px solid #979797' }}
                    onClick={(): void => {
                        handleSelected(item, timeLineMarkerProps, theme, setSelectFlag);
                        setTimeDiff(getTimeDifference(0, 0, timeLineMarkerProps.session.isNsMode));
                    }
                    }>
                    <div style={{ display: 'flex' }}>
                        <div style={{ width: '40%', flexGrow: 1, overflow: 'hidden', whiteSpace: 'nowrap', textOverflow: 'ellipsis', lineHeight: item.anotherTimeStamp === undefined ? undefined : '38px' }}> { item.description }</div>
                        { handleMakerTimeDisplay(timeLineMarkerProps.session, item) }
                        <div style={{ width: '15%' }}><div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginTop: item.anotherTimeStamp === undefined ? '0px' : '11px' }}>
                            <div id={`${item.uid}color`} style={{ width: '12px', height: '12px', backgroundColor: item.color, cursor: 'pointer' }} onClick={(): void => handleSelectColor(item, timeLineMarkerProps, setFlagColor)} />
                            <DeleteIcon id={'deleteButton'} style={{ fill: theme.svgPlayBackgroundColor, cursor: 'pointer' }} onClick={(event): void => handleDelete({ event, deleteSignal, uid: item.uid, props: timeLineMarkerProps, setDeleteSignal, setTimeDiff, t })} />
                            <SvgComponent theme={theme} onClick = {(): void => { handleJump(item, timeLineMarkerProps.session); }} SvgElement={JumpingIcon}/>
                        </div>
                        </div>
                    </div>
                </div>
            ))
        }
    </MarkerTable>;
};

const handleMakerTimeDisplay = (session: Session, item: TimelineAxisFlag): JSX.Element => {
    const anotherMakerTimestamp = item.anotherTimeStamp;
    if (anotherMakerTimestamp === undefined) {
        return <div style={{ width: '40%' }}> {item.timeDisplay.toString()}</div>;
    }
    const anotherMakerTimeDisplay = getTimestamp(anotherMakerTimestamp, { precision: session.isNsMode ? 'ns' : 'ms' });
    return <div style={{ width: '40%' }}>
        <div>{item.timeDisplay.toString()}</div>
        <div>{anotherMakerTimeDisplay.toString()}</div>
    </div>;
};

interface SvgComponentProps {
    theme: Theme;
    onClick: (event?: React.MouseEvent) => void;
    SvgElement: React.ComponentType;
}
const SvgComponent: React.FC<SvgComponentProps> = ({ theme, onClick, SvgElement }) => (
    <svg onClick={ (event): void => { onClick(event); } } width={16}
        height={16} style={{ cursor: 'pointer' }} className={'JumpSvgIcon'}>
        <style>
            {`
        .JumpSvgIcon #Rectangle {
          fill: ${theme.svgPlayBackgroundColor};
        }
        .JumpSvgIcon path {
          fill: ${theme.selectedChartColor};
        }
      `}
        </style>
        <SvgElement/>
    </svg>
);
const handleJump = (item: TimelineAxisFlag, session: Session): void => {
    const domain = session.domain;
    if (item.anotherTimeStamp === undefined) {
        // 时间轴标记跳转
        if ((item.timeStamp >= session.domainRange.domainStart &&
            item.timeStamp <= session.domainRange.domainEnd) || session.endTimeAll === undefined) {
            return;
        };
        runInAction(() => {
            const start = Math.max(0, item.timeStamp - (domain.duration / 2));
            const end = Math.min(session.endTimeAll ?? 0, item.timeStamp + (domain.duration / 2));
            session.domainRange = { domainStart: start, domainEnd: end };
        });
    } else {
        // 框选标记跳转
        const selectedDuration = Math.abs(item.anotherTimeStamp - item.timeStamp);
        let domainStart = domain.domainRange.domainStart;
        let domainEnd = domain.domainRange.domainEnd;
        if (selectedDuration > domain.duration) {
            domainStart = item.timeStamp;
            domainEnd = item.anotherTimeStamp;
        } else {
            if (item.timeStamp < domain.domainRange.domainStart || item.anotherTimeStamp > domain.domainRange.domainEnd) {
                const middlePointTimestamp = (item.anotherTimeStamp + (item.timeStamp / 2));
                const start = Math.max(0, middlePointTimestamp - (domain.duration / 2));
                const end = Math.min(session.endTimeAll ?? 0, middlePointTimestamp + (domain.duration / 2));
                domainStart = end === session.endTimeAll ? end - domain.duration : start;
                domainEnd = start === 0 ? start + domain.duration : end;
            }
        }
        runInAction(() => {
            if (item.anotherTimeStamp === undefined) {
                return;
            }
            session.domainRange = { domainStart, domainEnd };
            session.selectedRange = [item.timeStamp, item.anotherTimeStamp];
            session.timelineMaker.oldMarkedRange = [item.timeStamp, item.anotherTimeStamp];
        });
    }
};

const TimeMakerListElement = observer((props: TimeLineMakerProps): JSX.Element => {
    const { t } = useTranslation();
    const [list] = useState(props.session.timelineMaker.timelineFlagList);
    const [selectedFlag, setSelectFlag] = useState(props.session.timelineMaker.selectedFlag);
    const [timeDiff, setTimeDiff] = useState(selectedFlag ? getTimeDifference(0, 0, props.session.isNsMode) : t('timelineMarker:unselected') as string);
    const [flagColor, setFlagColor] = useState('');
    const [deleteSignal, setDeleteSignal] = useState(0);
    const [theme, setTheme] = useState(useTheme());
    const [shouldShowDescToolTip, setShouldShowDescToolTip] = useState(false);
    React.useEffect(() => {
        if (theme !== themeInstance.getThemeType()) {
            setTheme(themeInstance.getThemeType());
        }
    }, [selectedFlag, list, timeDiff, flagColor, deleteSignal, themeInstance.getThemeType()]);

    return <MarkerListBody id="timeMakerList" style={{ backgroundColor: theme.tooltipBGColor, width: '400px', height: '263px' }}>
        <MarkerListText style={{ display: 'flex', fontSize: '14px' }}>
            <Tooltip visible={ shouldShowDescToolTip } id = "descToolTip" arrowContent={ null } arrowPointAtCenter={ false } placement={ 'topLeft' } destroyTooltipOnHide={ true } color={ theme.timeMakerListToolTipBackgroundColor } overlayStyle={{ fontSize: '12px', wordBreak: 'break-all' }} overlayInnerStyle={{ color: 'rgba(255, 255, 255, 0.9)', borderRadius: '16px', width: '345px' }} title={ getTitleDisplay(props, t) }>
                <TitleText id = "selectedMarkerDec"
                    onMouseOver={
                        (e): void => showDescTooTip(e, setShouldShowDescToolTip)
                    }
                    onMouseLeave={ (): void => {
                        setShouldShowDescToolTip(false);
                    } } style={{ color: theme.fontColor }}>
                    {getTitleDisplay(props, t)}
                </TitleText></Tooltip>
            <StyledButton style={{ width: '10%', backgroundColor: theme.tooltipBGColor }} icon={<CloseIcon style={{ fill: '#71757F' }}></CloseIcon>} onClick={(): void => { closeConfirm(props); }}></StyledButton>
        </MarkerListText>
        <MarkerListText style={{ color: theme.fontColor, opacity: '0.6', lineHeight: '10x' }}>{t('timelineMarker:guide')}</MarkerListText>
        <MarkerListTr style={{ display: 'flex', color: theme.fontColor, textAlign: 'left' }}>
            <th style={{ flexGrow: 1, width: '40%' }}>{t('timelineMarker:marker')}</th>
            <th style={{ width: '40%' }}>{t('timelineMarker:time')}</th>
            <th style={{ width: '15%' }}>{t('timelineMarker:operation')}</th>
        </MarkerListTr>
        <MarkerBody theme={theme} timeLineMarkerProps={props} list={list} setTimeDiff={setTimeDiff}
            setSelectFlag={setSelectFlag} setFlagColor={setFlagColor} setDeleteSignal={setDeleteSignal} deleteSignal={deleteSignal}/>
        <MarkerListText>
            <div id={'bottomActionGroup'} style={{ color: theme.svgPlayBackgroundColor, display: 'flex' }} >
                <div style={{ paddingTop: '5%', width: props.session.timelineMaker.selectedFlag ? '70%' : '60%' }}><DeleteIcon style={{ cursor: 'pointer', width: '20px', height: '20px', fill: theme.svgPlayBackgroundColor, verticalAlign: 'bottom' }} onClick={(): void => deleteAll(props)}></DeleteIcon><text style={{ cursor: 'pointer', paddingLeft: '10px' }} onClick={(): void => deleteAll(props)}>{t('timelineMarker:clear')}</text></div>
                <StyledTooltip title={t('timelineMarker:markerDiff')} placement={'bottom'}>
                    <div style={{ paddingTop: '5%' }}><TimeDiffIcon style={{ width: '20px', height: '20px', fill: theme.svgPlayBackgroundColor, verticalAlign: 'bottom' }}></TimeDiffIcon><text id={ 'timeDiffDisplay' } style={{ paddingLeft: '5px' }}>{timeDiff}</text></div>
                </StyledTooltip>
            </div>
        </MarkerListText>
    </MarkerListBody>
    ;
});

const DeleteALLConfirm = observer((props: TimeLineMakerProps): JSX.Element => {
    const list = props.session.timelineMaker.timelineFlagList;
    const [theme, setTheme] = useState(useTheme());
    const { t } = useTranslation();
    React.useEffect(() => {
        if (theme !== themeInstance.getThemeType()) {
            setTheme(themeInstance.getThemeType());
        }
    }, [themeInstance.getThemeType()]);
    return (
        <MarkerListBody id="deleteALLConfirm" style={{ backgroundColor: theme.tooltipBGColor, color: theme.svgPlayBackgroundColor, width: '258px', height: '150px' }} >
            <ListTitleText >{t('timelineMarker:confirmClear')}</ListTitleText>
            <ConfirmButton onClick={(): void => {
                list.splice(0);
                Modal.destroyAll();
                runInAction(() => {
                    props.session.timelineMaker.selectedFlag = undefined;
                    props.session.timelineMaker.refreshTrigger = (props.session.timelineMaker.refreshTrigger + 1) % 10;
                });
            }}>{t('timelineMarker:confirmButton')}</ConfirmButton>
            <CancelButton style={{ background: theme.solidLine }} onClick={(): void => {
                const maskDiv = document.getElementsByClassName('ant-modal-root');
                maskDiv[1].parentNode?.removeChild(maskDiv[1]);
            }}>
                {t('timelineMarker:cancelButton')}
            </CancelButton>
        </MarkerListBody>
    );
});

const closeConfirm = (props: TimeLineMakerProps): void => {
    const onToolTipVisibleChange = props.onToolTipVisibleChange;
    if (onToolTipVisibleChange === undefined) {
        return;
    }
    onToolTipVisibleChange(false);
    Modal.destroyAll();
};
