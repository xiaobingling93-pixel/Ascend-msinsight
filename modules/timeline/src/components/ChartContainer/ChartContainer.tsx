/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import React, { useMemo, useEffect, useRef } from 'react';
import { observable, runInAction } from 'mobx';
// hooks
import { useWatchResize } from '../../utils/useWatchDomResize';
// support utils/types
import type { Session } from '../../entity/session';
// components
import { ChartRow, type ChartRowProps } from '../base/ChartRow';
import { ChartInteractor } from '../charts/ChartInteractor';
import { ContextMenu } from '../ContextMenu';
// same level infer
import { PinnedUnits } from './PinnedUnits';
import { RefUnits } from './Units';
// common constant variable
import ChartHeader from './ChartHeader';
import HorizontalScroller from './HorizontalScrollbar';
import type { ChartInteractorHandles, InteractorMouseState } from '../charts/ChartInteractor/ChartInteractor';
import type { Pos } from '../charts/ChartInteractor/common';
import { THUMB_WIDTH_PX } from '../base';
import { MouseDownActionResult } from '../charts/ChartInteractor/actions';
import { loopActionFactory } from '../../utils/FactoryActions';
import { RenderEngineContext } from '../../context/context';
import { renderEngine } from '../../renderEngine';
import { DragDirection, useDraggableContainerEx } from '../../utils/useDraggableContainerEx';
import Resizor from 'lib/Resizor';

export const TIME_LINE_AXIS_HEIGHT_PX = 30;
const LANE_INFO_WIDTH_PX = observable({ value: 250 });
export const CHARTINTERACTOR_NAME = 'chartInteractor';

const Container = styled.div`
    flex-grow: 1;
    height: 100%;
    overflow: hidden;
    position: relative;
    display: flex;
    flex-direction: column;
    background-color: ${(props): string => props.theme.bgColor};
    border-radius: ${(props): string => props.theme.borderRadiusBase};

    .mask {
        position: absolute;
    }

    /* container header - timeline axis area */
    .timeStamp {
        display: flex;
        background-color: ${(props): string => props.theme.contentBackgroundColor};
        height: ${TIME_LINE_AXIS_HEIGHT_PX}px;
    }

    .line {
        position: absolute;
        bottom: 0;
        width: 100%;
        height: 1px;
    }
`;
interface Props {
    session: Session;
    interactive?: boolean;
    theme?: Theme;
}
// Overlay is not cover the lane info area, so it has a left offset -> LANE_INFO_WIDTH_PX
export const Overlay = styled(({ leftOffset, ...props }: { leftOffset: number } & Omit<ChartRowProps, 'leftWidth'>) => (<ChartRow leftWidth={leftOffset} {...props} />))`
    width: 100%;
    height: 100%;
    position: absolute;
    top:0;
    left:0;
    right:0;
    bottom:0;
    pointer-events: none;
`;

interface ChartBodyProps {
    session: Session;
    interactive?: boolean;
    interactorMouseState: InteractorMouseState;
    chartInteractorRef: React.RefObject<ChartInteractorHandles>;
}

const ChartBody = observer((props: ChartBodyProps) => {
    const { session, interactive, interactorMouseState, chartInteractorRef } = props;
    const { domainStart, domainEnd } = session.domainRange;
    const [height, ref] = useWatchResize<HTMLDivElement>('height');
    const [pinnedHeight, pinnedRef] = useWatchResize<HTMLDivElement>('height');
    const splitLineRef = React.useRef<HTMLDivElement>(null);
    const [view, handleSwitchOpen] = useDraggableContainerEx(
        { draggableWH: 100, dragDirection: DragDirection.TOP, splitLineRef, open: session.pinnedUnits.length > 0 });

    useEffect(() => {
        if (session.pinnedUnits.length > 0) {
            handleSwitchOpen(true);
        } else {
            handleSwitchOpen(false);
        }
        runInAction(() => {
            session.renderTrigger = !session.renderTrigger;
        });
    }, [session.pinnedUnits]);
    return (<>
        {
            view({
                mainContainer: <RefUnits session={session} height={height} ref={ref}
                    hasPinButton={Boolean(interactive)} laneInfoWidth={LANE_INFO_WIDTH_PX.value} />,
                draggableContainer: <PinnedUnits session={session} height={pinnedHeight} ref={pinnedRef} laneInfoWidth={LANE_INFO_WIDTH_PX.value} />,
                id: 'UnitsPage',
            })
        }
        <Overlay leftOffset={LANE_INFO_WIDTH_PX.value} rightAreaName={CHARTINTERACTOR_NAME}>
            <><Resizor style={{ width: '10px', right: '1px', pointerEvents: 'all' }} onResize={(deltaX: number, width: number, nextWidth: number): void => {
                runInAction(() => {
                    if (width > 150 && nextWidth > 100) {
                        LANE_INFO_WIDTH_PX.value = width;
                    }
                });
            }}/> </>
            <ChartInteractor ref={chartInteractorRef} splitLineRef={splitLineRef} domainStart={domainStart}
                domainEnd={domainEnd} endTimeAll={session.endTimeAll}
                interactorMouseState={interactorMouseState} isNsMode={session.isNsMode} session={session}
                renderTrigger={session.renderTrigger} scrollTop={session.scrollTop} selectedRange={session.selectedRange}
            />
        </Overlay>
        <ContextMenu session={session} interactorMouseState={interactorMouseState} chartInteractorRef={chartInteractorRef} />
    </>);
});

export const ChartContainer = observer((props: Props) => {
    const { session } = props;
    const [containerDom, setContainerDom] = React.useState<HTMLDivElement | undefined>(undefined);
    const chartInteractorRef = useRef<ChartInteractorHandles>(null);
    const scrollerRef = React.useRef<HTMLDivElement>(null);
    const { onMouseUp, onKeyDown, interactorMouseState, ...otherInteractors } =
        useInteractorMouseState(chartInteractorRef, scrollerRef, session, !!props.interactive);
    useEffect(() => {
        if (containerDom === undefined) {
            return (): void => {};
        }
        document.addEventListener('mouseup', onMouseUp);
        return (): void => {
            document.removeEventListener('mouseup', onMouseUp);
        };
    }, [containerDom]);
    const keyHoldAction = useMemo(() => loopActionFactory((e: React.KeyboardEvent<HTMLDivElement>) => onKeyDown(e), 40, 100), [session]);
    const handleKeyDownEvent = (e: KeyboardEvent): void => {
        if (!e.repeat) {
            keyHoldAction.beginLoop(e as unknown as React.KeyboardEvent<HTMLDivElement>);
        }
    };
    useEffect(() => {
        document.addEventListener('keydown', handleKeyDownEvent);
        document.addEventListener('keyup', keyHoldAction.clearLoop);
        document.addEventListener('blur', keyHoldAction.clearLoop);

        return (): void => {
            document.removeEventListener('keydown', handleKeyDownEvent);
            document.removeEventListener('keyup', keyHoldAction.clearLoop);
            document.removeEventListener('blur', keyHoldAction.clearLoop);
        };
    });
    return <Container
        onKeyDown={(e): void => {
            if (!e.repeat) {
                keyHoldAction.beginLoop(e);
            }
        }}
        onKeyUp={(): void => { keyHoldAction.clearLoop(); }}
        onBlur={(): void => { keyHoldAction.clearLoop(); }}
        {...otherInteractors}
        ref={(dom): void => {
            setContainerDom(dom ?? undefined);
        }}
        tabIndex={-1}
    >
        <RenderEngineContext.Provider value={renderEngine}>
            <ChartHeader
                session={session}
                laneInfoWidth={LANE_INFO_WIDTH_PX.value}
                timelineHeight={TIME_LINE_AXIS_HEIGHT_PX}
                showRecommendation={!props.interactive}
            />
            <ChartBody session={session} interactive={props.interactive} interactorMouseState={interactorMouseState} chartInteractorRef={chartInteractorRef}/>
        </RenderEngineContext.Provider>
        <HorizontalScroller
            session={session}
            leftLaneInfoWidth={LANE_INFO_WIDTH_PX.value}
            containerDom={containerDom}
            scrollerRef={scrollerRef}
        />
    </Container>;
});

const isMouseOnScrollbar = (e: React.MouseEvent, horizontalScroller: HTMLElement | null): boolean => {
    const target = e.target as HTMLElement;
    const maxClientX = target.getBoundingClientRect().right;
    return (e.clientX >= maxClientX - THUMB_WIDTH_PX) || (target === horizontalScroller);
};

function isTargetElement(event: React.MouseEvent): boolean {
    let ele: HTMLElement | null = event.target as HTMLElement;
    while (ele !== null && ele !== undefined && ele?.id !== 'root') {
        ele = ele.parentElement;
    }
    return Boolean(ele);
}
// eslint-disable-next-line max-lines-per-function
const useInteractorMouseState = (chartInteractorRef: React.RefObject<ChartInteractorHandles>, scrollerRef: React.RefObject<HTMLDivElement>,
    session: Session, interactive?: boolean): InteractorMouseHandlers => {
    const clickPos = useRef<undefined | Pos>(undefined); const lastPos = useRef<Pos | undefined>(undefined);
    const [interactorMouseState, setInteractorMouseState] = React.useState<InteractorMouseState>({ clickPos, lastPos });
    const onMouseMove = (e: React.MouseEvent): void => {
        if (!chartInteractorRef.current) {
            return;
        }
        chartInteractorRef.current.mouseMoveAction(interactorMouseState, e);
        const rect = e.currentTarget.getBoundingClientRect();
        const offsetX = e.nativeEvent.x - rect.left - LANE_INFO_WIDTH_PX.value;
        const offsetY = e.nativeEvent.y - rect.top;
        if (offsetX <= 0) {
            interactorMouseState.lastPos.current = interactorMouseState.clickPos.current ? { x: 0, y: offsetY } : undefined;
            return;
        }
        interactorMouseState.lastPos.current = { x: offsetX, y: offsetY };
    };
    const onMouseDown = (e: React.MouseEvent): void => {
        const disabled = !isTargetElement(e) || !chartInteractorRef.current || !interactive ||
            session.phase !== 'download' || isMouseOnScrollbar(e, scrollerRef.current);
        if (disabled) {
            interactorMouseState.lastPos.current = undefined;
            return;
        }
        const needDragOneSide = chartInteractorRef.current.mouseDownAction(interactorMouseState, e);
        if (needDragOneSide === MouseDownActionResult.NO_NEED_TO_DRAG_ONE_SIDE) {
            interactorMouseState.clickPos.current = interactorMouseState.lastPos.current;
        }
    };
    const onWheel = (e: React.WheelEvent<HTMLDivElement>): void => {
        if (!chartInteractorRef.current) {
            return;
        }
        chartInteractorRef.current.mouseWheelAction(interactorMouseState);
        setInteractorMouseState({ ...interactorMouseState, wheelEvent: { ctrlKey: e.ctrlKey, deltaY: e.deltaY } });
    };
    const onMouseUp = (e: MouseEvent): void => {
        if (!chartInteractorRef.current || !interactive) {
            return;
        }
        chartInteractorRef.current.mouseUpAction(interactorMouseState, e);
    };
    const onMouseLeave = (): void => {
        if (!chartInteractorRef.current) {
            return;
        }
        chartInteractorRef.current.mouseLeaveAction(interactorMouseState);
    };
    const onKeyDown = (e: React.KeyboardEvent<HTMLDivElement>): void => chartInteractorRef.current?.keyDownAction(e, interactorMouseState);
    return { onMouseMove, onMouseDown, onWheel, onMouseLeave, onMouseUp, onKeyDown, interactorMouseState };
};

interface InteractorMouseHandlers {
    onMouseUp: (e: MouseEvent) => void;
    onMouseMove: (e: React.MouseEvent) => void;
    onMouseLeave: () => void;
    onMouseDown: (e: React.MouseEvent) => void;
    onWheel: (e: React.WheelEvent<HTMLDivElement>) => void;
    onKeyDown: (e: React.KeyboardEvent<HTMLDivElement>) => void;
    interactorMouseState: InteractorMouseState;
};
