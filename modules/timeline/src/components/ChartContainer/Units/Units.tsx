/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import styled from '@emotion/styled';
import cls from 'classnames';
import { computed, runInAction } from 'mobx';
import { observer } from 'mobx-react';
import * as React from 'react';
import { useEffect, useRef, useState } from 'react';
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
import { useSelectUnit, useSelectUnits, useDeselectUnits } from './hooks/useSelectUnit';
import type { KeyedInsightUnit } from './types';
import { UnitInfo } from './UnitInfo';
import { ChartErrorBoundary } from '../../error/ChartErrorBoundary';
import eventBus, { EventType, useEventBus } from '../../../utils/eventBus';
import { Mask } from '../../charts/Mask';
import { useJumpTarget } from './hooks';
import type { OrderOptions } from './hooks';
import { CardUnit } from '../../../insight/units/AscendUnit';
import { getRootUnit } from '../../../utils';
import { MetaDataBase, ThreadMetaData } from '../../../entity/data';
import { PAGE_PADDING } from '../../charts/ChartInteractor/draw';
import { MouseButton } from '../../charts/ChartInteractor/actions';

const Lane = styled.div<{ laneHeight: number; className: string; top: number; zIndex: number; isDragging: boolean }>`
    position: ${(props): string => props.isDragging ? 'fixed' : 'relative'};
    display: flex;
    box-sizing: border-box;
    flex-direction: row;
    height: ${(props): number => props.laneHeight}px;
    border-bottom: solid 1px ${(props): string => props.theme.borderColor};
    top: ${(props): number => props.top}px;
    z-index: ${(props): number => props.zIndex};
    transform: ${(props): string => props.isDragging ? 'scale(1.01)' : 'scale(1)'};
    transition: transform .2s, box-shadow .2s;
    .unit-info {
        background-color: ${(props): string => props.className.includes(UNIT_SELECTED) ? props.theme.selectedChartBackgroundColor : props.theme.contentBackgroundColor};
        div svg g use {
            fill: ${(props): string => props.theme.fontColor};
        }
    }
    .chart-selected {
        box-shadow: 0 0 0 2px ${(props): string => props.theme.selectedChartBorderColor} inset;
    }
`;

const VIRTUAL_SCROLL_THRESHOLD = 30;
const UNIT_SELECTED = 'unit-selected';
const UNIT_VISIBLE = 'unit-visible';
export const UNIT_WRAPPER_SCROLLER_ID = 'unitWrapperScroller';
export const PINNED_UNIT_WRAPPER_SCROLLER_ID = 'pinnedUnitWrapperScroller';

const Splitter = styled.div`
    width: 100%;
    height: 1px;
    background-color: ${(props): string => props.theme.borderColor};
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
    nextUnitTop?: number;
    isSelecting?: boolean;
    inRangeUnitKeys?: Set<string>;
    forwardedRef?: React.ForwardedRef<HTMLDivElement>;
    enableDrag?: boolean;
    startYRef?: React.MutableRefObject<number>;
    getDraggingUnitIndexByKey?: (key: string) => void;
    setNextUnitTop?: React.Dispatch<React.SetStateAction<number>>;
    onMouseMove?: (e: MouseEvent, isMoveDown: boolean) => void;
}

const isSelectable = (unit: KeyedInsightUnit): boolean => {
    return !['Card', 'Root'].includes(unit?.name);
};

export const UnitObserver = observer((
    { unit, session, isVisible, isSelecting, inRangeUnitKeys, forwardedRef, startYRef, ...props }: UnitProps,
): JSX.Element => {
    const unitKey = getAutoKey(unit);
    const isSelected = session.selectedUnitKeys.includes(unitKey);
    const [chartWidth, ref] = useWatchResize<HTMLDivElement>('width');
    const height = unit.height() + 1; // to support modifying height from outside during runtime, don't useMemo
    const placeholder = React.useMemo(() => {
        return <div className="chart-invisible" style={{ width: chartWidth, height }}/>;
    }, [chartWidth, height]);
    // to support auto redraw when unit height is modified, don't useMemo
    const chart = <ChartView unit={unit} session={session} width={chartWidth} height={height} />;
    const clickedUnit = session.selectedUnits[0];
    const selectUnit = useSelectUnit(session);
    const selectUnits = useSelectUnits(session);
    const deSelectUnits = useDeselectUnits(session);

    const unitMetaData = unit?.metadata as ThreadMetaData;
    const clickedUnitMetaData = clickedUnit?.metadata as ThreadMetaData;
    const isSameCard = unitMetaData.cardId === clickedUnitMetaData?.cardId || unitMetaData.cardId?.endsWith('.db');

    // 拖拽泳道相关
    const [top, setTop] = useState<number>(0);
    const [zIndex, setZIndex] = useState<number>(0);
    const [isCurrentDragging, setIsCurrentDragging] = useState<boolean>(false);
    // 鼠标点击位置离拖拽泳道上边框的距离
    const elementTop = useRef<number>(0);
    const previousY = useRef<number>(0);

    const onMouseDown = (e: React.MouseEvent): void => {
        e.preventDefault();
        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', onMouseUp, { once: true });
        const ele = e.currentTarget.getBoundingClientRect();
        elementTop.current = e.clientY - ele.top;
        previousY.current = e.clientY;
        props.getDraggingUnitIndexByKey?.(unitKey);
        props.setNextUnitTop?.(ele.height);
        setZIndex(999);
        setIsCurrentDragging(true);
        setTop(ele.top);
    };

    const onMouseMove = (e: MouseEvent): void => {
        e.preventDefault();
        const topDistance = e.clientY - elementTop.current;
        // 拖拽过程中鼠标位置超出置顶区域时垂直方向触发滚动事件
        const scrollElement = document.getElementById(PINNED_UNIT_WRAPPER_SCROLLER_ID);
        if (scrollElement) {
            const rect = scrollElement.getBoundingClientRect();
            if (e.clientY > rect.bottom) {
                scrollElement.scrollBy(0, 10);
            }
            if (e.clientY < rect.top) {
                scrollElement.scrollBy(0, -10);
            }
        }
        if (e.clientY !== previousY.current) {
            props.onMouseMove?.(e, e.clientY > previousY.current);
        }
        previousY.current = e.clientY;
        setTop(topDistance);
        runInAction(() => {
            session.isDragging = true;
        });
    };

    const onMouseUp = (e: MouseEvent): void => {
        e.preventDefault();
        e.stopPropagation();
        document.removeEventListener('mousemove', onMouseMove);
        previousY.current = 0;
        props.setNextUnitTop?.(0);
        setTop(0);
        setZIndex(0);
        setIsCurrentDragging(false);
        runInAction(() => {
            session.isDragging = false;
        });
    };

    useEffect(() => {
        if (inRangeUnitKeys === undefined || [undefined, false].includes(isSelecting)) { return; }
        if (isSelectable(unit) && isSelectable(clickedUnit) && isSameCard) {
            if (inRangeUnitKeys?.has(unitKey)) {
                selectUnits(unit);
            } else {
                deSelectUnits(unit);
            }
        }
    }, [inRangeUnitKeys?.size]);

    return <Lane className={cls('unit', {
        [UNIT_SELECTED]: isSelected,
        [UNIT_VISIBLE]: isVisible,
    })} laneHeight={height} ref={forwardedRef} top={props.nextUnitTop ?? top} zIndex={zIndex} isDragging={isCurrentDragging}>
        <UnitInfo
            className={unit.name === 'Empty' ? 'empty' : ''}
            height={height}
            session={session}
            unit={unit}
            isSelected={isSelected}
            onMouseDown={onMouseDown}
            {...props}
        />
        <div className={isSelected ? 'chart-selected' : 'chart'} ref={ref}
            onMouseDown={(e): void => {
                if (session.selectedRangeIsLock) {
                    return;
                }
                if (e.button !== MouseButton.LEFT && session.selectedRange !== undefined) {
                    return;
                }
                selectUnit(unit);
                traceSingle('selectLane', [unit.name]);
            }}
            style={{ flexGrow: 1, minWidth: 0 }}>
            { isVisible ? chart : placeholder }
        </div>
    </Lane>;
});

export const Unit = React.forwardRef((props: UnitProps, ref: React.ForwardedRef<HTMLDivElement>) => {
    return <UnitObserver {...props} forwardedRef={ref}></UnitObserver>;
});

Unit.displayName = 'Unit';

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
        exclude: (unit: TreeNode<InsightUnit>): boolean => (unit.pinType === 'move' && isPinned(unit)) || !unit.isDisplay || unit.isMultiDeviceHidden,
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

interface MetaDataWithCardId extends MetaDataBase {
    cardId: string;
}

const computeOnScreenCardIdSet = (flattenUnits: InsightUnit[], first: number, last: number): Set<string> => {
    return new Set(
        flattenUnits
            .map(unit => (unit?.metadata as MetaDataWithCardId)?.cardId)
            .filter((id, i) => first <= i && i < last && id !== undefined && !id.endsWith('Host')),
    );
};

const FlattenUnits = observer(({ session, height, hasPinButton, laneInfoWidth, eventType }: FlattenUnitsProps): JSX.Element => {
    const [scrollTop, setScrollTop] = React.useState(0);
    // 监听滚动事件，计算虚拟滚动的泳道
    useEventBus(eventType, (value) => setScrollTop(value as number));
    const flattenUnitsAll = computed(() => orderOptions.preOrderFlatten(getRootUnit(session.units), 0, orderOptions.options)).get();
    const flattenUnits = computed(() => flattenUnitsAll.filter(unit => unit.isUnitVisible && !unit.isMerged)).get();
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

    const [isSelecting, setIsSelecting] = useState(false);
    const [selectedUnitKeys, setSelectedUnitKeys] = useState<Set<string>>(new Set());
    const startPoint = useRef({ x: 0, y: 0 });
    const unitsRefs = useRef(new Map());

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

    useEffect(() => {
        const mainContainer = document.getElementById('main-container');
        mainContainer?.addEventListener('mouseup', handleMouseUp);
        mainContainer?.addEventListener('mouseleave', handleMouseLeave);
        return () => {
            mainContainer?.removeEventListener('mouseup', handleMouseUp);
            mainContainer?.removeEventListener('mouseleave', handleMouseLeave);
        };
    }, []);

    const handleMouseUp = (): void => {
        setIsSelecting(false);
        for (const [key, value] of unitsRefs.current) {
            if (value === null) {
                unitsRefs.current.delete(key);
            }
        }
    };
    const handleMouseLeave = (e: MouseEvent): void => {
        setIsSelecting(false);
    };

    const handleMouseMove = (e: React.MouseEvent): void => {
        if (!isSelecting) { return; }
        if (session.selectedRangeIsLock) {
            return;
        }
        const top = Math.min(e.clientY, startPoint.current.y);
        const bottom = Math.max(e.clientY, startPoint.current.y);
        const newSelected: Set<string> = new Set();
        for (const [key, value] of unitsRefs.current) {
            const rect = value?.getBoundingClientRect();

            if (rect !== undefined && rect.top < bottom && rect.bottom > top) {
                newSelected.add(key);
            }
        }

        setSelectedUnitKeys(newSelected);
    };

    return <div ref={ref} style={{ display: 'flex', flexDirection: 'column', height: totalHeight }} className="laneView"
        onMouseDown={(e): void => {
            // 禁止在Unit Info区域触发框选
            if (e.clientX - PAGE_PADDING > laneInfoWidth) {
                setIsSelecting(true);
                startPoint.current = { x: e.clientX, y: e.clientY };
            }
        }}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
    >
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: headOffset }} />
        {flattenUnits.filter((_, i) => first <= i && i < last).map((unit) => {
            if (!isPinned(unit) || unit.pinType === 'copied') {
                const unitKey = getAutoKey(unit);
                return <Unit
                    ref={(el): void => {
                        unitsRefs.current.set(unitKey, el);
                    }}
                    key={unitKey}
                    laneInfoWidth={laneInfoWidth}
                    unit={unit}
                    session={session}
                    hasPinButton={hasPinButton}
                    hasExpandIcon={true}
                    isVisible={true}
                    isPinned={isPinned(unit)}
                    isSonPinned={isSonPinned(unit)}
                    isSelecting={isSelecting}
                    inRangeUnitKeys={selectedUnitKeys}
                />;
            } else {
                return false;
            }
        },
        )}
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: tailOffset }} />
    </div>;
});

const TableScroller = styled.div`
    flex-grow: 1;
    overflow-y: scroll;
    overflow-x: hidden;
    border-top: solid 1px ${(props): string => props.theme.borderColor};
    user-select: none;
`;

interface ScrollerProps {
    id: string;
    children: JSX.Element | null;
    session: Session;
    eventType: string;
    orderOptions: OrderOptions;
    unitsArea: InsightUnit[];
    supportJump: boolean;
    shouldUpdateScrollTop?: boolean;
}

export const Scroller = React.forwardRef(({
    id, session, children, eventType, orderOptions: sorderOptions, unitsArea, supportJump, shouldUpdateScrollTop,
}: ScrollerProps, ref: React.ForwardedRef<HTMLDivElement>): JSX.Element => {
    // 广播滚动事件
    function scroll(e: React.UIEvent<HTMLDivElement>): void {
        const scrollTop = e.currentTarget.scrollTop;
        eventBus.emit(eventType, scrollTop);
        if (shouldUpdateScrollTop !== undefined && shouldUpdateScrollTop) {
            // 修改session.scrollTop
            runInAction(() => {
                session.scrollTop = scrollTop;
            });
        }
    }

    function mouseEnter(): void {
        runInAction(() => {
            session.scrollArea = eventType === EventType.PINNEDUNITWRAPPERSCROLL ? 'pinned' : 'unpinned';
        });
    }

    function mouseLeave(): void {
        runInAction(() => {
            session.scrollArea = '';
        });
    }

    // 切换session滚动到之前记录的地方
    React.useEffect(() => {
        (ref as React.MutableRefObject<HTMLDivElement | null>).current?.scrollTo(0, session.scrollTop);
    }, [session]);

    // 跳转到指定泳道
    useJumpTarget(session, unitsArea, supportJump, sorderOptions, (ref as React.MutableRefObject<HTMLDivElement | null>).current);

    return <TableScroller id={id} className={`laneWrapper ${eventType === EventType.PINNEDUNITWRAPPERSCROLL ? 'pinnedScrollArea' : ''}`}
        onScroll={scroll} ref={ref} onMouseEnter={mouseEnter} onMouseLeave={mouseLeave}>
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
    return <Scroller id={UNIT_WRAPPER_SCROLLER_ID} session={session} unitsArea={session.units} supportJump={true}
        ref={ref} orderOptions={orderOptions} eventType={EventType.UNITWRAPPERSCROLL} shouldUpdateScrollTop>
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
