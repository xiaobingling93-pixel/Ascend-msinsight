/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import styled from '@emotion/styled';
import cls from 'classnames';
import { computed, runInAction } from 'mobx';
import { observer } from 'mobx-react';
import * as React from 'react';
import { useEffect, useRef } from 'react';
// hooks
import { useWatchResize } from '../../../utils/useWatchDomResize';
// support utils/types
import { preOrderFlatten, type TreeNode } from '../../../entity/common';
import type { InsightUnit } from '../../../entity/insight';
import type { Session } from '../../../entity/session';
import { getAutoKey } from '../../../utils/dataAutoKey';
import { traceSingle } from '../../../utils/traceLogger';
import { Chart } from '../../charts';
import { isPinned, isSonPinned } from '../unitPin';
import { useSelectUnit } from './hooks/useSelectUnit';
import type { KeyedInsightUnit } from './types';
import { UnitInfo } from './UnitInfo';
import { ChartErrorBoundary } from '../../error/ChartErrorBoundary';
import eventBus, { EventType, useEventBus } from '../../../utils/eventBus';
import { Mask } from '../../charts/Mask';
import { useJumpTarget } from './hooks';
import type { OrderOptions } from './hooks';
import { CardUnit } from '../../../insight/units/AscendUnit';
import { getRootUnit } from '../../../utils';

const Lane = styled.div<{ laneHeight: number; className: string }>`
    position: relative;
    display: flex;
    box-sizing: border-box;
    flex-direction: row;
    height: ${(props): number => props.laneHeight}px;
    border-bottom: solid 1px ${(props): string => props.theme.tableBorderColor};
    .unit-info {
        background-color: ${(props): string => props.className.includes(UNIT_SELECTED) ? props.theme.selectedChartBackgroundColor : props.theme.contentBackgroundColor};
        div svg g use {
            fill: ${(props): string => props.theme.fontColor};
        }
    }
    .empty.unit-info{
        background-color: ${(props): string => props.className.includes(UNIT_SELECTED) ? props.theme.selectedChartBackgroundColor : props.theme.buttonBackgroundColor};
     }
    .chart-selected {
        box-shadow: 0 0 0 3px ${(props): string => props.theme.selectedChartBorderColor} inset;
    }
`;

const VIRTUAL_SCROLL_THRESHOLD = 30;
const UNIT_SELECTED = 'unit-selected';
const UNIT_VISIBLE = 'unit-visible';

const Splitter = styled.div`
    width: 100%;
    height: 1px;
    background-color: ${(props): string => props.theme.tableBorderColor};
`;

const Join = (props: { joiner: React.FC; children: JSX.Element[] }): JSX.Element => (
    <>
        {props.children.slice(1).reduce((prev, cur, index) => [...prev, <props.joiner key={`joiner/${index}`}/>, cur], [props.children[0]])}
    </>
);

const ChartView = observer(({ unit, session, width, height }: {unit: KeyedInsightUnit; session: Session; width: number; height: number }): JSX.Element => {
    if (Array.isArray(unit.chart)) {
        return <Join joiner={Splitter}>
            {unit.chart.map((desc, index) =>
                <Chart desc={desc} key={`${getAutoKey(unit)}/${index}`} serial={`${getAutoKey(unit)}/${index}`} unit={unit}
                    title={unit.name} session={session} metadata={unit.metadata} width={width} phase={unit.phase} />,
            )}
        </Join>;
    }
    if (unit.chart !== undefined) {
        return <Chart
            unit={unit} desc={unit.chart} key={getAutoKey(unit)} serial={getAutoKey(unit)}
            title={unit.name} session={session} metadata={unit.metadata} width={width} phase={unit.phase} />;
    }
    if (unit instanceof CardUnit && (unit.phase === 'analyzing' || unit.phase === 'download')) {
        return <ChartErrorBoundary height={height} width={width} phase={unit.phase}>
            <div className="chart-empty" style={{ width, height }}/>
        </ChartErrorBoundary>;
    } else {
        return <ChartErrorBoundary height={height} width={width} phase={unit.phase}>
            <Mask unitPhase={unit.phase} isShowMask={session.id !== 'HomePage' && session.phase !== 'error' && unit.phase !== 'configuring' && unit.phase !== 'download' && unit.phase !== 'error'}>
                <div className="chart-empty" style={{ width, height }}/>
            </Mask>
        </ChartErrorBoundary>;
    }
});

interface UnitProps {
    unit: KeyedInsightUnit;
    session: Session;
    isVisible: boolean;
    hasPinButton: boolean;
    laneInfoWidth: number;
    hasExpandIcon: boolean;
    isPinned: boolean;
    isSonPinned: boolean;
}

export const Unit = observer(({ unit, session, isVisible, ...props }: UnitProps): JSX.Element => {
    const isSelected = (session.selectedUnitKeys as string[]).includes(getAutoKey(unit));
    const [chartWidth, ref] = useWatchResize<HTMLDivElement>('width');
    const height = unit.height() + 1; // to support modifying height from outside during runtime, don't useMemo
    const placeholder = React.useMemo(() => {
        return <div className="chart-invisible" style={{ width: chartWidth, height }}/>;
    }, [chartWidth, height]);
    // to support auto redraw when unit height is modified, don't useMemo
    const chart = <ChartView unit={unit} session={session} width={chartWidth} height={height} />;
    const selectUnit = useSelectUnit(session);
    return <Lane className={cls('unit', { [UNIT_SELECTED]: isSelected, [UNIT_VISIBLE]: isVisible })} laneHeight={height}>
        <UnitInfo
            className={unit.name === 'Empty' ? 'empty' : ''}
            height={height}
            session={session}
            unit={unit}
            {...props}
        />
        <div className={isSelected ? 'chart-selected' : 'chart'} ref={ref}
            onMouseDown={(): void => {
                selectUnit(unit);
                traceSingle('selectLane', [unit.name]);
            }}
            style={{ flexGrow: 1, minWidth: 0 }}>
            { isVisible ? chart : placeholder }
        </div>
    </Lane>;
});

export const computeVisibleUnitRange = (units: InsightUnit[], viewportHeight: number, scrollTop: number): [ number, number ] => {
    let start = 0;
    let end = 0;
    let yOffset = 0;
    for (let i = 0; i < units.length; i++) {
        if (yOffset + VIRTUAL_SCROLL_THRESHOLD > viewportHeight + scrollTop) {
            break;
        }
        if (yOffset + units[i].height() + VIRTUAL_SCROLL_THRESHOLD < scrollTop) {
            start++;
        }
        yOffset += units[i].height() + 1;
        end++;
    }
    return [start, end + 1];
};

interface FlattenUnitsProps {
    session: Session;
    height: number;
    laneInfoWidth: number;
    hasPinButton: boolean;
    eventType: string;
};

const orderOptions = {
    preOrderFlatten,
    options: {
        when: (unit: TreeNode<InsightUnit>): boolean => unit.isExpanded,
        bypass: (unit: TreeNode<InsightUnit>): boolean => unit.type === 'transparent',
        exclude: (unit: TreeNode<InsightUnit>): boolean => (unit.pinType === 'move' && isPinned(unit)) || !unit.isDisplay,
    },
};

const updateListener = (element: HTMLDivElement): any => {
    element.addEventListener('wheel', handleWheel, { passive: false });
    return () => {
        element.removeEventListener('wheel', handleWheel);
    };
};

const updateSession = (session: Session, totalHeight: number, cardIdSet: Set<string>): void => {
    session.totalHeight = totalHeight;
    let isSameSet = true;
    for (const cardId of cardIdSet) {
        if (!session.viewedCardIdSet.has(cardId)) {
            isSameSet = false;
            break;
        }
    }
    for (const cardId of session.viewedCardIdSet) {
        if (!cardIdSet.has(cardId)) {
            isSameSet = false;
            break;
        }
    }
    if (!isSameSet) {
        session.viewedCardIdSet = cardIdSet;
    }
};

const computeOnScreenCardIdSet = (flattenUnits: InsightUnit[], first: number, last: number): Set<string> => {
    return new Set(flattenUnits.filter((_, i) => first <= i && i < last).map(insightUnit => {
        const { cardId } = insightUnit.metadata as { cardId: string };
        return cardId;
    }));
};

const FlattenUnits = observer(({ session, height, hasPinButton, laneInfoWidth, eventType }: FlattenUnitsProps): JSX.Element => {
    const [scrollTop, setScrollTop] = React.useState(0);
    // 监听滚动事件，计算虚拟滚动的泳道
    useEventBus(eventType, (value) => setScrollTop(value as number));
    const flattenUnitsAll = computed(() => orderOptions.preOrderFlatten(getRootUnit(session.units), 0, orderOptions.options)).get();
    const flattenUnits = computed(() => flattenUnitsAll.filter(unit => unit.isUnitVisible)).get();
    const [first, last] = React.useMemo(() => computeVisibleUnitRange(flattenUnits, height, scrollTop),
        [session.pinnedUnits, flattenUnits, height, scrollTop],
    );
    const headOffset = React.useMemo(
        () => flattenUnits.filter((_, i) => i < first).reduce((prev, cur) => prev + cur.height() + 1, 0),
        [flattenUnits, first],
    );
    const visibleUnitsHeight = React.useMemo(
        () => flattenUnits.filter((_, i) => first <= i && i < last).reduce((prev, cur) => prev + cur.height() + 1, 0),
        [flattenUnits, first, last],
    );
    const tailOffset = React.useMemo(
        () => flattenUnits.filter((_, i) => i >= last).reduce((prev, cur) => prev + cur.height() + 1, 0),
        [flattenUnits, last],
    );
    const totalHeight = React.useMemo(() => headOffset + visibleUnitsHeight + tailOffset, [headOffset, visibleUnitsHeight, tailOffset]);
    const cardIdSet = computeOnScreenCardIdSet(flattenUnits, first, last);
    runInAction(() => {
        updateSession(session, totalHeight, cardIdSet);
    });
    const ref = useRef<HTMLDivElement>(null);
    useEffect(() => {
        const element = ref.current;
        if (element) {
            return updateListener(element);
        }
        return (): void => {};
    }, []);
    return <div ref={ref} style={{ display: 'flex', flexDirection: 'column', height: totalHeight }} className="laneView">
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: headOffset }} />
        {flattenUnits.filter((_, i) => first <= i && i < last).map((unit) => (!isPinned(unit) || unit.pinType === 'copied') &&
            <Unit
                key={getAutoKey(unit)}
                laneInfoWidth={laneInfoWidth}
                unit={unit}
                session={session}
                hasPinButton={hasPinButton}
                hasExpandIcon={true}
                isVisible={true}
                isPinned={isPinned(unit)}
                isSonPinned={isSonPinned(unit)}
            />)}
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: tailOffset }} />
    </div>;
});

const TableScroller = styled.div`
    flex-grow: 1;
    overflow-y: overlay;
    overflow-x: hidden;
    border-top: solid 1px ${(props): string => props.theme.tableBorderColor};
`;

interface ScrollerProps {
    children: JSX.Element | null;
    session: Session;
    eventType: string;
    orderOptions: OrderOptions;
    unitsArea: InsightUnit[];
    supportJump: boolean;
};

export const Scroller = React.forwardRef(({ session, children, eventType, orderOptions: sorderOptions, unitsArea, supportJump }: ScrollerProps,
    ref: React.ForwardedRef<HTMLDivElement>): JSX.Element => {
    // 广播滚动事件
    function scroll(e: React.UIEvent<HTMLDivElement>): void {
        const scrollTop = e.currentTarget.scrollTop;
        eventBus.emit(eventType, scrollTop);
        // 修改session.scrollTop
        runInAction(() => {
            session.scrollTop = scrollTop;
        });
    }

    // 切换session滚动到之前记录的地方
    React.useEffect(() => {
        (ref as React.MutableRefObject<HTMLDivElement | null>).current?.scrollTo(0, session.scrollTop);
    }, [session]);

    // 跳转到指定泳道
    useJumpTarget(session, unitsArea, supportJump, sorderOptions, (ref as React.MutableRefObject<HTMLDivElement | null>).current);

    return <TableScroller className={`laneWrapper ${eventType === EventType.PINNEDUNITWRAPPERSCROLL ? 'pinnedScrollArea' : ''}`} onScroll={scroll} ref={ref}>
        {children}
    </TableScroller>;
});
Scroller.displayName = 'insight-scroller';

const handleWheel = (event: WheelEvent): void => {
    if (event.ctrlKey) {
        event.preventDefault();
    }
};

const INVISIBLE_UNITS_PLACEHOLDER = 'invisible-units-placeholder';

const Units = ({ session, height, hasPinButton, laneInfoWidth }:
{ session: Session; height: number; hasPinButton: boolean; laneInfoWidth: number }, ref: React.ForwardedRef<HTMLDivElement>): JSX.Element => {
    return <Scroller session={session} unitsArea={session.units} supportJump={true}
        ref={ref} orderOptions={orderOptions} eventType={EventType.UNITWRAPPERSCROLL}>
        <FlattenUnits
            session={session}
            height={height}
            laneInfoWidth={laneInfoWidth}
            hasPinButton={hasPinButton}
            eventType={EventType.UNITWRAPPERSCROLL} />
    </Scroller>;
};

export const RefUnits = React.forwardRef(Units);
RefUnits.displayName = 'Units';
