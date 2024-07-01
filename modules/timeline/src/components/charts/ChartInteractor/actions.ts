/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { clamp, throttle, debounce } from 'lodash';
import { runInAction } from 'mobx';
import React from 'react';
import { Session } from '../../../entity/session';
import { traceStart } from '../../../utils/traceLogger';
import type { InteractorMouseState, InteractorParams } from './ChartInteractor';
import { INTERACTOR_WIDTH } from './ChartInteractor';
import { isOnSideline, SINGLE_DRAG_OFFSET } from './common';
import type { Pos } from './common';
import { draw, drawOnMove, MIN_BRUSH_SIZE } from './draw';
import type { DrawArgs } from './draw';
import { changeRangeMarkerTimestamp } from '../../TimelineMarker';
import { GOLDEN_RATE as MOVE_RATE } from '../../../entity/domain';
import type { Theme } from '@emotion/react';
import { setZoomHistory } from '../../ContextMenu';
import { isMac } from '../../../utils/is';

const dragInitData = {
    isDragging: false,
    xPos: 0,
    domainStart: 0,
    domainEnd: 0,
};

let dragData = { ...dragInitData };
function resetDragInitData(): void {
    dragData = { ...dragInitData };
}

export const resetCanvasSize = (canvas: React.RefObject<HTMLCanvasElement>, rect: DOMRectReadOnly | null): void => {
    if (!canvas.current) { return; }
    canvas.current.width = rect?.width ?? 0;
    canvas.current.height = rect?.height ?? 0;
};
export const mouseUpAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState, e: MouseEvent): void => {
    const { normalCanvas: canvas, hoverCanvas, session, xReverseScale, xScale, isNsMode, customRenderers, theme } = interactorParams;
    const clickPos = interactorMouseState.clickPos.current;
    const lastPos = interactorMouseState.lastPos.current;
    resetDragInitData();
    if (hoverCanvas.current) {
        hoverCanvas.current.style.pointerEvents = 'none';
    }
    if (clickPos === undefined || canvas.current === null || session.endTimeAll === undefined || lastPos === undefined) { return; }

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
    }

    interactorMouseState.clickPos.current = undefined;
    draw(canvas.current.getContext('2d'), canvas.current.clientWidth, canvas.current.clientHeight, xReverseScale, xScale, interactorMouseState, session.selectedRange, isNsMode, session, customRenderers, theme);
};

type GetDrawOnMoveArgs = {
    canvas: React.RefObject<HTMLCanvasElement>;
    xReverseScale: (tx: number) => number;
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
    return {
        ctx: canvas.current.getContext('2d'),
        width: canvas.current.clientWidth,
        height: canvas.current.clientHeight,
        isNsMode: session.isNsMode,
        selectedRange: session.selectedRange,
        session,
        ...props,
    };
};

export const mouseLeaveAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState): void => {
    const { hoverCanvas: canvas, session, xReverseScale, xScale, theme } = interactorParams;
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
    drawOnMove(getDrawOnMoveArgs({ canvas, session, theme, xReverseScale, xScale, interactorMouseState }));
};

export enum MouseDownActionResult {
    NoMouseDownRequired,
    NeedDragOneSide,
    NoNeedToDragOneSide,
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
const shouldIgnoreRangeMarkerButton = (session: Session, lastPos: Pos, xReverseScale: (x: number) => number): boolean => {
    const rangeButtonCanvasHeight = 30;
    const offsetX = lastPos.x;

    if (session.selectedRange !== undefined && lastPos.y <= rangeButtonCanvasHeight) {
        const rangeMarkerButtonWidth = 18;
        const rangeEndTimestamp = session.selectedRange[0] > session.selectedRange[1] ? session.selectedRange[0] : session.selectedRange[1];
        const rangeEndOffsetX = xReverseScale(rangeEndTimestamp);
        if (offsetX >= rangeEndOffsetX - rangeMarkerButtonWidth && offsetX <= rangeEndOffsetX) {
            return true;
        }
    }

    return false;
};

export const mouseDownAction = (session: Session, xReverseScale: (x: number) => number, interactorMouseState: InteractorMouseState,
    e: React.MouseEvent, splitLineRef?: React.RefObject<HTMLDivElement>): MouseDownActionResult => {
    const lastPos = interactorMouseState.lastPos.current;
    const isPressingKey = (isMac && e.metaKey) || (!isMac && e.ctrlKey);
    // 点击context menu选项时屏蔽mouseDownAction
    const contextMenuVisible = session.contextMenu.isVisible as boolean;
    const rightClickOrNoLastPos = session.endTimeAll === undefined || !lastPos || e.button === MouseButton.RIGHT;
    if (isPressingKey) {
        dragData = { isDragging: true, xPos: e.nativeEvent.x, domainStart: session.domainRange.domainStart, domainEnd: session.domainRange.domainEnd };
        return MouseDownActionResult.NoMouseDownRequired;
    }

    if (rightClickOrNoLastPos || shouldIgnoreRangeMarkerButton(session, lastPos, xReverseScale) || contextMenuVisible) {
        return MouseDownActionResult.NoMouseDownRequired;
    }

    const offsetX = lastPos.x;
    const offsetY = lastPos.y;
    if (offsetX > xReverseScale(session.endTimeAll as number)) {
        runInAction(() => {
            let isSingleLine = false;
            Object.values(session.linkLines).forEach((linkLine) => {
                linkLine?.forEach(item => {
                    if (!isSingleLine) {
                        isSingleLine = Boolean(item.cat);
                    }
                });
            });
            isSingleLine && (session.linkLines = {});
            session.selectedRange = undefined;
            session.timelineMaker.oldMarkedRange = undefined;
        });
        return MouseDownActionResult.NoMouseDownRequired;
    }
    if (isInSplitLineY(offsetY, splitLineRef)) {
        return MouseDownActionResult.NoMouseDownRequired;
    }
    let needDragOneSide = false;
    if (session.selectedRange !== undefined && isOnSideline(lastPos, session.selectedRange, xReverseScale)) {
        const isOnLeftSide = offsetX <= xReverseScale(session.selectedRange[0]) + SINGLE_DRAG_OFFSET &&
            offsetX >= xReverseScale(session.selectedRange[0]) - SINGLE_DRAG_OFFSET;
        interactorMouseState.clickPos.current = { x: xReverseScale(isOnLeftSide ? session.selectedRange[1] : session.selectedRange[0]), y: lastPos.y };
        needDragOneSide = true;
    } else {
        interactorMouseState.clickPos.current = { x: offsetX, y: lastPos.y };
        session.timelineMaker.oldMarkedRange = undefined;
    }
    runInAction(() => { session.selectedRange = undefined; });
    return needDragOneSide ? MouseDownActionResult.NeedDragOneSide : MouseDownActionResult.NoNeedToDragOneSide;
};

export const mouseMoveAction = (interactorParams: InteractorParams, interactorMouseState: InteractorMouseState, e: React.MouseEvent): void => {
    const { hoverCanvas: canvas, session, xReverseScale, xScale, theme } = interactorParams;
    if (canvas.current === null) { return; }
    const lastPos = interactorMouseState.lastPos.current;
    const canDrag = ((isMac && e.metaKey) || (!isMac && e.ctrlKey)) && dragData.isDragging;
    if (canDrag) {
        moveDomainByDragging(session, dragData.xPos - e.clientX, canvas.current?.clientWidth ?? INTERACTOR_WIDTH);
        canvas.current.style.cursor = 'grabbing';
        canvas.current.style.pointerEvents = 'initial';
        return;
    }
    if (isOnSideline(lastPos, session.selectedRange, xReverseScale)) {
        canvas.current.style.cursor = 'e-resize';
    } else {
        canvas.current.style.cursor = 'default';
    }
    drawOnMove(getDrawOnMoveArgs({ canvas, session, theme, xReverseScale, xScale, interactorMouseState }));
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
    if (!dragData.isDragging) {
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

export const keyDownAction = (key: string, session: Session, zoomPoint: number | undefined): void => {
    if (key === 'w' || key === 'W') {
        zoomDomain(session, zoomOrMoveDirection.downOrLeft, zoomPoint);
    } else if (key === 's' || key === 'S') {
        zoomDomain(session, zoomOrMoveDirection.upOrRight, zoomPoint);
    } else if (key === 'a' || key === 'A') {
        moveDomain(session, zoomOrMoveDirection.downOrLeft);
    } else if (key === 'd' || key === 'D') {
        moveDomain(session, zoomOrMoveDirection.upOrRight);
    } else {
        // handle other keys
    }
};
