/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { useTheme } from '@emotion/react';
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import React, {
    type Ref,
    useEffect,
    useImperativeHandle,
} from 'react';
import * as d3 from 'd3';
import type { Session } from '../../../entity/session';
import { traceEnd } from '../../../utils/traceLogger';
import { useWatchDomResize } from '../../../utils/useWatchDomResize';
import { type CustomCrossRenderer, registerCrossUnitRenderer, useCustomRenderers } from './custom';
import type { Pos } from './common';
import { draw } from './draw';
import type { DrawCanvasArgs } from './draw';
import {
    resetCanvasSize,
    mouseDownAction,
    mouseUpAction,
    mouseLeaveAction,
    mouseWheelAction,
    mouseMoveAction,
    keyDownAction,
    type MouseDownActionResult,
} from './actions';
import type { TimeStamp } from '../../../entity/common';
registerCrossUnitRenderer({
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
    border-left: 1px solid ${(props): string => props.theme.tableBorderColor};
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
}

export interface InteractorMouseState {
    clickPos: React.MutableRefObject<Pos | undefined>;
    lastPos: React.MutableRefObject<Pos | undefined>;
    wheelEvent?: { ctrlKey: boolean; deltaY: number };
};

export interface InteractorParams {
    normalCanvas: React.RefObject<HTMLCanvasElement>;
    hoverCanvas: React.RefObject<HTMLCanvasElement>;
    xReverseScale: (x: number) => number;
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
    xReverseScale: d3.ScaleLinear<number, number>;
    splitLineRef: React.RefObject<HTMLDivElement>;
    accumulativeZoomRef: React.MutableRefObject<number>;
    point?: number;
}

const handleInteractorEvent = ({
    interactorParams, session, xReverseScale, splitLineRef, accumulativeZoomRef, point,
}: InteractorEventParams): any => {
    return () => ({
        mouseMoveAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent): void => {
            mouseMoveAction(interactorParams, interactorMouseState, e);
        },
        mouseDownAction: (interactorMouseState: InteractorMouseState, e: React.MouseEvent) =>
            mouseDownAction(session, xReverseScale, interactorMouseState, e, splitLineRef),
        mouseUpAction: (interactorMouseState: InteractorMouseState, e: MouseEvent): void => {
            mouseUpAction(interactorParams, interactorMouseState, e);
        },
        mouseWheelAction: (interactorMouseState: InteractorMouseState): void => {
            if (interactorMouseState.wheelEvent) {
                mouseWheelAction(session, accumulativeZoomRef, point, interactorMouseState.wheelEvent);
            }
        },
        mouseLeaveAction: (interactorMouseState: InteractorMouseState): void => {
            mouseLeaveAction(interactorParams, interactorMouseState);
        },
        keyDownAction: (e: React.KeyboardEvent<HTMLDivElement>, interactorMouseState: InteractorMouseState): void => {
            keyDownAction(e.key, session, point);
        },
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
    const interactorParams: InteractorParams = { normalCanvas, hoverCanvas, xReverseScale, xScale, isNsMode, session, customRenderers, theme };
    useEffect(() => { resetCanvasSize(normalCanvas, normalRect); resetCanvasSize(hoverCanvas, hoverRect); }, [normalRect, hoverRect]);
    useEffect(() => {
        if (!normalCanvas.current) { return; }
        const drawArgs: DrawCanvasArgs = {
            ctx: normalCanvas.current.getContext('2d'),
            width: normalCanvas.current.clientWidth,
            height: normalCanvas.current.clientHeight,
            xReverseScale,
            xScale,
            interactorMouseState,
            selectedRange: session.selectedRange,
            isNsMode,
            session,
            customRenderers,
            theme,
        };
        draw(drawArgs);
        const traceAction: string[] = ['selectBrushScope', 'dragLane', 'zoomProportion'];
        traceAction.forEach((item) => { traceEnd(item); });
    }, [domainStart, domainEnd, endTimeAll, selectedRange, theme, normalRect,
        scrollTop, renderTrigger, ...customRenderTriggers]);
    const point = interactorMouseState.lastPos?.current?.x !== undefined
        ? xScale(interactorMouseState.lastPos?.current?.x)
        : undefined;
    useImperativeHandle(ref, handleInteractorEvent({ interactorParams, session, xReverseScale, splitLineRef, accumulativeZoomRef, point }));
    return <>
        <Overlay ref={normalCanvas} />
        <Overlay ref={hoverCanvas} />
    </>;
};
export const ChartInteractor = React.forwardRef(Interactor);
ChartInteractor.displayName = 'ChartInteractor';
