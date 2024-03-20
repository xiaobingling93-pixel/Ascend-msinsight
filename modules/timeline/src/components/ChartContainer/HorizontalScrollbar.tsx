import * as React from 'react';
import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import _, { clamp } from 'lodash';
import { Session } from '../../entity/session';
import { GOLDEN_RATE as MOVE_RATE } from '../../entity/domain';
import { THUMB_WIDTH_PX } from '../base';
import { traceStart } from '../../utils/traceLogger';

interface ScrollerProps {
    leftLaneInfoWidth: number;
};
const Scroller = styled.div<ScrollerProps>`
    width: calc(100% - ${(props) => props.leftLaneInfoWidth}px);
    height: 8px;
    overflow-x: auto;
    overflow-y: hidden;
    position: absolute;
    left: ${(props) => props.leftLaneInfoWidth}px;
    bottom: 0;
    z-index: 2;

    &::-webkit-scrollbar {
        height: 7px;
        transition: opacity 120ms ease-out;
    }

    &::-webkit-scrollbar:hover {
        cursor: pointer;
    }

    &::-webkit-scrollbar-track {
        background-color: transparent;
    }

    &::-webkit-scrollbar-thumb {
        background-color: rgba(127, 127, 127, .5);
        border-radius: 5px;
        transition: .3s background-color;
    }
`;

interface ScrollBarProps {
    containerDom: HTMLElement | undefined;
    leftLaneInfoWidth: number;
    session: Session;
    scrollerRef: React.RefObject<HTMLDivElement>;
};

const useWatchDomainChange = (scrollerRef: React.RefObject<HTMLDivElement>, isManulHandleRef: React.MutableRefObject<boolean>,
    session: Session, domainStart: number, domainEnd: number, totalTime: number): void => {
    const bias = Math.max(0, (session.endTimeAll ?? 0) - session.maxDuration);
    React.useEffect(() => {
        // upadte scroller position when user is not handling and session is recording
        if (scrollerRef.current && !isManulHandleRef.current && totalTime > 0) {
            scrollerRef.current.scrollLeft = ((domainStart - bias) / totalTime) * scrollerRef.current.scrollWidth;
        }
    }, [session.endTimeAll, domainStart, domainEnd]);
};

const useWatchScrollEvent = (containerDom: HTMLElement | undefined,
    isManulHandleRef: React.MutableRefObject<boolean>, session: Session, paddingWidth: number,
    domainStart: number, domainEnd: number): void => {
    const lastExecutor = React.useRef<Promise<void>>();
    const accumulativeShiftRef = React.useRef(0);
    React.useEffect(() => {
        const wheel = _.throttle((e: WheelEvent): void => {
            if (!e.ctrlKey && !(e.shiftKey && paddingWidth > 100)) { return; }
            e.preventDefault(); e.stopPropagation();

            const curExecutor = Promise.resolve();
            lastExecutor.current = curExecutor;
            if (e.shiftKey && paddingWidth > 100) {
                isManulHandleRef.current = true;
                // Browser's native scroll event could be different between off-screen-rendering turn on and turn off.
                // When off-screen-rendering on and press 'shift' key, e.deltaY is 0, e.deltaX is scroll distance, else, the scroll distance is e.deltaY.
                const scrollDist = e.deltaX === 0 ? e.deltaY : -e.deltaX;
                accumulativeShiftRef.current += Math.sign(scrollDist);
                curExecutor.then(() => {
                    if (curExecutor !== lastExecutor.current) { return; }
                    const timeDuration = domainEnd - domainStart;
                    const timeOffset = accumulativeShiftRef.current *
                        MOVE_RATE * timeDuration;
                    const newEnd = clamp(domainEnd + timeOffset, timeDuration, session.endTimeAll ?? session.domain.defaultDuration);
                    runInAction(() => {
                        session.realTimeUpdate = newEnd === (session.endTimeAll ?? 0) && session.phase === 'recording';
                        session.domainRange = { domainStart: newEnd - timeDuration, domainEnd: newEnd };
                    });
                    accumulativeShiftRef.current = 0;
                    isManulHandleRef.current = false;
                });
            }
        }, 100);
        if (containerDom) { containerDom.addEventListener('wheel', wheel, false); };
        return () => { containerDom?.removeEventListener('wheel', wheel); };
    });
};

const HorizontalScroller = observer((props: ScrollBarProps) => {
    const { containerDom, leftLaneInfoWidth, session, scrollerRef } = props;
    const { domainRange: { domainStart, domainEnd } } = session;
    const totalTime = Math.min(session.endTimeAll ?? session.domain.defaultDuration, session.maxDuration);

    const prevPaddingWidthRef = React.useRef(0);
    const paddingWidth = React.useMemo(() => {
        if (domainEnd === domainStart) {
            return 0;
        }
        const width = 100 * totalTime / (domainEnd - domainStart);
        // default scroll to right when the scroller display
        if (width > 100 && prevPaddingWidthRef.current <= 100) {
            runInAction(() => {
                session.realTimeUpdate = true;
            });
        }
        prevPaddingWidthRef.current = width;
        return width;
    }, [totalTime, domainStart, domainEnd]);
    const isManulHandleRef = React.useRef(false);

    useWatchDomainChange(scrollerRef, isManulHandleRef, session, domainStart, domainEnd, totalTime);
    useWatchScrollEvent(containerDom, isManulHandleRef, session, paddingWidth, domainStart, domainEnd);

    const handleScroll = _.throttle(({ currentTarget }: React.UIEvent<HTMLDivElement>): void => {
        runInAction(() => {
            if (scrollerRef.current && isManulHandleRef.current && currentTarget.scrollWidth !== 0) {
                traceStart('dragLane', { action: 'dragLane' });
                const bias = Math.max(0, (session.endTimeAll ?? 0) - session.maxDuration);
                const start = totalTime * (currentTarget.scrollLeft / currentTarget.scrollWidth) + bias;
                const end = start + domainEnd - domainStart;
                // when calculate end value is within 50ms below than endTimeAll value,
                // set domainEnd as endTimeAll approximately.
                const isEndPoint = end > (session.endTimeAll ?? Number.MAX_SAFE_INTEGER) - 50;
                session.realTimeUpdate = isEndPoint && session.phase === 'recording';
                session.domainRange = { domainStart: Math.round(start), domainEnd: Math.round(start) + session.domain.duration };
            }
        });
    }, 100);

    return (
        <Scroller leftLaneInfoWidth={leftLaneInfoWidth}
            ref={scrollerRef}
            onMouseDown={() => { isManulHandleRef.current = true; }}
            onMouseUp={() => { isManulHandleRef.current = false; }}
            onScroll={handleScroll}
        >
            <div className="padding" style={{ width: `${paddingWidth}%`, height: THUMB_WIDTH_PX * 1.2 }} />
        </Scroller>
    );
});
export default HorizontalScroller;
