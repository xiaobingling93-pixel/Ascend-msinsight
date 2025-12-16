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

import * as React from 'react';
import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import _, { clamp } from 'lodash';
import type { Session } from '../../entity/session';
import { PAN_RATE } from '../../entity/domain';
import { THUMB_WIDTH_PX } from '../base';
import { traceStart } from '../../utils/traceLogger';

interface ScrollerProps {
    leftLaneInfoWidth: number;
};
const Scroller = styled.div<ScrollerProps>`
    width: calc(100% - ${(props): number => props.leftLaneInfoWidth}px);
    height: 8px;
    overflow-x: auto;
    overflow-y: hidden;
    position: absolute;
    left: ${(props): number => props.leftLaneInfoWidth}px;
    bottom: 0;
    z-index: 2;
`;

interface ScrollBarProps {
    containerDom?: HTMLElement;
    leftLaneInfoWidth: number;
    session: Session;
    scrollerRef: React.RefObject<HTMLDivElement>;
};

interface WatchDomainChangeProps {
    scrollerRef: React.RefObject<HTMLDivElement>;
    isManualHandleRef: React.MutableRefObject<boolean>;
    session: Session;
    domainStart: number;
    domainEnd: number;
    totalTime: number;
}
const useWatchDomainChange = (props: WatchDomainChangeProps): void => {
    const {
        scrollerRef, isManualHandleRef,
        session, domainStart, domainEnd, totalTime,
    } = props;
    const bias = Math.max(0, (session.endTimeAll ?? 0) - session.maxDuration);
    React.useEffect(() => {
        // upadte scroller position when user is not handling and session is recording
        if (scrollerRef.current && !isManualHandleRef.current && totalTime > 0) {
            scrollerRef.current.scrollLeft = ((domainStart - bias) / totalTime) * scrollerRef.current.scrollWidth;
        }
    }, [session.endTimeAll, domainStart, domainEnd]);
};

interface WatchScrollEventProps {
    containerDom?: HTMLElement;
    isManualHandleRef: React.MutableRefObject<boolean>;
    session: Session;
    paddingWidth: number;
    domainStart: number;
    domainEnd: number;
}

const useWatchScrollEvent = (props: WatchScrollEventProps): void => {
    const {
        containerDom, isManualHandleRef,
        session, paddingWidth, domainStart, domainEnd,
    } = props;
    const lastExecutor = React.useRef<Promise<void>>();
    const accumulativeShiftRef = React.useRef(0);
    React.useEffect(() => {
        const wheel = _.throttle((e: WheelEvent): void => {
            if (!e.ctrlKey && !(e.shiftKey && paddingWidth > 100)) { return; }
            e.preventDefault(); e.stopPropagation();

            const curExecutor = Promise.resolve();
            lastExecutor.current = curExecutor;
            if (e.shiftKey && paddingWidth > 100) {
                isManualHandleRef.current = true;
                // Browser's native scroll event could be different between off-screen-rendering turn on and turn off.
                // When off-screen-rendering on and press 'shift' key, e.deltaY is 0, e.deltaX is scroll distance, else, the scroll distance is e.deltaY.
                const scrollDist = e.deltaX === 0 ? e.deltaY : -e.deltaX;
                accumulativeShiftRef.current += Math.sign(scrollDist);
                curExecutor.then(() => {
                    if (curExecutor !== lastExecutor.current) { return; }
                    const timeDuration = domainEnd - domainStart;
                    const timeOffset = accumulativeShiftRef.current *
                        PAN_RATE * timeDuration;
                    const newEnd = clamp(domainEnd + timeOffset, timeDuration, session.endTimeAll ?? session.domain.defaultDuration);
                    runInAction(() => {
                        session.realTimeUpdate = newEnd === (session.endTimeAll ?? 0) && session.phase === 'recording';
                        session.domainRange = { domainStart: newEnd - timeDuration, domainEnd: newEnd };
                    });
                    accumulativeShiftRef.current = 0;
                    isManualHandleRef.current = false;
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
    const isManualHandleRef = React.useRef(false);

    useWatchDomainChange({ scrollerRef, isManualHandleRef, session, domainStart, domainEnd, totalTime });
    useWatchScrollEvent({ containerDom, isManualHandleRef, session, paddingWidth, domainStart, domainEnd });

    const handleScroll = _.throttle(({ currentTarget }: React.UIEvent<HTMLDivElement>): void => {
        runInAction(() => {
            if (scrollerRef.current && isManualHandleRef.current && currentTarget.scrollWidth !== 0) {
                traceStart('dragLane', { action: 'dragLane' });
                const bias = Math.max(0, (session.endTimeAll ?? 0) - session.maxDuration);
                const start = (totalTime * (currentTarget.scrollLeft / currentTarget.scrollWidth)) + bias;
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
            onMouseDown={(): void => { isManualHandleRef.current = true; }}
            onMouseUp={(): void => { isManualHandleRef.current = false; }}
            onScroll={handleScroll}
        >
            <div className="padding" style={{ width: `${paddingWidth}%`, height: THUMB_WIDTH_PX * 1.2 }} />
        </Scroller>
    );
});
export default HorizontalScroller;
