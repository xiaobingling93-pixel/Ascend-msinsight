/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { useMemo } from 'react';
import { message } from 'antd';
import { Line, Rectangle } from './shape';
import connector from '../../connection';
import { Session } from '../../entity/session';
import parallelismStore from '../../store/parallelism';
import { useParallelSwitchConditions } from './Context';
import { copyObjectToClipboard } from 'ascend-utils';

interface ContextMenuItem {
    name: string;
    label: string;
    action?: () => void;
}

interface UseContextMenuItemsProps {
    session: Session;
    activeRect: Rectangle | null;
    activeLine: Line | null;
    tooltipsData: Record<string, string> | null;
}

interface ContextMenuItemState extends UseContextMenuItemsProps {
    dyeingMode: string;
    setStartVal: (val: number) => void;
    setEndVal: (val: number) => void;
    startVal: number | null;
    endVal: number | null;
}

type MenuItemAction = (state: ContextMenuItemState) => ContextMenuItem;

const goToCommunicationDurationAnalysis = (line: Line | null): void => {
    if (line === null) {
        return;
    }
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'communication',
            toModuleEvent: 'viewCommunicationDurationAnalysis',
            params: {
                pgName: line.type,
                stage: `(${line.rectList.map(r => r.index).join(', ')})`,
            },
        },
    });
};

// 展开维度
const actionExpandDimension: MenuItemAction = ({ activeRect }) => {
    const { activeDimension, dimensionLevels } = parallelismStore;

    return {
        name: 'expandDimension',
        label: 'summary:contextMenu.Expand dimension',
        visible: false,
        disabled: activeDimension === dimensionLevels[dimensionLevels.length - 1],
        action: () => parallelismStore.expandDimension(activeRect),
    };
};

// 折叠维度
const actionCollapseDimension: MenuItemAction = ({ activeRect }) => {
    const { activeDimension, dimensionLevels } = parallelismStore;

    return {
        name: 'collapseDimension',
        label: 'summary:contextMenu.Collapse dimension',
        visible: false,
        disabled: activeDimension === dimensionLevels[0],
        action: () => parallelismStore.collapseDimension(activeRect),
    };
};

// 跳转到通信耗时分析
const actionViewCommAnalysis: MenuItemAction = ({ activeLine }) => ({
    name: 'viewCommunicationDurationAnalysis',
    label: 'summary:contextMenu.View Communication Duration Analysis',
    visible: activeLine !== null,
    action: () => {
        if (activeLine) {
            goToCommunicationDurationAnalysis(activeLine);
        }
    },
});

// 设置为最小筛选值
const actionSetAsLowerBound: MenuItemAction = ({ activeRect, dyeingMode, session, endVal, setStartVal }) => {
    const activeRectIndex = activeRect?.index;
    const value = activeRectIndex !== undefined
        ? session.performanceDataMap.get(activeRectIndex)?.[dyeingMode] ??
        session.performanceDataMap.get(activeRectIndex)?.commCompare?.[dyeingMode]
        : undefined;

    return {
        name: 'setAsLowerBound',
        label: 'summary:contextMenu.Set as Lower Bound',
        visible: dyeingMode !== 'None' && activeRectIndex !== undefined,
        action: () => {
            if (value !== undefined) {
                if (endVal !== null && value > endVal) {
                    message.warning('Minimum value cannot be greater than maximum value');
                    return;
                }
                setStartVal(value);
            }
        },
    };
};

// 设置为最大筛选值
const actionSetAsUpperBound: MenuItemAction = ({ activeRect, dyeingMode, session, startVal, setEndVal }) => {
    const activeRectIndex = activeRect?.index;
    const value = activeRectIndex !== undefined
        ? session.performanceDataMap.get(activeRectIndex)?.[dyeingMode] ??
        session.performanceDataMap.get(activeRectIndex)?.commCompare?.[dyeingMode]
        : undefined;

    return {
        name: 'setAsUpperBound',
        label: 'summary:contextMenu.Set as Upper Bound',
        visible: dyeingMode !== 'None' && activeRectIndex !== undefined,
        action: () => {
            if (value !== undefined) {
                if (startVal !== null && value < startVal) {
                    message.warning('Maximum value cannot be less than minimum value');
                    return;
                }
                setEndVal(value);
            }
        },
    };
};

// 复制属性
const actionCopyAttributes: MenuItemAction = ({ activeRect, tooltipsData }) => ({
    name: 'copyAttributes',
    label: 'summary:contextMenu.Copy attributes',
    visible: activeRect?.index !== undefined,
    action: () => {
        copyObjectToClipboard(tooltipsData);
    },
});

export const useContextMenuItems = (props: UseContextMenuItemsProps): ContextMenuItem[] => {
    const { activeRect, activeLine } = props;
    const { dyeingMode, setStartVal, setEndVal, startVal, endVal } = useParallelSwitchConditions();

    const actions: MenuItemAction[] = [
        actionExpandDimension,
        actionCollapseDimension,
        actionViewCommAnalysis,
        actionSetAsLowerBound,
        actionSetAsUpperBound,
        actionCopyAttributes,
    ];

    const fullState: ContextMenuItemState = {
        ...props,
        dyeingMode,
        setStartVal,
        setEndVal,
        startVal,
        endVal,
    };

    return useMemo(
        () => actions.map(action => action(fullState)),
        [activeRect, activeLine, dyeingMode]);
};
