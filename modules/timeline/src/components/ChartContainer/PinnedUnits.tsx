/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import * as React from 'react';
import { useEffect, useRef } from 'react';
import { computed } from 'mobx';
import { observer } from 'mobx-react';
import { preOrderPinnedFlatten } from '../../entity/common';
import type { TreeNode } from '../../entity/common';
import type { InsightUnit } from '../../entity/insight';
import styled from '@emotion/styled';
// support utils/types
import type { Session } from '../../entity/session';
import { getAutoKey } from '../../utils/dataAutoKey';
// same level infer
import { Unit, Scroller, computeVisibleUnitRange } from './Units';
import { isPinned, isSonPinned } from './unitPin';
import { EventType, useEventBus } from '../../utils/eventBus';
import { PINNED_UNIT_WRAPPER_SCROLLER_ID } from './Units/Units';

const PinnedUnitsContainer = styled.div`
    box-shadow: 1px 2px 11px 1px ${(props): string => props.theme.shadowBackgroundColor};
    z-index: 1;
    flex-grow: 1;
`;

const handleWheel = (event: WheelEvent): void => {
    if (event.ctrlKey) {
        event.preventDefault();
    }
};

const INVISIBLE_UNITS_PLACEHOLDER = 'invisible-units-placeholder';

const orderOptions = {
    preOrderFlatten: preOrderPinnedFlatten,
    options: {
        when: (unit: TreeNode<InsightUnit>): boolean => unit.isExpanded,
        bypass: (unit: TreeNode<InsightUnit>): boolean => unit.type === 'transparent',
        exclude: (unit: TreeNode<InsightUnit>): boolean => !unit.isDisplay,
        excludeEx: (unit: TreeNode<InsightUnit>): boolean => isSonPinned(unit),
    },
};

const FlattenUnits = observer(({ session, height, laneInfoWidth, eventType }:
{ session: Session; laneInfoWidth: number; height: number; eventType: string }, ref: React.ForwardedRef<HTMLDivElement>): JSX.Element => {
    const [scrollTop, setScrollTop] = React.useState(0);
    useEventBus(eventType, (value) => setScrollTop(value as number));
    const flattenUnits = computed(() => preOrderPinnedFlatten(session.pinnedUnits, 0, orderOptions.options)).get();

    const [first, last] = React.useMemo(
        () => computeVisibleUnitRange(flattenUnits, height, scrollTop),
        [session.pinnedUnits, flattenUnits, height, scrollTop],
    );
    const headOffset = React.useMemo(
        () => flattenUnits.filter((_, i) => i < first).reduce((prev, cur) => prev + cur.height(), 0),
        [flattenUnits, first],
    );
    const visibleUnitsHeight = React.useMemo(
        () => flattenUnits.filter((_, i) => first <= i && i < last).reduce((prev, cur) => prev + cur.height(), 0),
        [flattenUnits, first, last],
    );
    const tailOffset = React.useMemo(
        () => flattenUnits.filter((_, i) => i >= last).reduce((prev, cur) => prev + cur.height(), 0),
        [flattenUnits, last],
    );
    const totalHeight = React.useMemo(() => headOffset + visibleUnitsHeight + tailOffset, [headOffset, visibleUnitsHeight, tailOffset]);

    const wrapRef = useRef<HTMLDivElement>(null);
    useEffect(() => {
        const element = wrapRef.current;
        if (element) {
            element.addEventListener('wheel', handleWheel, { passive: false });
            return () => { element.removeEventListener('wheel', handleWheel); };
        }
        return () => {};
    }, []);
    return <PinnedUnitsContainer ref={wrapRef} style={{ display: 'flex', flexDirection: 'column', height: totalHeight }}>
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: headOffset }} />
        {flattenUnits.filter((_, i) => first <= i && i < last).map((pinnedUnit) => <Unit
            key={getAutoKey(pinnedUnit)}
            laneInfoWidth={laneInfoWidth}
            unit={pinnedUnit}
            session={session}
            hasPinButton={true}
            hasExpandIcon={true}
            isVisible={true}
            isPinned={isPinned(pinnedUnit)}
            isSonPinned={isSonPinned(pinnedUnit)}
        />)}
        <div className={INVISIBLE_UNITS_PLACEHOLDER} style={{ height: tailOffset }} />
    </PinnedUnitsContainer>;
});

const PUnits = ({ session, height, laneInfoWidth }:
{ session: Session; height: number; laneInfoWidth: number }, ref: React.ForwardedRef<HTMLDivElement>): JSX.Element => {
    return <Scroller id={PINNED_UNIT_WRAPPER_SCROLLER_ID} session={session} unitsArea={session.pinnedUnits} supportJump={false}
        ref={ref} orderOptions={orderOptions} eventType={EventType.PINNEDUNITWRAPPERSCROLL}>
        <FlattenUnits
            session={session}
            height={height}
            laneInfoWidth={laneInfoWidth}
            eventType={EventType.PINNEDUNITWRAPPERSCROLL} />
    </Scroller>;
};

export const PinnedUnits = React.forwardRef(PUnits);
