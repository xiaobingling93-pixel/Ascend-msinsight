/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import * as React from 'react';
import { observer } from 'mobx-react';
import { useRef, useState } from 'react';
import { Modal } from 'antd';
import { runInAction } from 'mobx';
import styled from '@emotion/styled';
import type { Theme } from '@emotion/react';
import type { TFunction } from 'i18next';
import { useTranslation } from 'react-i18next';
import { ThemeProvider, useTheme } from '@emotion/react';
import { themeInstance } from 'ascend-theme';
import { adaptDpr } from 'ascend-utils';
import { Button } from 'ascend-components';
import type { Session } from '../entity/session';
import { getTimestamp } from '../utils/humanReadable';
import { registerCrossUnitRenderer } from './charts/ChartInteractor';
import { ReactComponent as GCIcon } from '../assets/images/insights/ark_gc.svg';
import type { TimelineAxisFlag } from '../entity/timeMaker';
import { ReactComponent as BrushIcon } from '../assets/images/timeline/ic_brush_black_lined.svg';
import { platform } from '../platforms';
import { useWatchResize } from '../utils/useWatchDomResize';
import type { TimeLineMakerProps } from '../utils/TimeMakerUtils';
import { TIME_MARKER_AXIS_HEIGHT } from './TimeMakerAxis';
import { ReactComponent as CloseIcon } from '../assets/images/insights/UIicon_closeFlagList.svg';
import type { TimeStamp } from '../entity/common';

interface DrawTimelineAxisFlag extends TimelineAxisFlag {
    offsetX: number;
    anotherOffsetX?: number;
}

const FLAG_DEFAULT_NAME_REG = /default-\d+/;
enum FLAG_TYPE {
    NORMAL = 0,
    START = 1,
    END = 2,
}
const FLAG_PATH = new Path2D('M3.83098304,1.33333333 C4.1069561,1.33333333 4.33067648,1.55705371 4.33067648,1.83302677 L4.33067648,14.1669732' +
    ' C4.33067648,14.4429463 4.1069561,14.6666667 3.83098304,14.6666667 C3.55500997,14.6666667 3.3312896,14.4429463 3.3312896,14.1669732' +
    ' L3.3312896,1.83302677 C3.3312896,1.55705371 3.55500997,1.33333333 3.83098304,1.33333333 Z M5.19768345,1.74044554 C6.64235918,1.27775407' +
    ' 7.99528737,1.38953933 9.2360342,2.07770724 L9.2360342,2.07770724 L9.22142861,2.06970097 L9.30617372,2.10533802 C10.2887351,2.50433059' +
    ' 11.1500438,2.42159423 11.9275976,1.85989258 L11.9275976,1.85989258 L12.0025846,1.80403924 C12.1798301,1.66793876 12.4353401,1.69941983' +
    ' 12.573234,1.87430962 C12.6287561,1.9447278 12.6589005,2.03139645 12.6589005,2.1206107 L12.6589005,2.1206107 L12.6589005,7.55986423' +
    ' C12.6589005,7.67672191 12.5869677,7.78187027 12.4771192,7.82558455 C10.915867,8.44688613 9.59848253,8.38324415 8.55407822,7.60848799' +
    ' C7.74832078,6.97558972 6.72041589,6.97558972 5.42079102,7.63882347 C5.22748177,7.73747441 4.9969344,7.59904941 4.9969344,7.38433194' +
    ' L4.9969344,7.38433194 L4.9969344,2.01296814 C4.9969344,1.88881143 5.07798608,1.77878145 5.19768345,1.74044554 Z');
const FLAG_ANGLE_PATH = new Path2D('M6.435091,10.4096942 C6.68055089,10.4096942 6.88469937,10.5865693 6.92703533,10.8198185 L6.935091,10.9096942' +
    ' L6.935,13.559 L12.4231469,13.5599916 C12.6686067,13.5599916 12.8727552,13.7368667 12.9150912,13.9701159 L12.9231469,14.0599916 C12.9231469,14.3054515' +
    ' 12.7462717,14.5095999 12.5130225,14.5519359 L12.4231469,14.5599916 L6.435091,14.5599916 C6.18963111,14.5599916 5.98548263,14.3831164' +
    ' 5.94314667,14.1498672 L5.935091,14.0599916 L5.935091,10.9096942 C5.935091,10.6335518 6.15894862,10.4096942 6.435091,10.4096942 Z');
const ANGLE_TRANSFORM_ORIGIN = [9.429119, 12.484843];
export function drawFlag(
    ctx: CanvasRenderingContext2D,
    x: number,
    color: string,
    type: FLAG_TYPE = FLAG_TYPE.NORMAL,
): void {
    ctx.fillStyle = color;
    ctx.strokeStyle = color;
    const iconPadding = 3;
    const [sx, sy] = [x - iconPadding, -1];
    ctx.translate(sx, sy);
    ctx.fill(FLAG_PATH);
    switch (type) {
        case FLAG_TYPE.START:
            ctx.translate(ANGLE_TRANSFORM_ORIGIN[0], ANGLE_TRANSFORM_ORIGIN[1]);
            ctx.scale(1, -1);
            ctx.translate(-ANGLE_TRANSFORM_ORIGIN[0], -ANGLE_TRANSFORM_ORIGIN[1]);
            ctx.stroke(FLAG_ANGLE_PATH);
            break;
        case FLAG_TYPE.END:
            ctx.translate(ANGLE_TRANSFORM_ORIGIN[0], ANGLE_TRANSFORM_ORIGIN[1]);
            ctx.scale(-1, 1);
            ctx.translate(-ANGLE_TRANSFORM_ORIGIN[0], -ANGLE_TRANSFORM_ORIGIN[1]);
            ctx.stroke(FLAG_ANGLE_PATH);
            break;
        default:
            break;
    }
    ctx.setTransform(1, 0, 0, 1, 0, 0); // 重置变换矩阵为默认值
}

const drawTimelineFlag = (ctx: CanvasRenderingContext2D, beginX: number, item: TimelineAxisFlag, session: Session): void => {
    if (item === undefined) { return; }
    drawFlag(ctx, beginX, item.color);
};

/**
 * 绘制跟随鼠标移动的旗子
 * 当鼠标移动到已存在的旗帜，高亮
 * @param ctx 画布
 * @param beginX 起始位置
 * @param highlightFlag 高亮的旗帜
 */
const drawTimeLineFlagForMouseMove = (ctx: CanvasRenderingContext2D | null, beginX: number, highlightFlag?: DrawTimelineAxisFlag): void => {
    if (ctx === null) { return; }
    // 若鼠标在存在的旗帜上，在其上叠加绘制一层
    if (highlightFlag) {
        drawFlag(ctx, highlightFlag.offsetX, highlightFlag.color);
        if (highlightFlag.anotherOffsetX) {
            drawFlag(ctx, highlightFlag.anotherOffsetX, highlightFlag.color);
        }
    } else {
        drawFlag(ctx, beginX, '#3778ED');
    }
};

/**
 * 绘制竖线（在旗子下方）
 *
 * @param ctx 画布内容对象
 * @param beginX 竖线绘制坐标
 * @param height 竖线高度
 * @param color 颜色
 */
const drawVerticalLineUnderFlag = (ctx: CanvasRenderingContext2D, beginX: number, height: number, color: string): void => {
    const sx = beginX + 0.7;
    ctx.beginPath();
    ctx.moveTo(sx, 0);
    ctx.setLineDash([4, 2]);
    ctx.strokeStyle = color;
    ctx.lineTo(sx, height);
    ctx.stroke();
};

const drawRangeFlag = ({ ctx, rangeStartX, rangeEndX, color }: IDrawRangeFlagParams): void => {
    drawFlag(ctx, rangeStartX, color, FLAG_TYPE.START);
    drawFlag(ctx, rangeEndX, color, FLAG_TYPE.END);
};

interface IDrawRangeFlagParams {
    ctx: CanvasRenderingContext2D;
    rangeStartX: number;
    rangeEndX: number;
    color: string;
}

interface IDrawFlagsParams {
    session: Session;
    domain: number[];
    canvas: HTMLCanvasElement;
    range: React.MutableRefObject<[number, number]>;
    t: TFunction;
    isHead: boolean; // 绘制旗帜头还是旗帜下的线，true 绘制旗帜头，false 绘制旗帜下的线
}

export const drawTimelineFlags = ({ session, domain, canvas, range, t, isHead }: IDrawFlagsParams): void => {
    // draw timeline flag
    const ctx = canvas.getContext('2d');
    if (!ctx || session.name === t('Realtime Monitor')) {
        return;
    }
    const { canvasWidth, canvasHeight } = adaptDpr(canvas, ctx);
    // 清空画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const flagList = session.timelineMaker.timelineFlagList;
    flagList?.forEach((item: TimelineAxisFlag) => {
        switch (item.type) {
            case 'point': {
                if (!(item.timeStamp >= domain[0] && item.timeStamp <= domain[1])) { return; }
                const x = transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth);
                isHead ? drawTimelineFlag(ctx, x, item, session) : drawVerticalLineUnderFlag(ctx, x, canvasHeight, item.color); // 在旗子下方绘制线条
                break;
            }
            case 'range': {
                if (item.anotherTimeStamp === undefined) { return; }
                // 判断标记中首尾时间戳是否有其中一个在范围内
                const isTimeStampInDomain = item.timeStamp >= domain[0] && item.timeStamp <= domain[1];
                const isAnotherTimeStampInDomain = item.anotherTimeStamp >= domain[0] && item.anotherTimeStamp <= domain[1];
                if (!(isTimeStampInDomain) && !(isAnotherTimeStampInDomain)) { return; }
                if (isHead) {
                    drawRangeFlag({
                        ctx,
                        rangeStartX: transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth),
                        rangeEndX: transformTimeToLeft(domain[0], domain[1], item.anotherTimeStamp, canvasWidth),
                        color: item.color,
                    });
                } else {
                    // 在范围边界的两个旗子下绘制竖线
                    drawVerticalLineUnderFlag(ctx, transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth), canvasHeight, item.color);
                    drawVerticalLineUnderFlag(ctx, transformTimeToLeft(domain[0], domain[1], item.anotherTimeStamp, canvasWidth), canvasHeight, item.color);
                }
                break;
            }
            default:
                break;
        }
    });
};

export const drawTimelineFlagAxisInvalidBackground = (canvas: HTMLCanvasElement, width: number, color: string): void => {
    const ctx = canvas.getContext('2d');
    if (!ctx) { return; }
    const { canvasWidth, canvasHeight } = adaptDpr(canvas, ctx);
    // 清空画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    if (width < canvasWidth) {
        ctx.fillStyle = color;
        ctx.fillRect(width, 0, canvasWidth, canvasHeight);
    }
};

export const transformTimeToLeft = (domainStart: number, domainEnd: number, timestamp: number, canvasWidth: number): number => {
    if (domainEnd === domainStart) {
        return 0;
    }
    return canvasWidth * (timestamp - domainStart) / (domainEnd - domainStart);
};

const addNewFlag = (session: Session, timeStamp: number, timeDisplay: string): void => {
    const maxNumber = generateDefaultNumber(session);
    const defaultDesc = `default-${maxNumber.toString()}`;
    const color = session.timelineMaker.timelineFlagColorList[maxNumber % session.timelineMaker.timelineFlagColorList.length];
    runInAction(() => {
        session.timelineMaker.selectedFlag = {
            uid: crypto.getRandomValues(new Uint32Array(3)).join('-'),
            timeStamp,
            timeDisplay,
            color,
            colorCache: color,
            description: defaultDesc,
            descriptionCache: defaultDesc,
            type: 'point',
            anotherTimeStamp: undefined,
        };
        session.timelineMaker.timelineFlagList.push(session.timelineMaker.selectedFlag);
        platform.trace('useTag', {});
        session.timelineMaker.timelineFlagList.sort((a, b): number => {
            return a.timeStamp - b.timeStamp;
        });
        // 通知时间轴进行标记绘图更新
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
    });
};

export const addRangeFlag = (session: Session, rangeStartTimeStamp: number, rangeStartDisplay: string, rangeEndTimeStamp: number): void => {
    const maxNumber = generateDefaultNumber(session);
    const defaultDesc = `default-${maxNumber.toString()}`;
    const color = session.timelineMaker.timelineFlagColorList[maxNumber % session.timelineMaker.timelineFlagColorList.length];
    const uid = crypto.getRandomValues(new Uint32Array(3)).join('-');
    runInAction(() => {
        const rangeStartFlag: TimelineAxisFlag = {
            uid,
            timeStamp: rangeStartTimeStamp,
            timeDisplay: rangeStartDisplay,
            color,
            colorCache: color,
            description: defaultDesc,
            descriptionCache: defaultDesc,
            type: 'range',
            anotherTimeStamp: rangeEndTimeStamp,
        };
        session.timelineMaker.timelineFlagList.push(rangeStartFlag);
        session.timelineMaker.timelineFlagList.sort((a, b): number => {
            return a.timeStamp - b.timeStamp;
        });
        // 通知时间轴进行标记绘图更新
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
    });
};

export const deleteRangeFlag = (session: Session, rangeStartTimestamp: TimeStamp, rangeEndTimestamp: TimeStamp): void => {
    runInAction(() => {
        for (let index = 0; index < session.timelineMaker.timelineFlagList.length; index++) {
            if (session.timelineMaker.timelineFlagList[index].anotherTimeStamp === undefined) { continue; }
            if (session.timelineMaker.timelineFlagList[index].timeStamp === rangeStartTimestamp &&
                session.timelineMaker.timelineFlagList[index].anotherTimeStamp === rangeEndTimestamp) {
                session.timelineMaker.timelineFlagList.splice(index, 1);
                session.timelineMaker.oldMarkedRange = undefined;
                // 通知时间轴进行标记绘图更新
                session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
                return;
            }
        }
    });
};

const generateDefaultNumber = (session: Session): number => {
    let maxNumber = 0;
    session.timelineMaker.timelineFlagList.forEach((item: TimelineAxisFlag) => {
        if (item.description.match(FLAG_DEFAULT_NAME_REG)) {
            const matchRes = item.description.split('-');
            const defaultNum = parseInt(matchRes[1]);
            if (defaultNum >= maxNumber) {
                maxNumber = defaultNum + 1;
            }
        }
    });
    return maxNumber;
};

const tableElementId = ['timeMakerList', 'singleFlagEdit', 'colorEditor', 'deleteALLConfirm'];

/**
 * 获取鼠标位置
 * @param e 鼠标事件
 * @param current 画布元素
 */
export const getMouse = (e: MouseEvent, current: HTMLCanvasElement): { x: number; y: number } | null => {
    const mouse = { x: 0, y: 0 };
    const rect = current.getBoundingClientRect();
    // 校验鼠标是否在画布中
    const isWidthIllegal = e.clientX > rect.right || e.clientX < rect.left;
    const isHeightIllegal = e.clientY < rect.top || e.clientY > rect.bottom;
    if (isWidthIllegal || isHeightIllegal) {
        return null;
    }
    // 计算鼠标在画布中的相对位置
    mouse.x = e.clientX - rect.left;
    mouse.y = e.clientY - rect.top;
    return mouse;
};

/**
 * 鼠标移动动作处理器
 *
 * @param e 鼠标事件
 * @param session session
 * @param range 区间大小
 * @param domains 可用时间区间
 * @param current canvas画布元素dom节点
 */
export const handleMouseMove = (e: MouseEvent, session: Session, range: React.MutableRefObject<[number, number]>,
    domains: number[], current: HTMLCanvasElement | null): void => {
    const [domainStart, domainEnd] = domains;
    const ctx = current?.getContext('2d');

    if (!current || !ctx) {
        return;
    }
    const mouse = getMouse(e, current);
    adaptDpr(current, ctx);
    // 清空画布
    ctx.clearRect(0, 0, current.width, current.height);

    // 判断鼠标位置是否为空，如果鼠标不在画布中，则直接返回
    if (!mouse) {
        return;
    }

    let highlightFlag: DrawTimelineAxisFlag | undefined;
    const xOffset = linearScaleFactory([domainStart, domainEnd], range.current);
    for (const flag of session.timelineMaker.timelineFlagList) {
        const flagXOffset = xOffset(flag.timeStamp);
        const anotherMarkerTimestamp = flag.anotherTimeStamp;
        // 如果鼠标移动到旗帜上、或者移动到旗帜区间的另一个旗帜上时
        if (isInFlagXOffsetRange(e.offsetX, flagXOffset) ||
            (anotherMarkerTimestamp !== undefined && isInFlagXOffsetRange(e.offsetX, xOffset(anotherMarkerTimestamp)))) {
            highlightFlag = {
                ...flag,
                offsetX: flagXOffset,
                anotherOffsetX: anotherMarkerTimestamp !== undefined ? xOffset(anotherMarkerTimestamp) : undefined,
            };
            break;
        }
    }

    current.style.cursor = highlightFlag ? 'pointer' : 'auto';
    drawTimeLineFlagForMouseMove(ctx, mouse.x, highlightFlag);
};

interface ISingleClickParams {
    t: TFunction;
    event: MouseEvent;
    session: Session;
    range: React.MutableRefObject<[number, number]>;
    domainStart: number;
    domainEnd: number;
    current: HTMLCanvasElement | null;
}

export const handleSingleClick = ({ t, event, session, range, domainStart, domainEnd, current }: ISingleClickParams): void => {
    if (['waiting', 'recording', 'analyzing'].includes(session.phase) || session.selectedRange !== undefined) { return; }
    const xScale = linearScaleFactory(range.current, [domainStart, domainEnd]);
    const timeStamp = Math.floor(xScale(event.offsetX));
    const timeDisplay = getTimestamp(timeStamp, { precision: session.isNsMode ? 'ns' : 'ms' });
    if (current === null || session.endTimeAll === undefined || timeStamp > session.endTimeAll) { return; }
    const ctx = current?.getContext('2d');
    const clickedElementStack = document.elementsFromPoint(event.clientX, event.clientY);
    if (clickedElementStack.length < 1 || clickedElementStack[0].tagName !== 'CANVAS') {
        // 点击不是从 canvas 触发的，直接返回
        return;
    }
    const flagCanvas = clickedElementStack.find(element => element.id === 'timelineFlagCnvas');
    const canvasNow = clickedElementStack.find(element => element.classList.contains('drawCanvas'));
    const tableElement = clickedElementStack.find(element => tableElementId.includes(element.id));
    if (!ctx || !flagCanvas) {
        if (canvasNow && !tableElement) {
            runInAction(() => {
                session.timelineMaker.selectedFlag = undefined;
                session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
            });
        } else {
            runInAction(() => {
                session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
            });
        }
        return;
    }
    // 已放置标记,选中标记
    const xOffset = linearScaleFactory([domainStart, domainEnd], range.current);
    for (let index = 0; index < session.timelineMaker.timelineFlagList.length; index++) {
        const flagXOffset = xOffset(session.timelineMaker.timelineFlagList[index].timeStamp);
        const anotherMakerTimeStamp = session.timelineMaker.timelineFlagList[index].anotherTimeStamp;
        if (isInFlagXOffsetRange(event.offsetX, flagXOffset)) {
            setSelectFlag(session, index, event.offsetX, xOffset);
            return;
        }
        if (anotherMakerTimeStamp !== undefined && isInFlagXOffsetRange(event.offsetX, xOffset(anotherMakerTimeStamp))) {
            setSelectFlag(session, index, event.offsetX, xOffset);
            return;
        }
        setSelectFlag(session, index, event.offsetX, xOffset);
    }
    // 未放置标记，则放置标记
    addNewFlag(session, timeStamp, timeDisplay);
};

const SingleFlagEditDiv = styled.div`
    pointer-events: auto;
    padding-top: 13px;
    width: 258px;
    box-shadow: 0 10px 100px 0 rgba(0,0,0,0.50);
    border-radius: 4px;
`;

const Describe = styled.div`
    margin-left: 19px;
    font-size: 12px;
    height: 19px;
    line-height: 19px;
`;

const EditInput = styled.input`
    outline: none;
    border: 0px;
    margin-left: 19px;
    margin-top: 9px;
    padding-right: 57px;
    height: 28px;
    width: 220px;
    border-radius: 3px;
    font-size: 12px;
    line-height: 19px;
    float: left
`;

const ColorText = styled.div`
    margin-left: 19px;
    margin-top: 11px;
    font-size: 12px;
    height: 19px;
    line-height: 19px;
`;

const InputLengthContainer = styled.div`
    float: left;
    position: absolute;
    top: 43px;
    right: 184px;
`;

const EditInputContainer = styled.div`
    height: 34px;
`;

const ButtonContainer = styled.div`
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 20px;
    padding: 36px 0 30px;
`;

const SingleFlagEditElement = observer((props: TimeLineMakerProps): JSX.Element => {
    const { t } = useTranslation();
    const index = props.index;
    const session = props.session;
    if (index === undefined) { return <></>; }
    const timelineAxisFlag = session.timelineMaker.timelineFlagList[index];
    const [theme, setTheme] = useState(useTheme());
    React.useEffect(() => {
        if (theme !== themeInstance.getThemeType()) {
            setTheme(themeInstance.getThemeType());
        }
    }, [themeInstance.getThemeType()]);
    const colorBoxClickListener = (event: React.MouseEvent<HTMLDivElement, MouseEvent>): void => handleColorBoxClick(theme, event, timelineAxisFlag);
    const diyColorListener = (): void => handleColorSelect(theme, session, timelineAxisFlag);
    const confirmListener = (): void => handleConfirm(session, timelineAxisFlag);
    const deleteListener = (): void => handleDelete(session, index);
    const cancelListener = (): void => handleCancel(session, index);
    const inputChangeListener = (e: React.ChangeEvent<HTMLInputElement>): void => handleEditInputChange(e, timelineAxisFlag, theme);
    return timelineAxisFlag !== undefined
        ? <SingleFlagEditDiv id={ 'singleFlagEdit' } style={{ background: theme.bgColorLight }}>
            <Describe style={{ color: theme.svgPlayBackgroundColor }}>{t('timelineMarker:description')}<CloseIcon style={{ float: 'right', marginRight: '15px', fill: '#71757F' }} onClick={cancelListener}></CloseIcon></Describe>
            <EditInputContainer>
                <EditInput style={{ color: theme.svgPlayBackgroundColor, backgroundColor: theme.searchBackgroundColor }} defaultValue={ timelineAxisFlag.descriptionCache } onChange={inputChangeListener} type={ 'text' }/>
                <InputLengthContainer>
                    <span id={ 'inputLength' } style={{ fontSize: '10px', color: timelineAxisFlag.descriptionCache.length === 256 ? '#D94838' : theme.svgPlayBackgroundColor }}>{ timelineAxisFlag.descriptionCache.length }</span>
                    <span style={{ fontSize: '10px', color: theme.svgPlayBackgroundColor }}>/</span>
                    <span style={{ fontSize: '10px', color: theme.svgPlayBackgroundColor }}>256</span></InputLengthContainer></EditInputContainer>
            <ColorText style={{ color: theme.svgPlayBackgroundColor }} id={ 'colorText' }>{t('timelineMarker:color')}</ColorText>
            <div id={ 'colorBoxes' } style={{ height: '30px', width: '220px', marginLeft: '19px' }}>
                { session.timelineMaker.timelineFlagColorList.map((item: string, idx: number) => {
                    let needAddFlagColor = true;
                    if (idx < 23) {
                        if (item === timelineAxisFlag.color) {
                            needAddFlagColor = false;
                        }
                        if (idx === 23 && needAddFlagColor) {
                            return <div key={idx} onClick={colorBoxClickListener} id = { timelineAxisFlag.color } style={{ float: 'left', backgroundColor: item, height: '12px', width: '12px', marginTop: '6px', marginRight: '6px', border: theme.colorSelectedBorder }}></div>;
                        } else {
                            return <div key={idx} onClick={colorBoxClickListener} id = { item } style={{ float: 'left', backgroundColor: item, height: '12px', width: '12px', marginTop: '6px', marginRight: '6px', border: item === timelineAxisFlag.colorCache ? theme.colorSelectedBorder : 'none' }}></div>;
                        }
                    }
                    return '';
                }) }
                <BrushIcon id={ 'brushIconBox' } onClick={ diyColorListener } style={{ fill: theme.svgPlayBackgroundColor, marginTop: '6px', marginRight: '6px' }}/>
            </div>
            <ButtonContainer>
                <Button type={'primary'} onClick={ confirmListener }>{t('timelineMarker:confirmButton')}</Button>
                <Button onClick={ deleteListener }>{t('timelineMarker:deleteButton')}</Button>
            </ButtonContainer>
        </SingleFlagEditDiv>
        : <></>;
});

const handleEditInputChange = (e: React.ChangeEvent<HTMLInputElement>, timelineAxisFlag: TimelineAxisFlag, theme: Theme): void => {
    if (e.target.value === undefined) {
        return;
    }
    if (e.target.value.length > 256) {
        e.target.value = e.target.value.substring(0, 256);
    }
    runInAction(() => {
        timelineAxisFlag.descriptionCache = e.target.value;
    });
    const inputLength = document.getElementById('inputLength');
    if (inputLength !== null) {
        inputLength.innerText = timelineAxisFlag.descriptionCache.length.toString();
    }
};

const handleColorBoxClick = (theme: Theme, event: React.MouseEvent<HTMLDivElement, MouseEvent>, timelineAxisFlag: TimelineAxisFlag): void => {
    const id = event.currentTarget.id;
    selectColorBox(theme, id, timelineAxisFlag);
};

const selectColorBox = (theme: Theme, id: string, timelineAxisFlag: TimelineAxisFlag): void => {
    document.getElementById('colorBoxes')?.querySelectorAll('div').forEach((element) => {
        if (element.id !== id) {
            element.style.border = 'none';
            return;
        }
        element.style.border = theme.colorSelectedBorder;
        runInAction(() => {
            timelineAxisFlag.colorCache = element.id;
        });
    });
};

const handleColorSelect = (theme: Theme, session: Session, timelineAxisFlag: TimelineAxisFlag): void => {
    const colorInput = document.getElementById('colorText')?.querySelector('input');
    if (colorInput) {
        colorInput.click();
        return;
    }
    const input = document.createElement('input');
    input.type = 'color';
    input.style.zIndex = '999';
    document.getElementById('colorText')?.appendChild(input);
    const handleBlurEvent = (e: Event): void => {
        const color = (e.target as HTMLInputElement).value;
        runInAction(() => {
            timelineAxisFlag.colorCache = color;
        });
        handleNewColor(theme, session, color, timelineAxisFlag);
        selectColorBox(theme, color, timelineAxisFlag);
        input.remove();
    };
    // mac不支持监听blur事件需要加上PaletteDidChange事件
    input.addEventListener('PaletteDidChange', handleBlurEvent);
    input.addEventListener('blur', handleBlurEvent);
    setTimeout(() => {
        input.click();
        input.focus();
    }, 100);
};

const handleNewColor = (theme: Theme, session: Session, newColor: string, timelineAxisFlag: TimelineAxisFlag): void => {
    for (let index = 0; index < session.timelineMaker.timelineFlagColorList.length; index++) {
        if (newColor === session.timelineMaker.timelineFlagColorList[index]) {
            // 已存在的颜色，直接返回
            return;
        }
    }
    // 色块插入新颜色移除旧颜色
    const colorBoxParent = document.getElementById('colorBoxes');
    const brushIconBox = document.getElementById('brushIconBox');
    if (!colorBoxParent || !brushIconBox) {
        return;
    }
    const colorBoxClickListener = (event: React.MouseEvent<HTMLDivElement, MouseEvent>): void => handleColorBoxClick(theme, event, timelineAxisFlag);
    const newColorBox = document.createElement('div');
    newColorBox.style.width = '12px';
    newColorBox.style.height = '12px';
    newColorBox.style.float = 'left';
    newColorBox.style.backgroundColor = newColor;
    newColorBox.style.marginTop = '6px';
    newColorBox.style.marginRight = '6px';
    newColorBox.style.border = theme.colorSelectedBorder;
    newColorBox.id = newColor;
    newColorBox.onclick = (e): (e: React.MouseEvent<HTMLDivElement, MouseEvent>) => void => colorBoxClickListener;
    runInAction(() => {
        if (session.timelineMaker.timelineFlagColorList.length >= 23) {
            const removeColor = session.timelineMaker.timelineFlagColorList[0];
            document.getElementById(removeColor)?.remove();
            session.timelineMaker.timelineFlagColorList.splice(0, 1);
        }
        session.timelineMaker.timelineFlagColorList.push(newColor);
    });
};

const handleConfirm = (session: Session, timelineAxisFlag: TimelineAxisFlag): void => {
    runInAction(() => {
        timelineAxisFlag.color = timelineAxisFlag.colorCache;
        timelineAxisFlag.description = timelineAxisFlag.descriptionCache;
        Modal.destroyAll();
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
    });
};

const handleDelete = (session: Session, index: number): void => {
    runInAction(() => {
        if (session.timelineMaker.timelineFlagList[index].type === 'range') {
            session.timelineMaker.oldMarkedRange = undefined;
        } else {
            session.timelineMaker.selectedFlag = undefined;
        }
        session.timelineMaker.timelineFlagList.splice(index, 1);
        Modal.destroyAll();
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
    });
};

const handleCancel = (session: Session, index: number): void => {
    const selectedFlag = session.timelineMaker.timelineFlagList[index];
    selectedFlag.colorCache = selectedFlag.color;
    selectedFlag.descriptionCache = selectedFlag.description;
    Modal.destroyAll();
};

export const handleDoubleClick = (
    e: MouseEvent, session: Session, range: React.MutableRefObject<[number, number]>, domainStart: number, domainEnd: number,
): void => {
    const clickedElementStack = document.elementsFromPoint(e.clientX, e.clientY);
    if (clickedElementStack.length < 1 || clickedElementStack[0].tagName !== 'CANVAS') {
        // 点击不是从 canvas 触发的，直接返回
        return;
    }
    const canvasNow = clickedElementStack.find(t => t.classList.contains('timeMakerAxis'));
    if (!canvasNow) {
        return;
    }
    const xOffset = linearScaleFactory([domainStart, domainEnd], range.current);
    for (let index = 0; index < session.timelineMaker.timelineFlagList.length; index++) {
        const flagXOffset = Math.floor(xOffset(session.timelineMaker.timelineFlagList[index].timeStamp));
        const anotherMarkerTimestamp = session.timelineMaker.timelineFlagList[index].anotherTimeStamp;
        if (isInFlagXOffsetRange(e.offsetX, flagXOffset)) {
            popupEditor(session, index);
            return;
        }
        if (anotherMarkerTimestamp !== undefined && isInFlagXOffsetRange(e.offsetX, xOffset(anotherMarkerTimestamp))) {
            // 框选标记双击框选结束时的标记也会弹出编辑框
            popupEditor(session, index);
            return;
        }
    }
};

const popupEditor = (session: Session, index: number): void => {
    Modal.confirm({
        modalRender: () => <ThemeProvider theme={themeInstance.getThemeType()}>
            <SingleFlagEditElement session={session} index={index}/>
        </ThemeProvider>,
        maskClosable: false,
    });
};

const isInFlagXOffsetRange = (mouseOffsetX: number, flagOffsetX: number): boolean => {
    return mouseOffsetX >= flagOffsetX - 3 && mouseOffsetX <= flagOffsetX + 13;
};

const setSelectFlag = (session: Session, index: number, mouseXOffset: number, xOffset: (x: number) => number): void => {
    runInAction(() => {
        const flagItem = session.timelineMaker.timelineFlagList[index];
        const flagXOffset = Math.floor(xOffset(flagItem.timeStamp));
        if (flagItem.anotherTimeStamp !== undefined) {
            const anotherMarkerXOffset = Math.floor(xOffset(flagItem.anotherTimeStamp));
            if (isInFlagXOffsetRange(mouseXOffset, flagXOffset) || isInFlagXOffsetRange(mouseXOffset, anotherMarkerXOffset)) {
                const rangeStart = flagItem.timeStamp < flagItem.anotherTimeStamp ? flagItem.timeStamp : flagItem.anotherTimeStamp;
                const rangeEnd = flagItem.timeStamp > flagItem.anotherTimeStamp ? flagItem.timeStamp : flagItem.anotherTimeStamp;
                session.timelineMaker.oldMarkedRange = [rangeStart, rangeEnd];
                session.selectedRange = [rangeStart, rangeEnd];
            }
        } else {
            if (isInFlagXOffsetRange(mouseXOffset, flagXOffset)) {
                session.timelineMaker.selectedFlag = session.timelineMaker.timelineFlagList[index];
                // 通知时间轴进行标记绘图更新
                session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
            }
        }
    });
};

export const linearScaleFactory = (from: [number, number], to: [number, number]) => {
    return (x: number): number => {
        if (from[1] === from[0]) {
            return x;
        }
        const scale = (to[1] - to[0]) / (from[1] - from[0]);
        return ((x - from[0]) * scale) + to[0];
    };
};

interface TimelineMarkerProps {
    session: Session;
    theme: Theme;
};

registerCrossUnitRenderer({
    action: (ctx, session, xScale) => {
        const selectedFlag = session.timelineMaker.selectedFlag; // should filter on data type
        if (ctx !== null && selectedFlag !== undefined) {
            ctx.beginPath();
            ctx.moveTo(xScale(selectedFlag.timeStamp), 40);
            ctx.setLineDash([4, 2]);
            ctx.strokeStyle = '#5291FF';
            ctx.lineTo(xScale(selectedFlag.timeStamp), 9999);
            ctx.stroke();
            ctx.setLineDash([]);
        }
    },
    triggers: session => [session.timelineMaker.selectedFlag],
});

// 点击gc按钮绘制gc图标提示
registerCrossUnitRenderer({
    action: (ctx, session, xScale, theme) => {
        const gcActionTimeStamp = session.sharedState?.GCActionTimeStamp as number[] | undefined;
        if (ctx !== null && gcActionTimeStamp !== undefined) {
            gcActionTimeStamp.forEach(itemTimeStamp => {
                const img = new Image();
                let svg: SVGSVGElement | undefined;
                document.querySelectorAll('svg').forEach(svgItem => {
                    if (svgItem.id !== 'gc_icon') {
                        return;
                    }
                    svg = svgItem;
                });
                if (!svg) {
                    return;
                }
                const svgString = new XMLSerializer().serializeToString(svg);
                const svgData = `data:image/svg+xml;base64,${btoa(svgString)}`;
                img.src = svgData;
                ctx.drawImage(img, xScale(itemTimeStamp), 32);
            });
        }
    },
    triggers: session => [
        session.sharedState?.GCActionTimeStamp,
    ],
});

const CanvasContainer = styled.div`
    width: 100%;
`;

export const TimelineMarkerElement = observer(({ session, theme }: TimelineMarkerProps): JSX.Element => {
    const { t } = useTranslation();
    const range = useRef<[ number, number ]>([0, 0]);
    const canvas = React.useRef<HTMLCanvasElement>(null);
    const { domainStart, domainEnd } = session.domainRange;
    const [width, ref] = useWatchResize<HTMLDivElement>('width');
    // 竖线画板高度，根据DOM树的laneView节点高度动态调整
    const height = session.totalHeight;
    const [verticalHeight, vertical] = useWatchResize<HTMLCanvasElement>('height');
    const flagCursor = React.useRef<HTMLCanvasElement>(null);
    const background = React.useRef<HTMLCanvasElement>(null);

    React.useEffect(() => {
        const mouseMoveListener = (e: MouseEvent): void => handleMouseMove(e, session, range, [domainStart, domainEnd], flagCursor.current);
        addEventListener('mousemove', mouseMoveListener);
        return () => {
            removeEventListener('mousemove', mouseMoveListener);
        };
    }, [domainStart, domainEnd]);

    React.useEffect(() => {
        if (!canvas.current) {
            return (): void => {};
        }
        const singleClickListener =
            (e: MouseEvent): void => handleSingleClick({ t, event: e, session, range, domainStart, domainEnd, current: canvas.current });
        const doubleClickListener = (e: MouseEvent): void => handleDoubleClick(e, session, range, domainStart, domainEnd);
        if (session.name !== t('Realtime Monitor')) {
            addEventListener('click', singleClickListener);
            addEventListener('dblclick', doubleClickListener);
        }
        // 绘制旗帜头
        drawTimelineFlags({ session, domain: [domainStart, domainEnd], canvas: canvas.current, range, t, isHead: true });
        range.current = [0, canvas.current.clientWidth];
        return () => {
            removeEventListener('click', singleClickListener);
            removeEventListener('dblclick', doubleClickListener);
        };
    }, [width, domainStart, domainEnd, session.timelineMaker.refreshTrigger, session.selectedRange]);

    // 当 height 改变，导致 verticalCanvas 高度改变
    // 改变 canvas 的尺寸会导致浏览器重新分配其位图数据，原有的绘图数据不会被保留
    // 因此需要监听 verticalHeight 改变后重绘
    React.useEffect(() => {
        if (!vertical.current) { return; }
        // 绘制旗帜下的线
        drawTimelineFlags({ session, domain: [domainStart, domainEnd], canvas: vertical.current, range, t, isHead: false });
    }, [verticalHeight, width, domainStart, domainEnd, session.timelineMaker.refreshTrigger, session.selectedRange]);

    // 绘制不可点击的区间
    React.useEffect(() => {
        if (!session.endTimeAll || background.current === null || session.name === t('Realtime Monitor')) { return; }
        const xScale = linearScaleFactory([domainStart, domainEnd], range.current);
        const maxX = Math.floor(xScale(session.endTimeAll));
        drawTimelineFlagAxisInvalidBackground(background.current, maxX, theme.bgColorLight);
    }, [width, domainStart, domainEnd, session.endTimeAll, theme]);
    return <CanvasContainer ref={ref}>
        <canvas ref={background} width={width} height={TIME_MARKER_AXIS_HEIGHT}
            style={{ width, height: TIME_MARKER_AXIS_HEIGHT, position: 'absolute', top: 0, left: 0 }}/>
        <canvas
            id="timelineFlagCnvas"
            ref={canvas}
            width={width}
            height={TIME_MARKER_AXIS_HEIGHT}
            style={{ width, height: TIME_MARKER_AXIS_HEIGHT, position: 'absolute', top: 0, left: 0 }}/>
        <GCIcon style={{ display: 'none' }} id="gc_icon" fill={themeInstance.getThemeType().buttonFontColor}/>
        <canvas ref={flagCursor} width={width} height={TIME_MARKER_AXIS_HEIGHT}
            style={{ width, height: TIME_MARKER_AXIS_HEIGHT, position: 'absolute', top: 0, left: 0 }}/>
        <canvas ref={vertical} width={width} height={height}
            style={{ width, height, position: 'absolute', top: 15, left: 0 }}/>
    </CanvasContainer>;
});

const confirmClickHandler = (timelineAxisFlag: TimelineAxisFlag, session: Session, setFlagColor: (flagColor: string) => void): void => {
    runInAction(() => {
        timelineAxisFlag.color = timelineAxisFlag.colorCache;
        timelineAxisFlag.description = timelineAxisFlag.descriptionCache;
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
        setFlagColor(timelineAxisFlag.colorCache);
    });
    const maskDiv = document.getElementsByClassName('ant-modal-root');
    maskDiv[1].parentNode?.removeChild(maskDiv[1]);
};

const closeEditorWindow = (): void => {
    const maskDiv = document.getElementsByClassName('ant-modal-root');
    maskDiv[1].parentNode?.removeChild(maskDiv[1]);
};

export const ColorEditor = observer((props: TimeLineMakerProps): JSX.Element => {
    const timelineAxisFlag = props.item;
    const setFlagColor = props.setFlagColor;
    if (!timelineAxisFlag || !setFlagColor) {
        return <></>;
    }
    const session = props.session;
    const colorBoxClickListener = (event: React.MouseEvent<HTMLDivElement, MouseEvent>): void => handleColorBoxClick(theme, event, timelineAxisFlag);
    const diyColorListener = (): void => handleColorSelect(theme, session, timelineAxisFlag);
    const [theme, setTheme] = useState(useTheme());
    const { t } = useTranslation();
    React.useEffect(() => {
        if (theme !== themeInstance.getThemeType()) {
            setTheme(themeInstance.getThemeType());
        }
    }, [themeInstance.getThemeType()]);
    return <SingleFlagEditDiv id={ 'colorEditor' } style={{ backgroundColor: theme.bgColorLight, height: '150px' }}>
        <ColorText id={ 'colorText' } style={{ color: theme.svgPlayBackgroundColor, userSelect: 'none' }}>{t('timelineMarker:color')}</ColorText >
        <div id={ 'colorBoxes' } style={{ height: '30px', width: '220px', marginLeft: '19px' }}>
            { session.timelineMaker.timelineFlagColorList.map((item: string, index: number) => {
                let needAddFlagColor = true;
                if (index < 23) {
                    if (item === timelineAxisFlag.color) {
                        needAddFlagColor = false;
                    }
                    if (index === 23 && needAddFlagColor) {
                        return <div key={index} onClick={colorBoxClickListener} id = { timelineAxisFlag.color } style={{ float: 'left', backgroundColor: item, height: '12px', width: '12px', marginTop: '6px', marginRight: '6px', border: theme.colorSelectedBorder }}></div>;
                    } else {
                        return <div key={index} onClick={colorBoxClickListener} id = { item } style={{ float: 'left', backgroundColor: item, height: '12px', width: '12px', marginTop: '6px', marginRight: '6px', border: item === timelineAxisFlag.colorCache ? theme.colorSelectedBorder : 'none' }}></div>;
                    }
                }
                return '';
            }) }
            <BrushIcon id={ 'brushIconBox' } onClick={ diyColorListener } style={{ fill: theme.svgPlayBackgroundColor, marginTop: '6px', marginRight: '6px' }}/>
        </div>
        <ButtonContainer>
            <Button type={'primary'} onClick={ (): void => {
                confirmClickHandler(timelineAxisFlag, session, setFlagColor);
            }}>
                {t('timelineMarker:confirmButton')}
            </Button>
            <Button onClick={closeEditorWindow}>
                {t('timelineMarker:cancelButton')}
            </Button>
        </ButtonContainer>
    </SingleFlagEditDiv>;
});

export const changeRangeMarkerTimestamp = (session: Session, newRange: [TimeStamp, TimeStamp]): void => {
    // 未放置range标记则无需修改
    if (session.timelineMaker.oldMarkedRange === undefined) { return; }
    const rangeStart = newRange[0] < newRange[1] ? newRange[0] : newRange[1];
    const rangeEnd = newRange[0] > newRange[1] ? newRange[0] : newRange[1];
    const oldRange = session.timelineMaker.oldMarkedRange;
    session.timelineMaker.timelineFlagList.forEach((item) => {
        if (item.anotherTimeStamp === undefined) { return; }
        if (item.timeStamp === oldRange[0] && item.anotherTimeStamp === oldRange[1]) {
            runInAction(() => {
                if (item.anotherTimeStamp === undefined) { return; }
                item.timeStamp = rangeStart;
                item.timeDisplay = getTimestamp(rangeStart, { precision: session.isNsMode ? 'ns' : 'ms' });
                item.anotherTimeStamp = rangeEnd;
                session.timelineMaker.oldMarkedRange = [rangeStart, rangeEnd];
                // 通知时间轴进行标记绘图更新
                session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
            });
        }
    });
};
