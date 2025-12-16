/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { useTheme } from '@emotion/react';
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import React, { type Ref, useEffect, useImperativeHandle } from 'react';
import * as d3 from 'd3';
import type { Session } from '../../../entity/session';
import { traceEnd } from '../../../utils/traceLogger';
import { useWatchDomResize } from '../../../utils/useWatchDomResize';
import { type CustomCrossRenderer, registerCrossUnitRenderer, useCustomRenderers } from './custom';
import type { Pos, ExtendPos } from './common';
import { draw, drawMEventMask, drawRectOfSlice } from './draw';
import type { DrawCanvasArgs, DrawArgs } from './draw';
import {
    resetCanvasSize,
    mouseDownAction,
    mouseUpAction,
    mouseLeaveAction,
    mouseWheelAction,
    mouseMoveAction,
    type MouseDownActionResult, handleMousePosChange,
} from './actions';
import type { TimeStamp } from '../../../entity/common';
import { useEventBus } from '../../../utils/eventBus';

registerCrossUnitRenderer({ // 绘制选中区间的虚线
    action: (ctx, session, xScale) => {
        const selectedRange = session.selectedRange; // should filter on data type
        if (ctx !== null && selectedRange !== undefined) {
            ctx.beginPath();
            ctx.moveTo(xScale(selectedRange[0]), 0);
            ctx.setLineDash([4, 2]);
            ctx.strokeStyle = '#5291FF';
            ctx.lineTo(xScale(selectedRange[0]), 9999);
            ctx.stroke();
            ctx.setLineDash([]);

            ctx.beginPath();
            ctx.moveTo(xScale(selectedRange[1]), 0);
            ctx.setLineDash([4, 2]);
            ctx.strokeStyle = '#5291FF';
            ctx.lineTo(xScale(selectedRange[1]), 9999);
            ctx.stroke();
            ctx.setLineDash([]);
        }
    },
    triggers: session => [session.selectedRange],
});

const Overlay = styled.canvas`
    border-left: 1px solid ${(props): string => props.theme.borderColor};
    position: absolute;
    top: 0;
    bottom: 0;
    left: 0;
    right: 0;
    height: 100%;
    width: 100%;
    pointer-events: none;
    z-index: 1;
`;

interface TimeStampCallbackFunc {
    (time: number): void;
}

export interface ChartInteractorProps {
    domainStart: number;
    domainEnd: number;
    endTimeAll?: number;
    session: Session;
    interactorMouseState: InteractorMouseState;
    onTimeStamp?: TimeStampCallbackFunc;

    // These events are tracked for interacting with the underlying lanes.
    isNsMode: boolean;
    splitLineRef: React.RefObject<HTMLDivElement>;
    renderTrigger: boolean;
    scrollTop: number;
    selectedRange?: [ TimeStamp, TimeStamp ];
}

export interface ChartInteractorHandles {
    mouseMoveAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent) => void;
    mouseDownAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent) => MouseDownActionResult;
    mouseUpAction: (interactorMouseState: InteractorMouseState, e: MouseEvent) => void;
    mouseWheelAction: (interactorMouseState: InteractorMouseState) => void;
    mouseLeaveAction: (interactorMouseState: InteractorMouseState) => void;
    keyDownAction: (e: React.KeyboardEvent<HTMLDivElement>, interactorMouseState: InteractorMouseState) => void;
    keyUpAction: (e: KeyboardEvent, interactorMouseState: InteractorMouseState) => void;
    xScale: (x: number) => number;
}

export interface InteractorMouseState {
    clickPos: React.MutableRefObject<ExtendPos | undefined>;
    lastPos: React.MutableRefObject<Pos | undefined>;
    wheelEvent?: { ctrlKey: boolean; deltaY: number };
};

export type XReverseScaleRef = React.MutableRefObject<d3.ScaleLinear<number, number>>;

export interface InteractorParams {
    normalCanvas: React.RefObject<HTMLCanvasElement>;
    hoverCanvas: React.RefObject<HTMLCanvasElement>;
    xReverseScale: (x: number) => number;
    xReverseScaleRef: XReverseScaleRef;
    xScale: (x: number) => number;
    isNsMode: boolean;
    session: Session;
    customRenderers: CustomCrossRenderer[];
    theme: Theme;
};

export const INTERACTOR_WIDTH = 1153;

interface InteractorEventParams {
    interactorParams: InteractorParams;
    session: Session;
    xReverseScaleRef: XReverseScaleRef;
    splitLineRef: React.RefObject<HTMLDivElement>;
    accumulativeZoomRef: React.MutableRefObject<number>;
}

// 获取缩放的基准点
export const getZoomPoint = (xScale: (x: number) => number, interactorMouseState: InteractorMouseState): number | undefined => {
    return interactorMouseState.lastPos?.current?.x !== undefined
        ? xScale(interactorMouseState.lastPos?.current?.x)
        : undefined;
};

const handleInteractorEvent = ({
    interactorParams, session, xReverseScaleRef, splitLineRef, accumulativeZoomRef,
}: InteractorEventParams): any => {
    return () => ({
        mouseMoveAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent): void => {
            mouseMoveAction(interactorParams, interactorMouseState, e);
        },
        mouseDownAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent) =>
            mouseDownAction({ session, xReverseScaleRef, interactorMouseState, e, splitLineRef, interactorParams }),
        mouseUpAction: (interactorMouseState: InteractorMouseState, e: MouseEvent): void => {
            mouseUpAction(interactorParams, interactorMouseState, e);
        },
        mouseWheelAction: (interactorMouseState: InteractorMouseState): void => {
            if (interactorMouseState.wheelEvent) {
                const point = getZoomPoint(interactorParams.xScale, interactorMouseState);
                mouseWheelAction(session, accumulativeZoomRef, point, interactorMouseState.wheelEvent);
            }
        },
        mouseLeaveAction: (interactorMouseState: InteractorMouseState): void => {
            mouseLeaveAction(interactorParams, interactorMouseState);
        },
        xScale: interactorParams.xScale,
    });
};

const Interactor = ({
    domainStart, domainEnd, endTimeAll, session, interactorMouseState, isNsMode, splitLineRef, renderTrigger, scrollTop, selectedRange,
}: ChartInteractorProps, ref: Ref<ChartInteractorHandles>): JSX.Element => {
    const theme = useTheme();
    const accumulativeZoomRef = React.useRef(0);
    const [customRenderers, customRenderTriggers] = useCustomRenderers(session);
    const [normalRect, normalCanvas] = useWatchDomResize<HTMLCanvasElement>();
    const [hoverRect, hoverCanvas] = useWatchDomResize<HTMLCanvasElement>();
    const xScale = React.useMemo(() => d3.scaleLinear().range([domainStart, domainEnd]).domain([0, normalRect?.width ?? INTERACTOR_WIDTH])
        , [normalRect?.width, session.domain.timePerPx, domainStart, domainEnd]);
    const xReverseScale = React.useMemo(() => d3.scaleLinear().range([0, normalRect?.width ?? INTERACTOR_WIDTH]).domain([domainStart, domainEnd])
        , [normalRect?.width, session.domain.timePerPx, domainStart, domainEnd]);
    const xReverseScaleRef = React.useRef(xReverseScale);
    xReverseScaleRef.current = xReverseScale;
    const interactorParams: InteractorParams = {
        normalCanvas,
        hoverCanvas,
        xReverseScale,
        xReverseScaleRef,
        xScale,
        isNsMode,
        session,
        customRenderers,
        theme,
    };

    function getDrawArgs(): DrawCanvasArgs | null {
        if (!normalCanvas.current) { return null; }
        return {
            ctx: normalCanvas.current.getContext('2d'),
            width: normalCanvas.current.clientWidth,
            height: normalCanvas.current.clientHeight,
            xReverseScaleRef,
            xScale,
            interactorMouseState,
            selectedRange: session.selectedRange,
            isNsMode,
            session,
            customRenderers,
            theme,
        };
    }

    function selectionModeChange(): void {
        if (!session.sliceSelection.activeIsChanged) {
            return;
        }
        session.resetOfSliceSelection(false);
        session.sliceSelection.activeIsChanged = false;
        session.selectedRange = undefined;
        session.selectedRangeIsLock = false;
        if (!hoverCanvas.current) {
            return;
        }
        hoverCanvas.current.getContext('2d')?.clearRect(0, 0, hoverCanvas.current.clientWidth, hoverCanvas.current.clientHeight);
        normalCanvas.current?.getContext('2d')?.clearRect(0, 0, normalCanvas.current?.clientWidth, normalCanvas.current?.clientHeight);
    }

    useEffect(() => {
        const normalCanvasCtx = normalCanvas.current?.getContext('2d');
        const hoverCanvasCtx = hoverCanvas.current?.getContext('2d');

        resetCanvasSize(normalCanvas, normalRect);
        resetCanvasSize(hoverCanvas, hoverRect);

        normalCanvasCtx?.setTransform(1, 0, 0, 1, 0, 0);
        hoverCanvasCtx?.setTransform(1, 0, 0, 1, 0, 0);
        normalCanvasCtx?.scale(devicePixelRatio, devicePixelRatio);
        hoverCanvasCtx?.scale(devicePixelRatio, devicePixelRatio);
    },
    [normalRect, hoverRect]);
    useEffect(() => {
        if (session.selectedRangeIsLock) {
            return;
        }
        const drawArgs = getDrawArgs();
        if (drawArgs === null) { return; }
        drawMEventMask(drawArgs);
    }, [domainStart, domainEnd, endTimeAll, theme, scrollTop, session.mKeyRender, session.mMaskRange, renderTrigger, ...customRenderTriggers]);
    useEffect(() => { // 当页面中的范围等变化时，必须更新绘图
        const drawArgs = getDrawArgs();
        if (drawArgs === null) { return; }
        draw(drawArgs);
        const traceAction: string[] = ['selectBrushScope', 'dragLane', 'zoomProportion'];
        traceAction.forEach((item) => { traceEnd(item); });
    }, [domainStart, domainEnd, endTimeAll, selectedRange, theme, scrollTop, renderTrigger, ...customRenderTriggers]);

    useEffect(() => {
        const drawArgs = getDrawArgs();
        if (drawArgs === null) { return; }
        // 加延时确保画布尺寸变化后，正常重新绘制，否则可能不能及时绘制(场景：框选后，底部面板弹起，触发重新绘制)
        setTimeout(() => {
            draw(drawArgs);
        }, 10);
    }, [normalRect]);
    useEffect(() => { // 用于在鼠标按住框选时配合 awsd 方向按键更新拖动选框绘制
        if (session.selectedRangeIsLock) { return; }
        handleMousePosChange(interactorParams, interactorMouseState);
    }, [domainStart, domainEnd]);

    // 切换算子框选模式清空画板
    useEffect(() => {
        selectionModeChange();
    }, [session.sliceSelection.active]);

    useEffect(() => {
        const { active, selecting } = session.sliceSelection;
        if (active && selecting) {
            const drawArg = getDrawArgs();
            if (!drawArg || !hoverCanvas.current) {
                return;
            }
            drawRectOfSlice({ ...drawArg, ctx: hoverCanvas.current.getContext('2d') } as DrawArgs);
        }
    }, [scrollTop]);

    useEventBus('sliceActiveChanged', (): void => {
        selectionModeChange();
    });

    useImperativeHandle(ref, handleInteractorEvent({ interactorParams, session, xReverseScaleRef, splitLineRef, accumulativeZoomRef }));
    return <>
        <Overlay ref={normalCanvas} />
        <Overlay ref={hoverCanvas} />
    </>;
};
export const ChartInteractor = React.forwardRef(Interactor);
ChartInteractor.displayName = 'ChartInteractor';
