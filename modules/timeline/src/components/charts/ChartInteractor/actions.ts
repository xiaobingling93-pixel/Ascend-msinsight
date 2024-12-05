/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { clamp, throttle, debounce } from 'lodash';
import { runInAction } from 'mobx';
import type React from 'react';
import type { Session } from '../../../entity/session';
import { traceStart } from '../../../utils/traceLogger';
import type { InteractorMouseState, InteractorParams, XReverseScaleRef } from './ChartInteractor';
import { INTERACTOR_WIDTH } from './ChartInteractor';
import { isOnSideline, SINGLE_DRAG_OFFSET } from './common';
import type { Pos } from './common';
import { draw, drawOnMove, MIN_BRUSH_SIZE } from './draw';
import type { DrawArgs, DrawCanvasArgs } from './draw';
import { changeRangeMarkerTimestamp } from '../../TimelineMarker';
import { GOLDEN_RATE as MOVE_RATE } from '../../../entity/domain';
import type { Theme } from '@emotion/react';
import { setZoomHistory } from '../../ContextMenu';
import { isMac } from '../../../utils/is';
import { CardMetaData, SliceMeta, ThreadTrace } from '../../../entity/data';
import { getTimeOffsetKey } from '../../../insight/units/utils';
import { UNIT_WRAPPER_SCROLLER_ID } from '../../ChartContainer/Units/Units';

const dragInitData = {
    isDragging: false,
    xPos: 0,
    domainStart: 0,
    domainEnd: 0,
};

// 每次滚动的步长
const SCROLL_STEP = 20;

let dragData = { ...dragInitData };
function resetDragInitData(): void {
    dragData = { ...dragInitData };
}

export const resetCanvasSize = (canvas: React.RefObject<HTMLCanvasElement>, rect: DOMRectReadOnly | null): void => {
    if (!canvas.current) { return; }
    canvas.current.width = (rect?.width ?? 0) * devicePixelRatio;
    canvas.current.height = (rect?.height ?? 0) * devicePixelRatio;
};

const updateSessionStatus = (e: MouseEvent, session: Session, newSelected: [number, number]): void => {
    runInAction(() => {
        if (e.altKey) {
            session.domainRange = { domainStart: newSelected[0], domainEnd: newSelected[1] };
            setZoomHistory(session, session.domainRange);
        }
        session.selectedRange = newSelected;
        changeRangeMarkerTimestamp(session, newSelected);
        const selectedRange = session.selectedRange[1] - session.selectedRange[0];
        traceStart('selectBrushScope', {
            action: 'selectBrushScope',
            units: session.selectedUnits.map((unit) => unit?.name),
            selectedRange: session.isNsMode ? Math.ceil(selectedRange / 1e6) : selectedRange,
        });
    });
};

export const mouseUpAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState, e: MouseEvent): void => {
    const { normalCanvas: canvas, hoverCanvas, session, xReverseScaleRef, xScale, isNsMode, customRenderers, theme } = interactorParams;
    const clickPos = interactorMouseState.clickPos.current;
    const lastPos = interactorMouseState.lastPos.current;
    resetDragInitData();
    if (hoverCanvas.current) {
        hoverCanvas.current.style.pointerEvents = 'none';
    }
    if (session.endTimeAll === undefined) {
        return;
    }
    const isInValid = (clickPos === undefined || canvas.current === null || lastPos === undefined);
    if (isInValid) { return; }

    if (session.contextMenu.isVisible) {
        return;
    }
    // when selected range changes, the 'more' panel should be cleared (by resetting session.selectedDetailKeys)
    runInAction(() => {
        session.selectedDetailKeys = [];
        session.selectedDetails = [];
    });

    if (Math.abs(lastPos.x - clickPos.x) >= MIN_BRUSH_SIZE) {
        const mouseRange: [number, number] = [xScale(clickPos.x), xScale(lastPos.x)];
        const newSelected = mouseRange.sort((a, b) => a - b);

        if (newSelected[0] < session.endTimeAll && session.endTimeAll < newSelected[1]) { newSelected[1] = session.endTimeAll; }
        updateSessionStatus(e, session, newSelected);
    }

    interactorMouseState.clickPos.current = undefined;
    const drawArgs: DrawCanvasArgs = {
        ctx: canvas.current.getContext('2d'),
        width: canvas.current.clientWidth,
        height: canvas.current.clientHeight,
        xReverseScaleRef,
        xScale,
        interactorMouseState,
        selectedRange: session.selectedRange,
        isNsMode,
        session,
        customRenderers,
        theme,
    };
    draw(drawArgs);
};

interface GetDrawOnMoveArgs {
    canvas: React.RefObject<HTMLCanvasElement>;
    xReverseScaleRef: XReverseScaleRef;
    xScale: (pos: number) => number;
    session: Session;
    interactorMouseState: InteractorMouseState;
    theme: Theme;
};
const getDrawOnMoveArgs = ({
    canvas,
    session,
    ...props
}: GetDrawOnMoveArgs): DrawArgs => {
    if (!canvas.current) { throw Error('missed canvas'); }
    const ctx = canvas.current.getContext('2d');
    if (!ctx) {
        throw Error('Failed to get CanvasRenderingContext2D');
    }
    return {
        ctx,
        width: canvas.current.clientWidth,
        height: canvas.current.clientHeight,
        isNsMode: session.isNsMode,
        selectedRange: session.selectedRange,
        session,
        ...props,
    };
};

export const mouseLeaveAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState): void => {
    const { hoverCanvas: canvas, session, xReverseScaleRef, xScale, theme } = interactorParams;
    if (canvas.current === null || session.endTimeAll === undefined) { return; }
    const clickPos = interactorMouseState.clickPos.current;
    const lastPos = interactorMouseState.lastPos.current;
    if (clickPos && lastPos) {
        // mouse leave when mouse down
        const x = lastPos.x < 0 ? 0 : lastPos.x;
        const mouseRange: [number, number] = [xScale(clickPos.x), xScale(x)];
        const newSelected = mouseRange.sort((a, b) => a - b);

        if (newSelected[0] < session.endTimeAll && session.endTimeAll < newSelected[1]) {
            newSelected[1] = session.endTimeAll;
        }

        if (Math.abs(x - clickPos.x) >= MIN_BRUSH_SIZE) {
            runInAction(() => { session.selectedRange = newSelected; });
        } else {
            runInAction(() => { session.selectedRange = undefined; });
        }
    }
    interactorMouseState.clickPos.current = undefined; interactorMouseState.lastPos.current = undefined;
    drawOnMove(getDrawOnMoveArgs({ canvas, session, theme, xReverseScaleRef, xScale, interactorMouseState }));
};

export enum MouseDownActionResult {
    NO_MOUSEDOWN_REQUIRED = 0,
    NEED_DRAG_ONE_SIDE = 1,
    NO_NEED_TO_DRAG_ONE_SIDE = 2,
}

export enum MouseButton {
    LEFT = 0,
    MIDDLE = 1,
    RIGHT = 2,
}

const getOffsetTop = (ele: HTMLElement): number => {
    return (
        ele.offsetTop + (ele.offsetParent ? getOffsetTop(ele.offsetParent as HTMLElement) : 0)
    );
};

const isInSplitLineY = (offsetY: number, splitLineRef: React.RefObject<HTMLDivElement> | undefined): boolean => {
    if (splitLineRef?.current) {
        const splitLinePosTop = getOffsetTop(splitLineRef.current as HTMLElement);
        const splitLineHeight = splitLineRef.current.clientHeight ?? 0;
        const splitLinePosY = [splitLinePosTop, splitLinePosTop + splitLineHeight];
        if (offsetY >= splitLinePosY[0] && offsetY <= splitLinePosY[1]) {
            return true;
        }
    }
    return false;
};

// 点击放置框选标记按钮则屏蔽mouseDownAction，避免当前框选丢失
const shouldIgnoreRangeMarkerButton = (session: Session, lastPos: Pos, xReverseScale: XReverseScaleRef): boolean => {
    const rangeButtonCanvasHeight = 30;
    const offsetX = lastPos.x;

    if (session.selectedRange !== undefined && lastPos.y <= rangeButtonCanvasHeight) {
        const rangeMarkerButtonWidth = 18;
        const rangeEndTimestamp = session.selectedRange[0] > session.selectedRange[1] ? session.selectedRange[0] : session.selectedRange[1];
        const rangeEndOffsetX = xReverseScale.current(rangeEndTimestamp);
        if (offsetX >= rangeEndOffsetX - rangeMarkerButtonWidth && offsetX <= rangeEndOffsetX) {
            return true;
        }
    }

    return false;
};

const isSingleLine = (session: Session): boolean => {
    return Object.values(session.linkLines).some(linkLine =>
        linkLine?.some(item => Boolean(item.cat)),
    );
};

export const mouseDownAction = (session: Session, xReverseScaleRef: XReverseScaleRef, interactorMouseState: InteractorMouseState,
    e: React.MouseEvent, splitLineRef?: React.RefObject<HTMLDivElement>): MouseDownActionResult => {
    const lastPos = interactorMouseState.lastPos.current;
    const isPressingKey = (isMac && e.metaKey) || (!isMac && e.ctrlKey);
    // 点击context menu选项时屏蔽mouseDownAction
    const contextMenuVisible = session.contextMenu.isVisible as boolean;
    const rightClickOrNoLastPos = session.endTimeAll === undefined || !lastPos || e.button === MouseButton.RIGHT;
    if (isPressingKey) {
        dragData = { isDragging: true, xPos: e.nativeEvent.x, domainStart: session.domainRange.domainStart, domainEnd: session.domainRange.domainEnd };
        return MouseDownActionResult.NO_MOUSEDOWN_REQUIRED;
    }

    if (rightClickOrNoLastPos || shouldIgnoreRangeMarkerButton(session, lastPos, xReverseScaleRef) || contextMenuVisible) {
        return MouseDownActionResult.NO_MOUSEDOWN_REQUIRED;
    }

    const offsetX = lastPos.x;
    const offsetY = lastPos.y;
    if (offsetX > xReverseScaleRef.current(session.endTimeAll as number)) {
        runInAction(() => {
            if (isSingleLine(session)) {
                session.linkLines = {};
            }
            session.selectedRange = undefined;
            session.timelineMaker.oldMarkedRange = undefined;
        });
        return MouseDownActionResult.NO_MOUSEDOWN_REQUIRED;
    }
    if (isInSplitLineY(offsetY, splitLineRef)) {
        return MouseDownActionResult.NO_MOUSEDOWN_REQUIRED;
    }
    let needDragOneSide = false;
    if (session.selectedRange !== undefined && isOnSideline(lastPos, session.selectedRange, xReverseScaleRef)) {
        const isOnLeftSide = offsetX <= xReverseScaleRef.current(session.selectedRange[0]) + SINGLE_DRAG_OFFSET &&
            offsetX >= xReverseScaleRef.current(session.selectedRange[0]) - SINGLE_DRAG_OFFSET;
        interactorMouseState.clickPos.current = {
            x: xReverseScaleRef.current(isOnLeftSide ? session.selectedRange[1] : session.selectedRange[0]),
            y: lastPos.y,
        };
        needDragOneSide = true;
    } else {
        interactorMouseState.clickPos.current = { x: offsetX, y: lastPos.y };
        session.timelineMaker.oldMarkedRange = undefined;
    }
    runInAction(() => { session.selectedRange = undefined; });
    return needDragOneSide ? MouseDownActionResult.NEED_DRAG_ONE_SIDE : MouseDownActionResult.NO_NEED_TO_DRAG_ONE_SIDE;
};

export const mouseMoveAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState, e: React.MouseEvent): void => {
    const { hoverCanvas: canvas, session, xReverseScaleRef, xScale, theme } = interactorParams;
    if (canvas.current === null) { return; }
    const lastPos = interactorMouseState.lastPos.current;
    const canDrag = ((isMac && e.metaKey) || (!isMac && e.ctrlKey)) && dragData.isDragging;
    if (canDrag) {
        moveDomainByDragging(session, dragData.xPos - e.clientX, canvas.current?.clientWidth ?? INTERACTOR_WIDTH);
        canvas.current.style.cursor = 'grabbing';
        canvas.current.style.pointerEvents = 'initial';
        return;
    }
    if (isOnSideline(lastPos, session.selectedRange, xReverseScaleRef)) {
        canvas.current.style.cursor = 'e-resize';
    } else {
        canvas.current.style.cursor = 'default';
    }
    drawOnMove(getDrawOnMoveArgs({ canvas, session, theme, xReverseScaleRef, xScale, interactorMouseState }));
};

const handleZoom = throttle((session: Session, accumulativeZoomRef: React.MutableRefObject<number>, zoomPoint: number | undefined): void => {
    runInAction(() => {
        session.zoom = { zoomCount: accumulativeZoomRef.current, zoomPoint };
    });
    accumulativeZoomRef.current = 0;
}, 50);

const setZoomHistoryDebounce = debounce((session) => {
    runInAction(() => {
        setZoomHistory(session, session.domainRange);
    });
}, 300);

export const mouseWheelAction = (
    session: Session,
    accumulativeZoomRef: React.MutableRefObject<number>,
    zoomPoint: number | undefined,
    wheelEvent: { ctrlKey: boolean; deltaY: number},
): void => {
    if (wheelEvent.ctrlKey) {
        accumulativeZoomRef.current += Math.sign(wheelEvent.deltaY);
        handleZoom(session, accumulativeZoomRef, zoomPoint);
        setZoomHistoryDebounce(session);
    }
};

const zoomDomain = (session: Session, zoomCount: number, zoomPoint: number | undefined): void => {
    runInAction(() => {
        session.zoom = { zoomCount, zoomPoint };
    });
    setZoomHistoryDebounce(session);
};

const moveDomain = (session: Session, direction: number): void => {
    const { domainRange: { domainStart, domainEnd } } = session;
    const timeDuration = domainEnd - domainStart;
    const timeOffset = direction * MOVE_RATE * timeDuration;
    const newEnd = clamp(domainEnd + timeOffset, timeDuration, session.endTimeAll ?? session.domain.defaultDuration);
    runInAction(() => {
        session.domainRange = { domainStart: newEnd - timeDuration, domainEnd: newEnd };
    });
};

const moveDomainByDragging = throttle((session: Session, offset: number, canvasWidth: number): void => {
    if (!dragData.isDragging || canvasWidth === 0) {
        return;
    }
    const { domainRange: { domainStart, domainEnd } } = session;
    const timeDuration = domainEnd - domainStart;
    const timeOffset = offset * timeDuration / canvasWidth;
    const newEnd = clamp(dragData.domainEnd + timeOffset, timeDuration, session.endTimeAll ?? session.domain.defaultDuration);
    runInAction(() => {
        session.domainRange = { domainStart: newEnd - timeDuration, domainEnd: newEnd };
    });
}, 100);

const zoomOrMoveDirection = {
    upOrRight: 1,
    downOrLeft: -1,
};

const processMKeyEvent = (session: Session): void => {
    let render = false;
    let range: number[] = [];
    if (!session.mKeyRender && session.selectedData === undefined) {
        // 页面无遮罩，无选中算子，点击m，页面无遮罩
        render = false;
        range = [];
    } else if (!session.mKeyRender && session.selectedData !== undefined) {
        // 页面无遮罩，选中算子，点击m，页面出现遮罩
        const selectedData = session.selectedData as ThreadTrace;
        range = [selectedData.startTime, selectedData.startTime + selectedData.duration];
        render = true;
    } else if (session.mKeyRender && session.selectedData === undefined) {
        // 页面有遮罩，无选中算子，点击m，页面无遮罩
        render = false;
        range = [];
    } else if (session.mKeyRender && session.selectedData !== undefined) {
        const selectedData = session.selectedData as ThreadTrace;
        const tempRange = [selectedData.startTime, selectedData.startTime + selectedData.duration];
        if (session.mMaskRange[0] === tempRange[0] && session.mMaskRange[1] === tempRange[1]) {
            // 页面有遮罩，有选中算子，但是同一个范围，点击m，页面无遮罩
            render = false;
            range = [];
        } else {
            // 页面有遮罩，有选中算子，但不是同一个范围，点击m，页面更换遮罩
            render = true;
            range = tempRange;
        }
    } else {
        render = session.mKeyRender;
        range = session.mMaskRange;
    }
    runInAction(() => {
        session.mKeyRender = render;
        session.mMaskRange = range;
    });
};

const extractDeviceId = (processId: number): number => {
    const DEVICE_ID_MASK = 0x1F;
    return processId & DEVICE_ID_MASK;
};

const updateSameDeviceOffset = (selectSliceMeta: SliceMeta, session: Session, selectOffsetKey: string, offsetDiff: number): void => {
    const deviceId = extractDeviceId(parseInt(selectSliceMeta.processId));
    session.units.forEach((unit) => {
        if ((unit.metadata as CardMetaData).cardId !== selectSliceMeta.cardId) {
            return;
        }
        if (unit.children === undefined || unit.children.length <= 0) {
            return;
        }
        for (const item of unit.children) {
            const tempMeta = item.metadata as SliceMeta;
            if (tempMeta.label !== 'NPU') {
                continue;
            }
            const key = getTimeOffsetKey(session, tempMeta);
            if (key === selectOffsetKey) {
                continue;
            }
            if (isNaN(Number(tempMeta.processId))) {
                continue;
            }
            const tempDeviceId = extractDeviceId(parseInt(tempMeta.processId));
            if (tempDeviceId !== deviceId) {
                continue;
            }
            session.unitsConfig.offsetConfig.timestampOffset[key] += offsetDiff;
        }
    });
};

const processOffsetEvent = (session: Session, isLeft: boolean): void => {
    if (session.benchMarkData === undefined || session.selectedData === undefined) {
        return;
    }
    const selectSliceMeta = session.selectedData as SliceMeta;
    const benchSliceMeta = session.benchMarkData as SliceMeta;
    const selectOffsetKey = getTimeOffsetKey(session, selectSliceMeta);
    const benchSliceOffsetKey = getTimeOffsetKey(session, benchSliceMeta);
    if (selectOffsetKey === benchSliceOffsetKey) {
        return;
    }
    let offsetDiff = 0;
    if (isLeft) {
        offsetDiff = selectSliceMeta.startTime - benchSliceMeta.startTime;
    } else {
        offsetDiff = selectSliceMeta.startTime + selectSliceMeta.duration - benchSliceMeta.startTime - benchSliceMeta.duration;
    }
    if (!isNaN(Number(selectSliceMeta.processId))) {
        updateSameDeviceOffset(selectSliceMeta, session, selectOffsetKey, offsetDiff);
    }
    const before = session.unitsConfig.offsetConfig.timestampOffset[selectOffsetKey];
    session.unitsConfig.offsetConfig.timestampOffset[selectOffsetKey] = before + offsetDiff;
    runInAction(() => {
        if (session.selectedData === undefined) {
            return;
        }
        const temp = session.selectedData as ThreadTrace;
        temp.startTime -= offsetDiff;
        const newAlignSliceData: Array<Record<string, unknown>> = [];
        newAlignSliceData.push(temp);
        session.alignSliceData.forEach((item) => {
            const itemTemp = item as SliceMeta;
            if (itemTemp.cardId === selectSliceMeta.cardId && itemTemp.processId === selectSliceMeta.processId) {
                return;
            }
            newAlignSliceData.push(item);
        });
        session.alignSliceData = newAlignSliceData;
        session.selectedData = undefined;
        session.alignRender = !session.alignRender;
    });
};

const scrollWrapper = (eventKey: string): void => {
    const scrollElement = document.getElementById(UNIT_WRAPPER_SCROLLER_ID);
    // 滚动方向，正数为向下滚动，反之向上
    const scrollDirection = eventKey === 'ArrowDown' ? 1 : -1;
    requestAnimationFrame(() => {
        scrollElement?.scrollBy(0, SCROLL_STEP * scrollDirection);
    });
};

export const keyDownAction = (key: string, session: Session, zoomPoint: number | undefined): void => {
    if (key === 'w' || key === 'W') {
        zoomDomain(session, zoomOrMoveDirection.downOrLeft, zoomPoint);
    } else if (key === 's' || key === 'S') {
        zoomDomain(session, zoomOrMoveDirection.upOrRight, zoomPoint);
    } else if (key === 'a' || key === 'A' || key === 'ArrowLeft') {
        moveDomain(session, zoomOrMoveDirection.downOrLeft);
    } else if (key === 'd' || key === 'D' || key === 'ArrowRight') {
        moveDomain(session, zoomOrMoveDirection.upOrRight);
    } else if (key === 'm' || key === 'M') {
        processMKeyEvent(session);
    } else if (key === 'r' || key === 'R') {
        processOffsetEvent(session, false);
    } else if (key === 'l' || key === 'L') {
        processOffsetEvent(session, true);
    } else if (key === 'ArrowUp' || key === 'ArrowDown') {
        scrollWrapper(key);
    } else {
        // handle other keys
    }
};
