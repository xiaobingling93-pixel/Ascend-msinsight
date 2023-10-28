import { useTheme } from '@emotion/react';
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import React, {
    Ref,
    useEffect,
    useImperativeHandle,
} from 'react';
import { observer } from 'mobx-react';
import * as d3 from 'd3';
import { Session } from '../../../entity/session';
import { traceEnd } from '../../../utils/traceLogger';
import { useWatchDomResize } from '../../../utils/useWatchDomResize';
import { CustomCrossRenderer, registerCrossUnitRenderer, useCustomRenderers } from './custom';
import { Pos } from './common';
import { draw } from './draw';
import {
    resetCanvasSize,
    mouseDownAction,
    mouseUpAction,
    mouseLeaveAction,
    mouseWheelAction,
    mouseMoveAction,
    keyDownAction,
    MouseDownActionResult,
} from './actions';

registerCrossUnitRenderer({
    action: (ctx, session, xScale) => {
        const selectedRange = session.selectedRange; // should filter on data type
        if (ctx !== null && selectedRange !== undefined) {
            ctx.beginPath();
            ctx.moveTo(xScale(selectedRange[0]), 0);
            ctx.setLineDash([ 4, 2 ]);
            ctx.strokeStyle = '#5291FF';
            ctx.lineTo(xScale(selectedRange[0]), 9999);
            ctx.stroke();
            ctx.setLineDash([]);

            ctx.beginPath();
            ctx.moveTo(xScale(selectedRange[1]), 0);
            ctx.setLineDash([ 4, 2 ]);
            ctx.strokeStyle = '#5291FF';
            ctx.lineTo(xScale(selectedRange[1]), 9999);
            ctx.stroke();
            ctx.setLineDash([]);
        }
    },
    triggers: session => [session.selectedRange],
});

const Overlay = styled.canvas`
    border-left: 1px solid ${props => props.theme.tableBorderColor};
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

type ChartInteractorProps = {
    domainStart: number;
    domainEnd: number;
    endTimeAll: number | undefined;
    session: Session;
    interactorMouseState: InteractorMouseState;
    onTimeStamp?: TimeStampCallbackFunc;

    // These events are tracked for interacting with the underlying lanes.
    isNsMode: boolean;
};

export interface ChartInteractorHandles {
    mouseMoveAction: (interactorMouseState: InteractorMouseState) => void;
    mouseDownAction: (interactorMouseState: InteractorMouseState) => MouseDownActionResult;
    mouseUpAction: (interactorMouseState: InteractorMouseState, e: MouseEvent) => void;
    mouseWheelAction: (interactorMouseState: InteractorMouseState) => void;
    mouseLeaveAction: (interactorMouseState: InteractorMouseState) => void;
    keyDownAction: (e: React.KeyboardEvent<HTMLDivElement>, interactorMouseState: InteractorMouseState) => void;
}

export type InteractorMouseState = {
    clickPos: React.MutableRefObject<Pos | undefined>;
    lastPos: React.MutableRefObject<Pos | undefined>;
    wheelEvent?: { ctrlKey: boolean; deltaY: number };
};

export type InteractorParams = {
    normalCanvas: React.RefObject<HTMLCanvasElement>;
    hoverCanvas: React.RefObject<HTMLCanvasElement>;
    xReverseScale: (x: number) => number;
    xScale: (x: number) => number;
    isNsMode: boolean;
    session: Session;
    customRenderers: CustomCrossRenderer[];
    theme: Theme;
};

const INTERACTOR_WIDTH = 1153;
const Interactor = ({ domainStart, domainEnd, endTimeAll, session, interactorMouseState, isNsMode }: ChartInteractorProps,
    ref: Ref<ChartInteractorHandles>): JSX.Element => {
    const theme = useTheme();
    // use ref instead of state
    // if using state when mousemoving lastPos will keep updating causing component reload
    const accumulativeZoomRef = React.useRef(0);
    // time -> pos
    const [ customRenderers, customRenderTriggers ] = useCustomRenderers(session);
    const [ normalRect, normalCanvas ] = useWatchDomResize<HTMLCanvasElement>();
    const [ hoverRect, hoverCanvas ] = useWatchDomResize<HTMLCanvasElement>();
    // pos -> time
    const xScale = React.useMemo(() => d3.scaleLinear().range([ domainStart, domainEnd ]).domain([ 0, normalRect?.width ?? INTERACTOR_WIDTH ])
        , [ normalRect?.width, session.domain.timePerPx, domainStart, domainEnd ]);
    const xReverseScale = React.useMemo(() => d3.scaleLinear().range([ 0, normalRect?.width ?? INTERACTOR_WIDTH ]).domain([ domainStart, domainEnd ])
        , [ normalRect?.width, session.domain.timePerPx, domainStart, domainEnd ]);
    const interactorParams: InteractorParams = { normalCanvas, hoverCanvas, xReverseScale, xScale, isNsMode, session, customRenderers, theme };
    useEffect(() => {
        resetCanvasSize(normalCanvas, normalRect); resetCanvasSize(hoverCanvas, hoverRect);
    }, [ normalRect, hoverRect ]);
    useEffect(() => {
        if (!normalCanvas.current) { return; }
        draw(normalCanvas.current.getContext('2d'), normalCanvas.current.clientWidth, normalCanvas.current.clientHeight, xReverseScale, xScale, interactorMouseState, session.selectedRange, isNsMode, session, customRenderers, theme);
        const traceAction: string[] = [ 'selectBrushScope', 'dragLane', 'zoomProportion' ];
        traceAction.forEach((item) => { traceEnd(item); });
    }, [ domainStart, domainEnd, endTimeAll, session.selectedRange, theme, normalRect, session.linkData, session.scrollTop, ...customRenderTriggers ]);
    useEffect(() => { if (!normalCanvas.current) { return; } draw(normalCanvas.current.getContext('2d'), normalCanvas.current.clientWidth, normalCanvas.current.clientHeight, xReverseScale, xScale, interactorMouseState, session.selectedRange, isNsMode, session, customRenderers, theme); }, [ session.linkLines, session.totalHeight ]);
    const point = interactorMouseState.lastPos?.current?.x !== undefined ? xScale(interactorMouseState.lastPos?.current?.x) : undefined;
    useImperativeHandle(ref, () => ({
        mouseMoveAction: (interactorMouseState: InteractorMouseState) => {
            mouseMoveAction(interactorParams, interactorMouseState);
        },
        mouseDownAction: (interactorMouseState: InteractorMouseState) => {
            return mouseDownAction(session, xReverseScale, interactorMouseState);
        },
        mouseUpAction: (interactorMouseState: InteractorMouseState, e: MouseEvent) => {
            mouseUpAction(interactorParams, interactorMouseState, e);
        },
        mouseWheelAction: (interactorMouseState: InteractorMouseState) => {
            if (interactorMouseState.wheelEvent) {
                mouseWheelAction(session, accumulativeZoomRef, point, interactorMouseState.wheelEvent);
            }
        },
        mouseLeaveAction: (interactorMouseState: InteractorMouseState) => {
            mouseLeaveAction(interactorParams, interactorMouseState);
        },
        keyDownAction: (e: React.KeyboardEvent<HTMLDivElement>, interactorMouseState: InteractorMouseState) => {
            keyDownAction(e.key, session, point);
        },
    }));
    return <>
        <Overlay ref={normalCanvas} />
        <Overlay ref={hoverCanvas} />
    </>;
};
export const ChartInteractor = observer(React.forwardRef(Interactor));
ChartInteractor.displayName = 'ChartInteractor';
