import { Session } from '../entity/session';
import * as React from 'react';
import { getTimestamp } from '../utils/humanReadable';
import { observer } from 'mobx-react';
import { registerCrossUnitRenderer } from './charts/ChartInteractor';
import { useRef, useState } from 'react';
import { ReactComponent as TimeLineFlagSvg } from '../assets/images/timeline/timelineFlag.svg';
import { ReactComponent as RangeEndFlagSvg } from '../assets/images/timeline/ic_range_end_flag.svg';
import { ReactComponent as RangeStartFlagSvg } from '../assets/images/timeline/ic_range_start_flag.svg';
import { ReactComponent as GCIcon } from '../assets/images/insights/ark_gc.svg';
import { TimelineAxisFlag } from '../entity/timeMaker';
import { ReactComponent as BrushIcon } from '../assets/images/timeline/ic_brush_black_lined.svg';
import { Modal } from 'antd';
import { runInAction } from 'mobx';
import { platform } from '../platforms';
import styled from '@emotion/styled';
import { useWatchResize } from '../utils/useWatchDomResize';
import type { Theme } from '@emotion/react';
import { TimeLineMakerProps } from '../utils/TimeMakerUtils';
import { ThemeProvider, useTheme } from '@emotion/react';
import { themeInstance } from '../theme/theme';
import type { TFunction } from 'i18next';
import { useTranslation } from 'react-i18next';
import { TIME_MARKER_AXIS_HEIGHT } from './TimeMakerAxis';
import { ReactComponent as CloseIcon } from '../assets/images/insights/UIicon_closeFlagList.svg';
import { TimeStamp } from '../entity/common';
import { adaptDpr } from 'lib/CommonUtils';

const FLAG_DEFAULT_NAME_REG = /default-\d+/;
const SCROLLBAR_WIDTH = 7;

const drawTimelineFlag = (ctx: CanvasRenderingContext2D, beginX: number, item: TimelineAxisFlag, session: Session, svg: SVGSVGElement): void => {
    svg.style.fill = item.color;
    svg.style.stroke = 'none';
    const svgString = new XMLSerializer().serializeToString(svg);
    const svgData = 'data:image/svg+xml;base64,' + btoa(svgString);
    const icon = new Image();
    icon.src = svgData;
    icon.onload = (): void => {
        ctx.drawImage(icon, beginX - 3, 0);
    };
};

/**
 * 绘制跟随鼠标移动的旗子
 * @param ctx 画布
 * @param beginX 起始位置
 * @param svg svg图
 */
const drawTimeLineFlagForMouseMove = (ctx: CanvasRenderingContext2D | null, beginX: number, svg: SVGSVGElement): void => {
    if (ctx === null || svg === null) {
        return;
    }
    svg.style.fill = '#3778ED';
    svg.style.stroke = 'none';
    const svgString = new XMLSerializer().serializeToString(svg);
    const svgData = `data:image/svg+xml;base64,${btoa(svgString)}`;
    const icon = new Image();
    icon.src = svgData;
    ctx.drawImage(icon, beginX - 3, 0);
};

/**
 * 绘制竖线（在旗子下方）
 *
 * @param ctx 画布内容对象
 * @param beginX 竖线绘制坐标
 * @param color 颜色
 */
const drawVerticalLineUnderFlag = (ctx: CanvasRenderingContext2D, beginX: number, color: string): void => {
    ctx.beginPath();
    ctx.moveTo(beginX + 1, 0);
    ctx.setLineDash([4, 2]);
    ctx.strokeStyle = color;
    ctx.lineTo(beginX, 9999);
    ctx.stroke();
};

const drawRangeFlag = (ctx: CanvasRenderingContext2D, rangStartX: number, rangeEndX: number, color: string,
    session: Session, rangStartSvg: SVGSVGElement, rangEndSvg: SVGSVGElement): void => {
    rangStartSvg.style.fill = color;
    rangEndSvg.style.fill = color;
    const rangStartSvgString = new XMLSerializer().serializeToString(rangStartSvg);
    const rangEndSvgString = new XMLSerializer().serializeToString(rangEndSvg);
    const rangStartSvgData = 'data:image/svg+xml;base64,' + btoa(rangStartSvgString);
    const rangEndSvgData = 'data:image/svg+xml;base64,' + btoa(rangEndSvgString);
    const rangeStartIcon = new Image();
    const rangeEndIcon = new Image();
    rangeStartIcon.src = rangStartSvgData;
    rangeEndIcon.src = rangEndSvgData;
    rangeStartIcon.onload = (): void => {
        ctx.drawImage(rangeStartIcon, rangStartX - 3, 0);
    };
    rangeEndIcon.onload = (): void => {
        ctx.drawImage(rangeEndIcon, rangeEndX - 3, 0);
    };
};

interface IDrawFlagsParams {
    session: Session;
    domain: number[];
    current: HTMLCanvasElement;
    range: React.MutableRefObject<[number, number]>;
    vertical: HTMLCanvasElement;
    t: TFunction;
}

export const drawTimelineFlags = ({ session, domain, current, range, vertical, t }: IDrawFlagsParams): void => {
    // draw timeline flag
    const ctx = current.getContext('2d');
    const verticalCtx = vertical.getContext('2d');
    if (!ctx || !verticalCtx || session.name === t('Realtime Monitor')) {
        return;
    }
    const { canvasWidth } = adaptDpr(current, ctx);
    adaptDpr(vertical, verticalCtx);
    // 清空画布
    ctx.clearRect(0, 0, current.width, current.height);
    verticalCtx.clearRect(0, 0, vertical.width, vertical.height);
    const flagList = session.timelineMaker.timelineFlagList;
    let pointSvg: SVGSVGElement | undefined;
    let rangeStartSvg: SVGSVGElement | undefined;
    let rangeEndSvg: SVGSVGElement | undefined;
    document.querySelectorAll('svg').forEach(svgItem => {
        switch (svgItem.id) {
            case 'ic_flag_svg':
                pointSvg = svgItem;
                break;
            case 'ic_range_start_flag':
                rangeStartSvg = svgItem;
                break;
            case 'ic_range_end_flag':
                rangeEndSvg = svgItem;
                break;
        }
    });
    flagList?.forEach((item: TimelineAxisFlag) => {
        if (!pointSvg || !rangeStartSvg || !rangeEndSvg) { return; }
        switch (item.type) {
            case 'point':
                if (!(item.timeStamp >= domain[0] && item.timeStamp <= domain[1])) { return; }
                drawTimelineFlag(ctx, transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth), item, session, pointSvg);
                // 在旗子下方绘制线条
                drawVerticalLineUnderFlag(verticalCtx, transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth), item.color);
                break;
            case 'range':
                if (item.anotherTimeStamp === undefined) {
                    return;
                }
                // 判断标记中首尾时间戳是否有其中一个在范围内
                if (!(item.timeStamp >= domain[0] && item.timeStamp <= domain[1]) && !(item.anotherTimeStamp >= domain[0] && item.anotherTimeStamp <= domain[1])) { return; }
                drawRangeFlag(ctx, transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth),
                    transformTimeToLeft(domain[0], domain[1], item.anotherTimeStamp, canvasWidth),
                    item.color, session, rangeStartSvg, rangeEndSvg);
                // 在范围边界的两个旗子下绘制竖线
                drawVerticalLineUnderFlag(verticalCtx, transformTimeToLeft(domain[0], domain[1], item.timeStamp, canvasWidth), item.color);
                drawVerticalLineUnderFlag(verticalCtx, transformTimeToLeft(domain[0], domain[1], item.anotherTimeStamp, canvasWidth), item.color);
                break;
            default:
        }
    });
};

export const transformTimeToLeft = (domainStart: number, domainEnd: number, timestamp: number, canvasWidth: number): number => {
    if (domainEnd === domainStart) {
        return 0;
    }
    return canvasWidth * (timestamp - domainStart) / (domainEnd - domainStart);
};

const addNewFlag = (session: Session, timeStamp: number, timeDisplay: string): void => {
    const maxNumber = generateDefaultNumber(session);
    const defaultDesc = 'default-' + maxNumber.toString();
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
        session.timelineMaker.timelineFlagList.sort(function(a, b) {
            return a.timeStamp - b.timeStamp;
        });
        // 通知时间轴进行标记绘图更新
        session.timelineMaker.refreshTrigger = (++session.timelineMaker.refreshTrigger) % 10;
    });
};

export const addRangeFlag = (session: Session, rangeStartTimeStamp: number, rangeStartDisplay: string, rangeEndTimeStamp: number): void => {
    const maxNumber = generateDefaultNumber(session);
    const defaultDesc = 'default-' + maxNumber.toString();
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
        session.timelineMaker.timelineFlagList.sort(function(a, b) {
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
 * @param current canvas画布元素dom节点
 */
export const handleMouseMove = (e: MouseEvent, session: Session, current: HTMLCanvasElement | null): void => {
    const ctx = current?.getContext('2d');
    // 获取旗子svg图
    let svg: SVGSVGElement | undefined;
    document.querySelectorAll('svg').forEach(svgItem => {
        if (svgItem.id === 'ic_flag_svg') {
            svg = svgItem;
        }
    });

    if (!current || !ctx || !svg) {
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

    drawTimeLineFlagForMouseMove(ctx, mouse.x, svg);
};

export const handleSingleClick = (e: MouseEvent, session: Session, range: React.MutableRefObject<[number, number]>,
    domainStart: number, domainEnd: number, current: HTMLCanvasElement | null): void => {
    if (session.phase === 'waiting' || session.phase === 'recording' || session.phase === 'analyzing' || session.selectedRange !== undefined) { return; }
    const xScale = linearScaleFactory(range.current, [domainStart, domainEnd]);
    const timeStamp = Math.floor(xScale(e.offsetX));
    const timeDisplay = getTimestamp(timeStamp, { precision: session.isNsMode ? 'ns' : 'ms' });
    if (current === null || session.endTimeAll === undefined || timeStamp > session.endTimeAll) { return; }
    const ctx = current?.getContext('2d');
    const flagCanvas = document.elementsFromPoint(e.clientX, e.clientY).find(t => t.id === 'timelineFlagCnvas');
    const canvasNow = document.elementsFromPoint(e.clientX, e.clientY).find(t => t.classList.contains('drawCanvas'));
    const tableElement = document.elementsFromPoint(e.clientX, e.clientY).find(t => tableElementId.includes(t.id));
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
        if (isInFlagXOffsetRange(e.offsetX, flagXOffset)) {
            setSelectFlag(session, index, e.offsetX, xOffset);
            return;
        } else if (anotherMakerTimeStamp !== undefined) {
            if (isInFlagXOffsetRange(e.offsetX, xOffset(anotherMakerTimeStamp))) {
                setSelectFlag(session, index, e.offsetX, xOffset);
                return;
            }
        }
        setSelectFlag(session, index, e.offsetX, xOffset);
    }
    // 未放置标记，则放置标记
    addNewFlag(session, timeStamp, timeDisplay);
};

const SingleFlagEditDiv = styled.div`
    pointer-events: auto;
    padding-top: 13px;
    width: 258px;
    height: 189px;
    box-shadow: 0 10px 100px 0 rgba(0,0,0,0.50);
    border-radius: 16px;
`;

const Describe = styled.div`
    margin-left: 19px;
    font-size: 12px;
    font-family: MicrosoftYaHei;
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
    font-family: MicrosoftYaHei;
    line-height: 19px;
    float: left
`;

const ColorText = styled.div`
    margin-left: 19px;
    margin-top: 11px;
    font-size: 12px;
    font-family: MicrosoftYaHei;
    height: 19px;
    line-height: 19px;
`;

const ConfirmButton = styled.button`
    position: absolute;
    left: 33px;
    bottom: 37px;
    width: 88px;
    background: #5291FF;
    border: 0px;
    border-radius: 20px;
    text-align: center;
    font-size: 12px;
    font-family: MicrosoftYaHei;
    line-height: 28px;
`;
const DeleteButton = styled.button`
    position: absolute;
    left: 137px;
    bottom: 37px;
    height: 28px;
    width: 88px;
    border: 0px;
    border-radius: 20px;
    text-align: center;
    font-size: 12px;
    font-family: MicrosoftYaHei;
    line-height: 28px;
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

const SingleFlagEditElement = observer((props: TimeLineMakerProps): JSX.Element => {
    const { t } = useTranslation();
    const index = props.index;
    const session = props.session;
    if (index === undefined) {
        return <></>;
    }
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
        ? <SingleFlagEditDiv id={ 'singleFlagEdit' } style={{ background: theme.tooltipBGColor }}>
            <Describe style={{ color: theme.svgPlayBackgroundColor }}>{t('timelineMarker:description')}<CloseIcon style={{ float: 'right', marginRight: '15px', fill: '#71757F' }} onClick={cancelListener}></CloseIcon></Describe>
            <EditInputContainer>
                <EditInput style={{ color: theme.svgPlayBackgroundColor, backgroundColor: theme.searchBackgroundColor }} defaultValue={ timelineAxisFlag.descriptionCache } onChange={inputChangeListener} type={ 'text' }/>
                <InputLengthContainer>
                    <span id={ 'inputLength' } style={{ fontSize: '10px', color: timelineAxisFlag.descriptionCache.length === 256 ? '#D94838' : theme.svgPlayBackgroundColor }}>{ timelineAxisFlag.descriptionCache.length }</span>
                    <span style={{ fontSize: '10px', color: theme.svgPlayBackgroundColor }}>/</span>
                    <span style={{ fontSize: '10px', color: theme.svgPlayBackgroundColor }}>256</span></InputLengthContainer></EditInputContainer>
            <ColorText style={{ color: theme.svgPlayBackgroundColor }} id={ 'colorText' }>{t('timelineMarker:color')}</ColorText>
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
            <ConfirmButton style={{ color: theme.svgPlayBackgroundColor }} onClick={ confirmListener }>{t('timelineMarker:confirmButton')}</ConfirmButton>
            <DeleteButton style={{ color: theme.svgPlayBackgroundColor, background: theme.solidLine }} onClick={ deleteListener }>{t('timelineMarker:deleteButton')}</DeleteButton>
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

export const handleDoubleClick = (e: MouseEvent, session: Session, range: React.MutableRefObject<[number, number]>, domainStart: number, domainEnd: number): void => {
    const canvasNow = document.elementsFromPoint(e.clientX, e.clientY).find(t => t.classList.contains('timeMakerAxis'));
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
        } else if (anotherMarkerTimestamp !== undefined && isInFlagXOffsetRange(e.offsetX, xOffset(anotherMarkerTimestamp))) {
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
        return (x - from[0]) * scale + to[0];
    };
};

type TimelineMarkerProps = {
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
        const GCActionTimeStamp = session.sharedState?.GCActionTimeStamp as number[] | undefined;
        if (ctx !== null && GCActionTimeStamp !== undefined) {
            GCActionTimeStamp.forEach(itemTimeStamp => {
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
                const svgData = 'data:image/svg+xml;base64,' + btoa(svgString);
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

export const TimelineMarkerElement = observer(({ session }: TimelineMarkerProps): JSX.Element => {
    const { t } = useTranslation();
    const range = useRef<[ number, number ]>([0, 0]);
    const canvas = React.useRef<HTMLCanvasElement>(null);
    const { domainStart, domainEnd } = session.domainRange;
    const [width, ref] = useWatchResize<HTMLDivElement>('width');
    // 竖线画板高度，根据DOM树的laneView节点高度动态调整
    const height = session.totalHeight;
    const vertical = React.useRef<HTMLCanvasElement>(null);
    const flagCursor = React.useRef<HTMLCanvasElement>(null);
    const mouseMoveListener = (e: MouseEvent): void => handleMouseMove(e, session, flagCursor.current);
    addEventListener('mousemove', mouseMoveListener);

    React.useEffect(() => {
        if (!canvas.current || !vertical.current) {
            return;
        }
        const singleClickListener = (e: MouseEvent): void => handleSingleClick(e, session, range, domainStart, domainEnd, canvas.current);
        const doubleClickListener = (e: MouseEvent): void => handleDoubleClick(e, session, range, domainStart, domainEnd);
        if (session.name !== t('Realtime Monitor')) {
            addEventListener('click', singleClickListener);
            addEventListener('dblclick', doubleClickListener);
        }
        drawTimelineFlags({ session, domain: [domainStart, domainEnd], current: canvas.current, range, vertical: vertical.current, t });
        range.current = [0, canvas.current.clientWidth];
        return () => {
            removeEventListener('click', singleClickListener);
            removeEventListener('dblclick', doubleClickListener);
        };
    }, [width, domainStart, domainEnd, session.timelineMaker.refreshTrigger, session.selectedRange]);
    return <CanvasContainer ref={ref}>
        <canvas
            id={ 'timelineFlagCnvas' }
            ref={canvas}
            width={width}
            height={TIME_MARKER_AXIS_HEIGHT}
            style={{ width, height: TIME_MARKER_AXIS_HEIGHT, position: 'absolute', top: 0, left: 0 }}/>
        <TimeLineFlagSvg style={{ display: 'none' }}/>
        <RangeEndFlagSvg style={{ display: 'none' }}/>
        <RangeStartFlagSvg style={{ display: 'none' }}/>
        <GCIcon style={{ display: 'none' }} id = "gc_icon" fill = {themeInstance.getThemeType().buttonFontColor}/>
        <canvas ref={flagCursor} width={width} height={TIME_MARKER_AXIS_HEIGHT} style={{ width, height: TIME_MARKER_AXIS_HEIGHT, position: 'absolute', top: 0, left: 0 }} />
        <canvas ref={vertical} width={width - SCROLLBAR_WIDTH} height={height} style={{ width: width - SCROLLBAR_WIDTH, height, position: 'absolute', top: 15, left: 0 }} />
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
    return <SingleFlagEditDiv id={ 'colorEditor' } style={{ backgroundColor: theme.tooltipBGColor, height: '150px' }}>
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
        <ConfirmButton style={{ color: theme.svgPlayBackgroundColor }} onClick={ (): void => {
            confirmClickHandler(timelineAxisFlag, session, setFlagColor);
        }}>
            {t('timelineMarker:confirmButton')}
        </ConfirmButton>
        <DeleteButton style={{ color: theme.svgPlayBackgroundColor, background: theme.solidLine }} onClick={closeEditorWindow}>
            {t('timelineMarker:cancelButton')}
        </DeleteButton>
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
