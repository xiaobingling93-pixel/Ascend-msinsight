/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useRef, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import type { Session } from '../entity/session';
import type { ChartInteractorHandles, InteractorMouseState } from './charts/ChartInteractor/ChartInteractor';
import type { ThreadTrace, ThreadMetaData, CardMetaData, ThreadTraceRequest } from '../entity/data';
import { preOrderFlatten, TimeStamp } from '../entity/common';
import connector from '../connection';
import { type ChartDesc, type InsightUnit, unit } from '../entity/insight';
import { Tooltip } from 'ascend-components';
import type { StackStatusConfig } from '../entity/chart';
import { getTimeOffsetKey } from '../insight/units/utils';
import { isPinned, switchPinned } from './ChartContainer/unitPin';
import { getRootUnit } from '../utils';
import { getAutoKey } from '../utils/dataAutoKey';
export const MAX_ZOOM_COUNT = 10000;
interface Position {
    left: string;
    top: string;
};

interface Props {
    session: Session;
    interactorMouseState: InteractorMouseState;
    theme?: Theme;
    chartInteractorRef: React.RefObject<ChartInteractorHandles>;
}

interface SelectedUnitStatus {
    isThreadNameStartWithGroup: boolean;
    isGroupCommunicationUnit: boolean;
    groupNameValue: string;
    isPinned: boolean;
}

interface SelectedUnitListStatus {
    isAllThreadNameStartWithGroup: boolean;
    isAllGroupCommunicationUnit: boolean;
    isSameGroupNameValue: boolean;
    groupNameValue: string;
    isAllPinned: boolean;
    isAllUnpinned: boolean;
}

interface MenuItemModel {
    name: string;
    key: string;
    event: (session: Session, menuItem?: MenuItemModel) => void;
    disabled?: boolean;
    visible: boolean;
    title?: string;
}

const MenuContainer = styled.div`
    padding: 3px 0;
    min-width: 200px;
    border-radius: ${(props): string => props.theme.borderRadiusBase};
    background-color:  ${(props): string => props.theme.contextMenuBgColor};
    position: fixed;
    z-index: 99999;
    transition: all .1s ease;
    box-shadow: ${(props): string => props.theme.boxShadowLight};
    user-select: none;
`;

const MenuItem = styled.div`
    padding: 4px 16px;
    color: ${(props): string => props.theme.textColorPrimary};
    white-space: nowrap; /* 防止文本换行 */
    overflow: hidden;    /* 隐藏溢出的内容 */
    text-overflow: ellipsis; /* 添加省略号 */
    max-width: 300px;        /* 设置一个固定宽度或根据需要调整 */

    &:not(.disabled):hover{
      background: ${(props): string => props.theme.primaryColorHover};
      color: #ffffff;
    }
    &.disabled{
        color: ${(props): string => props.theme.textColorDisabled};
    }
`;

export function setZoomHistory(session: Session, domainRange: { domainStart: TimeStamp; domainEnd: TimeStamp }): void {
    session.contextMenu.zoomHistory.push(domainRange);
    if (session.contextMenu.zoomHistory.length > MAX_ZOOM_COUNT) {
        session.contextMenu.zoomHistory = session.contextMenu.zoomHistory.slice(-MAX_ZOOM_COUNT);
    }
}

function zoomIntoSelection(session: Session): void {
    runInAction(() => {
        if (session.selectedRange !== undefined) {
            const domainRange = { domainStart: session.selectedRange[0], domainEnd: session.selectedRange[1] };
            session.domainRange = domainRange;
            setZoomHistory(session, domainRange);
        }
        session.contextMenu.isVisible = false;
    });
}
function fitToScreen(session: Session): void {
    runInAction(() => {
        if (session.selectedData !== undefined) {
            const selectedData = session.selectedData as ThreadTrace;
            const domainRange = { domainStart: selectedData.startTime, domainEnd: selectedData.startTime + selectedData.duration };
            session.domainRange = domainRange;
            setZoomHistory(session, domainRange);
        }
        session.contextMenu.isVisible = false;
    });
}

async function findInCommunication(session: Session): Promise<void> {
    if (!session.selectedData) {
        return;
    }
    const { name, cardId: rankId } = session.selectedData;
    const params = {
        rankId,
        name,
    };
    const res = await window.requestData('unit/kernelDetail', params, 'timeline');
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'communication',
            toModuleEvent: 'locateHCCL',
            params: {
                operatorName: session.selectedData?.name,
                iterationId: res?.step,
                stage: res?.group,
            },
        },
    });
    session.contextMenu.isVisible = false;
}

function undoZoom(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled) {
        return;
    }
    runInAction(() => {
        session.contextMenu.zoomHistory.pop();
        const zoomHistoryLength = session.contextMenu.zoomHistory.length;
        if (zoomHistoryLength === 0) {
            session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
        } else {
            session.domainRange = session.contextMenu.zoomHistory[zoomHistoryLength - 1];
        }
        session.contextMenu.isVisible = false;
    });
}
function resetZoom(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled) {
        return;
    }
    runInAction(() => {
        session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
        session.contextMenu.zoomHistory = [];
        session.contextMenu.isVisible = false;
    });
}

function getSameGroupNameUnits(session: Session): InsightUnit[] {
    if (session.selectedUnits.length === 0) {
        return [];
    }
    const selectedGroupNameValue = (session.selectedUnits[0].metadata as ThreadMetaData).groupNameValue;
    const flattenUnits = preOrderFlatten(getRootUnit(session.units), 0);
    return flattenUnits
        .filter(({ metadata }) => (metadata as ThreadMetaData)?.groupNameValue === selectedGroupNameValue);
}

function pinAllSameGroupNameValue(session: Session, menuItem?: MenuItemModel): void {
    if ((menuItem?.disabled ?? false) || session.selectedUnits.length === 0) {
        return;
    }
    const sameGroupNameValueUnits = getSameGroupNameUnits(session);
    if (sameGroupNameValueUnits.length === 0) {
        return;
    }
    const pinnedUnitKeys = session.pinnedUnits.map((item) => getAutoKey(item));
    const addUnits = sameGroupNameValueUnits.reduce<InsightUnit[]>((acc, curr): InsightUnit[] => {
        const key = getAutoKey(curr);
        if (pinnedUnitKeys.includes(key)) {
            return acc;
        }
        acc.push(curr);
        pinnedUnitKeys.push(key);
        return acc;
    }, []);
    runInAction(() => {
        session.pinnedUnits = [...session.pinnedUnits, ...addUnits];
        addUnits.forEach((item): void => switchPinned(item));
    });
    closeMenu(session);
}

function unpinAllSameGroupNameValue(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled ?? false) {
        return;
    }
    const sameGroupNameValueUnits = getSameGroupNameUnits(session);
    if (sameGroupNameValueUnits.length === 0) {
        return;
    }
    const { pinnedUnits } = session;
    const pinnedUnitKeys = pinnedUnits.map((item) => getAutoKey(item));
    const subtractUnits = sameGroupNameValueUnits.reduce<InsightUnit[]>((acc, curr): InsightUnit[] => {
        const key = getAutoKey(curr);
        const findIdx = pinnedUnitKeys.findIndex((pinnedKey: string): boolean => pinnedKey === key);
        if (findIdx < 0) {
            return acc;
        }
        acc.push(curr);
        pinnedUnits.splice(findIdx, 1);
        pinnedUnitKeys.splice(findIdx, 1);
        return acc;
    }, []);
    runInAction(() => {
        session.pinnedUnits = [...pinnedUnits];
        subtractUnits.forEach((item): void => switchPinned(item));
    });
    closeMenu(session);
}

function unpinAll(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled ?? false) {
        return;
    }
    const { pinnedUnits } = session;
    runInAction(() => {
        session.pinnedUnits = [];
        pinnedUnits.forEach((item): void => switchPinned(item));
    });
    closeMenu(session);
}

function hideUnit(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled) {
        return;
    }
    hideUnits(session, session.selectedUnits);
    runInAction(() => {
        session.contextMenu.isVisible = false;
        session.renderTrigger = !session.renderTrigger;
    });
}

function showHidedUnit(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled) {
        return;
    }
    showAllHidedUnits(session);
    runInAction(() => {
        session.contextMenu.isVisible = false;
        session.renderTrigger = !session.renderTrigger;
    });
}

function closeMenu(session: Session): void {
    runInAction(() => {
        session.contextMenu.isVisible = false;
    });
}

function openMenu(session: Session): void {
    if (session.selectedUnits.length === 0) {
        return;
    }
    runInAction(() => {
        session.contextMenu.isVisible = true;
    });
}

const showAllHidedUnits = (session: Session): void => {
    const setUnitNotHide = (insightUnit: InsightUnit): void => {
        runInAction(() => {
            insightUnit.isUnitVisible = true;
            if (insightUnit.name === 'Empty') {
                insightUnit.parent?.children?.pop();
            }
        });

        if (insightUnit.children) {
            insightUnit.children.forEach(child => setUnitNotHide(child));
        }
    };

    const handleEmptyUnit = (selectUnit: InsightUnit): void => {
        if (selectUnit.parent) {
            setUnitNotHide(selectUnit.parent);
        } else {
            // 顶层 EmptyUnit 的处理方式
            session.units.forEach(cardUnit => setUnitNotHide(cardUnit));
            runInAction(() => {
                session.units.pop();
            });
        }
    };

    const showChildrenUnits = (insightUnit: InsightUnit): void => {
        if (insightUnit.children) {
            insightUnit.children.forEach(child => setUnitNotHide(child));
        }
    };

    session.selectedUnits.forEach((selectUnit): void => {
        if (selectUnit !== undefined) {
            if (selectUnit.name === 'Empty') {
                handleEmptyUnit(selectUnit);
            } else {
                showChildrenUnits(selectUnit);
            }
        }
    });
};

export const EmptyUnit = unit<EmptyMetaData>({
    name: 'Empty',
    pinType: 'copied',
    renderInfo: (session: Session, metadata: { count: number}) => <Tooltip placement="leftBottom">
        <span style={{ marginLeft: 3, overflow: 'hidden', fontSize: 14, textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
            {metadata.count}{' units hidden'}
        </span>
    </Tooltip>,
});

interface EmptyMetaData {
    count: number;
    dataSource: DataSource;
};

function unpinUnitIfPinned(session: Session, insightUnit: InsightUnit): void {
    if (isPinned(insightUnit)) {
        runInAction(() => {
            session.pinnedUnits = session.pinnedUnits.filter((item) =>
                item.metadata !== insightUnit.metadata);
            switchPinned(insightUnit);
        });
    }
}

const hideUnits = (session: Session, selectUnits: InsightUnit[]): void => {
    const hideEveryUnit = (insightUnit: InsightUnit, ancestorsIsVisible: boolean = true): void => {
        /**
         * 对于有父子Unit被选中的情况
         * 由于代码是从顶向下找的，当发现第一个父Unit是选中的，就直接 hideSelectUnit，然后退出程序
         * 这样可以避免再次选中子Unit导致子Unit又创建一个Hidden Unit这种不希望的情形出现
         */
        if (selectUnits.some((item): boolean => item.metadata === insightUnit.metadata)) {
            unpinUnitIfPinned(session, insightUnit);
            if (ancestorsIsVisible) { // 只有祖先都是可见的情况下，才需要隐藏，否则不需要处理
                hideSelectUnit(insightUnit);
            }
        } else if (insightUnit.children) {
            for (const child of insightUnit.children) {
                // 如果当前泳道已经是不可见（隐藏）的，设为 false, 否则用本次传入的
                hideEveryUnit(child, insightUnit.isUnitVisible ? ancestorsIsVisible : false);
            }
        }
    };

    const hideSelectUnit = (insightUnit: InsightUnit): void => {
        runInAction(() => {
            insightUnit.isUnitVisible = false;
            session.selectedUnits = [];
            updateEmptyUnits(insightUnit);
        });

        if (insightUnit.children) {
            setChildrenUnitHide(insightUnit.children);
        }
    };

    const updateEmptyUnits = (insightUnit: InsightUnit): void => {
        if (insightUnit.parent === undefined) {
            const rankEmptyUnit = session.units.find(item => item.name === 'Empty');
            if (rankEmptyUnit !== undefined) {
                (rankEmptyUnit.metadata as EmptyMetaData).count++;
            } else {
                session.units.push(new EmptyUnit({
                    count: 1,
                    dataSource: (insightUnit.metadata as CardMetaData).dataSource,
                } as EmptyMetaData));
            }
        } else {
            const emptyUnit = insightUnit.parent.children?.find(item => item.name === 'Empty');
            if (emptyUnit !== undefined) {
                (emptyUnit.metadata as EmptyMetaData).count++;
            } else {
                insightUnit.parent.children?.push(new EmptyUnit({ count: 1 } as EmptyMetaData));
            }
        }
    };

    for (const sessionUnit of session.units) {
        hideEveryUnit(sessionUnit);
    }
};

const setChildrenUnitHide = (units: InsightUnit[]): void => {
    const hideChildrenUnit = (insightUnit: InsightUnit): void => {
        runInAction(() => {
            insightUnit.isUnitVisible = false;
        });
        if (insightUnit.children) {
            for (const child of insightUnit.children) {
                hideChildrenUnit(child);
            }
        }
    };
    for (const insightUnit of units) {
        hideChildrenUnit(insightUnit);
    }
};

const isShowHideText = (session: Session): boolean => {
    // 必须只选中一个才能显示“显示全部已隐藏泳道”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    if (session.selectedUnits[0].name === 'Empty') {
        return true;
    }
    if (session.selectedUnits[0].children) {
        for (const child of session.selectedUnits[0].children) {
            if (!child.isUnitVisible) {
                return true;
            }
        }
    }
    return false;
};

const isHideText = (session: Session): boolean => {
    if (session.selectedUnits.length === 0) {
        return false;
    }
    return session.selectedUnits.every((item): boolean => item.name !== 'Empty');
};

function showInEventsView(session: Session, menuItem?: MenuItemModel): void {
    if (menuItem?.disabled) {
        return;
    }
    runInAction(() => {
        session.showEvent = !session.showEvent;
        session.eventUnits = session.selectedUnits;
        session.contextMenu.isVisible = false;
    });
}

const isShowEventMenu = (session: Session): boolean => {
    // 必须只选中一个才能显示“跳转数据窗口事件视图”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectUnit = session.selectedUnits[0];
    if (selectUnit === undefined || session.isSimulation) {
        return false;
    }
    if (['Empty', 'Card', 'Counter', 'Root'].includes(selectUnit.name)) {
        return false;
    }
    if (selectUnit.children) {
        for (const child of selectUnit.children) {
            if (child.name === 'Counter') {
                return false;
            }
        }
    }
    if ((selectUnit.metadata as ThreadMetaData).threadName?.includes('Plane')) {
        return false;
    }
    return true;
};

const showPythonFunction = (session: Session): void => {
    // 必须只选中一个才能调用“显示、隐藏python调用栈”菜单项
    if (session.selectedUnits.length !== 1) {
        return;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return;
    }
    const metadata = selectedUnit.metadata as ThreadMetaData;
    if (metadata.cardId !== undefined) {
        const pythonFunctionConfig = session.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>;
        const isFilteredPythonFunction = pythonFunctionConfig?.[`${metadata.cardId}_${metadata.threadName}`] ?? false;
        runInAction(() => {
            session.unitsConfig.filterConfig.pythonFunction = { ...pythonFunctionConfig, [`${metadata.cardId}_${metadata.threadName}`]: !isFilteredPythonFunction };
            session.linkLines = {};
            session.singleLinkLine = {};
            session.renderTrigger = !session.renderTrigger;
        });
    }
    runInAction(() => {
        session.contextMenu.isVisible = false;
    });
};
const isShowPythonFunction = (session: Session): boolean => {
    // 必须只选中一个才能显示“显示、隐藏python调用栈”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return false;
    }
    return selectedUnit.havePythonFunction ?? false;
};

const getShowPythonFunctionButtonText = (session: Session, t: TFunction): string => {
    // 必须只选中一个才能显示菜单项名
    if (session.selectedUnits.length !== 1) {
        return '';
    }
    let isFilteredPythonFunction = true;
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return '';
    }
    const metadata = selectedUnit.metadata as ThreadMetaData;
    if (metadata.cardId !== undefined) {
        isFilteredPythonFunction = (session.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>)?.[`${metadata.cardId}_${metadata.threadName}`] ?? false;
    }

    return isFilteredPythonFunction ? t('Show python call stack') : t('Hide python call stack');
};

const getAutoUnitHeightButtonText = (session: Session, t: TFunction): string => {
    return session.autoAdjustUnitHeight ? t('Disable auto unit height') : t('Enable auto unit height');
};

const expandUnits = (_unit: InsightUnit, shouldExpand: boolean): void => {
    if (_unit.children && _unit.children?.length > 0) {
        _unit?.children?.forEach(childUnit => {
            expandUnits(childUnit, shouldExpand);
            childUnit.isExpanded = shouldExpand;
            if (childUnit.name === 'Thread' && childUnit.collapsible) {
                const chart = childUnit.chart as ChartDesc<'stackStatus'>;
                (chart.config as StackStatusConfig).isCollapse = shouldExpand;
                childUnit.collapseAction?.(childUnit);
            }
        });
    }
};
const collapseOrExpandAll = (session: Session, menuItem?: MenuItemModel): void => {
    // 必须只选中一个才能调用“收起、展开全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return;
    }
    const selectedUnit = session.selectedUnits[0];
    const shouldExpand = menuItem?.key === 'expandAll';
    runInAction(() => {
        if (selectedUnit !== undefined) {
            expandUnits(selectedUnit, shouldExpand);
            selectedUnit.isExpanded = true;
        }
        session.renderTrigger = !session.renderTrigger;
        session.contextMenu.isVisible = false;
    });
};

const hideOrShowFlagEvents = (session: Session, menuItem?: MenuItemModel): void => {
    if (!session.isSimulation) {
        return;
    }
    runInAction(() => {
        session.areFlagEventsHidden = !session.areFlagEventsHidden;
        if (session.areFlagEventsHidden) {
            session.singleLinkLine = {};
            session.linkLines = {};
            session.renderTrigger = !session.renderTrigger;
        }
        session.contextMenu.isVisible = false;
    });
};

const setUpUintOffset = (session: Session, insightUint: InsightUnit, offsetValue: number): void => {
    const prevObj = session.unitsConfig.offsetConfig.timestampOffset;
    if (insightUint.children !== undefined && insightUint.children.length > 0) {
        for (const item of insightUint.children) {
            const key = getTimeOffsetKey(session, item.metadata as ThreadTraceRequest);
            session.unitsConfig.offsetConfig.timestampOffset[key] = offsetValue;
        }
    }
    session.unitsConfig.offsetConfig.timestampOffset = {
        ...prevObj,
        [(insightUint.metadata as CardMetaData).cardId]: (offsetValue),
    };
};

const clearOrRecoverCardDefaultOffset = (session: Session, menuItem?: MenuItemModel): void => {
    runInAction(() => {
        session.units.forEach((insightUint) => {
            const offsetValue = insightUint.alignStartTimestamp as number;
            setUpUintOffset(session, insightUint, offsetValue);
        });
        session.benchMarkData = undefined;
        session.alignSliceData = [];
        session.contextMenu.isVisible = false;
    });
};

const setBaseSlice = (session: Session, menuItem?: MenuItemModel): void => {
    runInAction(() => {
        session.benchMarkData = session.selectedData;
        session.contextMenu.isVisible = false;
    });
};

const clearBaseSlice = (session: Session, menuItem?: MenuItemModel): void => {
    runInAction(() => {
        session.benchMarkData = undefined;
        session.alignSliceData = [];
        session.alignRender = !session.alignRender;
        session.contextMenu.isVisible = false;
    });
};

const toggleAutoUnitHeight = (session: Session): void => {
    runInAction(() => {
        session.autoAdjustUnitHeight = !session.autoAdjustUnitHeight;
        session.contextMenu.isVisible = false;
        session.renderTrigger = !session.renderTrigger;
    });
};

const haveExpandedChildren = (_unit: InsightUnit): boolean => {
    if (!_unit.collapsible || !_unit.children || _unit.children.length === 0) {
        return false;
    }

    return _unit.children.some(child => (child.collapsible !== undefined && child.collapsible && child.isExpanded) || haveExpandedChildren(child));
};

const haveCollapsedChildren = (_unit: InsightUnit): boolean => {
    if (!_unit.collapsible || !_unit.children || _unit.children.length === 0) {
        return false;
    }

    return _unit.children.some(child => (child.collapsible !== undefined && child.collapsible && !child.isExpanded) || haveCollapsedChildren(child));
};

const isCollapseAllVisible = (session: Session): boolean => {
    // 必须只选中一个才能显示“收起全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit !== undefined) {
        return haveExpandedChildren(selectedUnit);
    }
    return false;
};

const isExpandAllVisible = (session: Session): boolean => {
    // 必须只选中一个才能显示“展开全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit !== undefined) {
        const isCollapsed = selectedUnit.collapsible && !selectedUnit.isExpanded;
        const haveChildUnits = selectedUnit.children && selectedUnit.children.length > 0;
        if (isCollapsed && haveChildUnits) {
            return true;
        }
        return haveCollapsedChildren(selectedUnit);
    }
    return false;
};

const isShowFlagEventsVisible = (session: Session): boolean => {
    return session.isSimulation && session.areFlagEventsHidden;
};

const isHideFlagEventsVisible = (session: Session): boolean => {
    return session.isSimulation && !session.areFlagEventsHidden;
};

const setBaseSliceVisible = (session: Session): boolean => {
    if (session.selectedData === undefined) {
        return false;
    }
    if (session.benchMarkData === undefined) {
        return true;
    }
    const selectedData = session.selectedData as ThreadTrace;
    const benchMarkData = session.benchMarkData as ThreadTrace;
    if (selectedData.id === benchMarkData.id && selectedData.threadId === benchMarkData.threadId) {
        return false;
    }
    return true;
};

const clearBaseSliceVisible = (session: Session): boolean => {
    return session.benchMarkData !== undefined;
};
function adjustMenuPosition({ menu, setPosition, xPos, yPos }: {
    menu: HTMLDivElement;
    setPosition: (_: Position) => void;
    xPos: React.MutableRefObject<number>;
    yPos: React.MutableRefObject<number>;
}): void {
    const winWidth = document.documentElement.clientWidth || document.body.clientWidth;
    const winHeight = document.documentElement.clientHeight || document.body.clientHeight;
    if (xPos.current >= winWidth - menu.offsetWidth) {
        xPos.current -= menu.offsetWidth;
    }
    if (yPos.current > winHeight - menu.offsetHeight) {
        yPos.current -= menu.offsetHeight;
    }
    setPosition({ left: `${xPos.current}px`, top: `${yPos.current}px` });
    menu.focus();
}

function calculateSelectedUnitStatus(selectedUnit: InsightUnit): SelectedUnitStatus {
    const hasStringValue = (str: string = ''): boolean => {
        return str !== '';
    };
    const metadata = selectedUnit.metadata as ThreadMetaData;
    return {
        isThreadNameStartWithGroup: metadata?.threadName?.startsWith('Group') ?? false,
        isGroupCommunicationUnit: hasStringValue(metadata?.groupNameValue),
        groupNameValue: (selectedUnit.metadata as ThreadMetaData)?.groupNameValue ?? '',
        isPinned: isPinned(selectedUnit),
    };
}

function calculateSelectedUnitListStatus(selectedUnits: InsightUnit[]): SelectedUnitListStatus {
    const selectedUnitStatuses = selectedUnits.map(calculateSelectedUnitStatus);
    const isAllThreadNameStartWithGroup = selectedUnitStatuses
        .every(({ isThreadNameStartWithGroup }) => isThreadNameStartWithGroup);
    const isAllGroupCommunicationUnit = selectedUnitStatuses
        .every(({ isGroupCommunicationUnit }) => isGroupCommunicationUnit);

    const isAllPinned = selectedUnitStatuses.every((status) => status.isPinned);
    const isAllUnpinned = selectedUnitStatuses.every((status) => !status.isPinned);

    const groupNameValue = selectedUnitStatuses?.[0]?.groupNameValue ?? '';
    const isSameGroupNameValue = isAllGroupCommunicationUnit && selectedUnitStatuses
        .reduce((acc, curr) => acc && curr.groupNameValue === groupNameValue, true);
    return {
        isAllThreadNameStartWithGroup,
        isAllGroupCommunicationUnit,
        isSameGroupNameValue,
        groupNameValue,
        isAllPinned,
        isAllUnpinned,
    };
}

function buildPinAndUnpinByGroupCommunicationUnitNameMenuItem(
    selectedUnitListStatus: SelectedUnitListStatus,
    t: TFunction): MenuItemModel[] {
    return [
        {
            name: t('Pin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            title: t('Pin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            key: 'pinByGroupNameValue',
            event: pinAllSameGroupNameValue,
            disabled: !selectedUnitListStatus.isAllUnpinned,
            visible: selectedUnitListStatus.isAllUnpinned && selectedUnitListStatus.isSameGroupNameValue,
        },
        {
            name: t('Unpin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            title: t('Unpin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            key: 'unpinByGroupNameValue',
            event: unpinAllSameGroupNameValue,
            disabled: !selectedUnitListStatus.isAllPinned,
            visible: selectedUnitListStatus.isAllPinned && selectedUnitListStatus.isSameGroupNameValue,
        },
    ];
}

const getMenuItems = (props: Props, t: TFunction): JSX.Element => {
    const { session, session: { contextMenu: { zoomHistory } } } = props;
    if (!Array.isArray(session.selectedUnits) || session.selectedUnits.length === 0) {
        return <></>;
    }
    // 为多选 unit 功能做准备
    const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
    // session.selectedData 在这里指选中的算子数据
    const isCommunicationOperator = (session.selectedData?.name as string)?.startsWith('hcom_') ?? false;
    const findInCommunicationVisible =
        selectedUnitListStatus.isAllThreadNameStartWithGroup && isCommunicationOperator && session.isCluster;
    const menuItems: MenuItemModel[] = [
        { name: t('Fit to screen'), key: 'fitToScreen', event: fitToScreen, disabled: session.selectedData?.duration === 0, visible: session.selectedData !== undefined },
        { name: t('Find in Communication'), key: 'findInCommunication', event: findInCommunication, visible: findInCommunicationVisible },
        { name: t('Zoom into selection'), key: 'zoomIntoSelection', event: zoomIntoSelection, visible: session.selectedRange !== undefined },
        { name: `${t('Undo Zoom')} (${zoomHistory.length})`, key: 'undoZoom', event: undoZoom, disabled: zoomHistory.length === 0, visible: true },
        { name: t('Reset Zoom'), key: 'resetZoom', event: resetZoom, disabled: zoomHistory.length === 0, visible: true },
        { name: t('Unpin All'), key: 'unpinAll', event: unpinAll, disabled: !selectedUnitListStatus.isAllPinned, visible: selectedUnitListStatus.isAllPinned },
        ...buildPinAndUnpinByGroupCommunicationUnitNameMenuItem(selectedUnitListStatus, t),
        { name: t('Hide'), key: 'hide', event: hideUnit, disabled: false, visible: isHideText(session) },
        { name: t('Show All Hidden'), key: 'showAllHidden', event: showHidedUnit, disabled: false, visible: isShowHideText(session) },
        { name: t('Show in events view'), key: 'showInEventsView', event: showInEventsView, disabled: false, visible: isShowEventMenu(session) },
        { name: getShowPythonFunctionButtonText(session, t), key: 'showPythonFunction', event: showPythonFunction, disabled: false, visible: isShowPythonFunction(session) },
        { name: t('Collapse all'), key: 'collapseAll', event: collapseOrExpandAll, disabled: false, visible: isCollapseAllVisible(session) },
        { name: t('Expand all'), key: 'expandAll', event: collapseOrExpandAll, disabled: false, visible: isExpandAllVisible(session) },
        { name: t('Hide flag events'), key: 'hideFlagEvents', event: hideOrShowFlagEvents, disabled: false, visible: isHideFlagEventsVisible(session) },
        { name: t('Show flag events'), key: 'showFlagEvents', event: hideOrShowFlagEvents, disabled: false, visible: isShowFlagEventsVisible(session) },
        { name: getAutoUnitHeightButtonText(session, t), key: 'autoUnitHeight', event: toggleAutoUnitHeight, visible: true },
        { name: t('Recover cards default offset'), key: 'recoverDefaultOffset', event: clearOrRecoverCardDefaultOffset, visible: true },
        { name: t('Set base slice'), key: 'setBaseSlice', event: setBaseSlice, visible: setBaseSliceVisible(session) },
        { name: t('Clear base slice'), key: 'clearBaseSlice', event: clearBaseSlice, visible: clearBaseSliceVisible(session) },
    ];

    return <>
        {menuItems.filter(menuItem => menuItem.visible).map(item => (<MenuItem className={`menu-item ${item.disabled ? 'disabled' : ''}`} key={item.key}
            title={item.title}
            onClick={(e): void => {
                if (item.disabled) {
                    return;
                }
                item.event(session, item);
            }}>
            {item.name}
        </MenuItem>))}
    </>;
};

const Menu = (props: Props): JSX.Element => {
    const { session } = props;
    const menuRef = useRef<HTMLDivElement>(null);
    const [position, setPosition] = useState<Position>({ left: '0px', top: '0px' });
    const xPos = useRef(0); const yPos = useRef(0);
    const { t } = useTranslation('timeline', { keyPrefix: 'contextMenu' });

    useEffect(() => {
        document.addEventListener('contextmenu', handleContextMenu);
        window.addEventListener('mousedown', handleMouseDown);
        window.addEventListener('wheel', handleCloseMenu);

        return () => {
            document.removeEventListener('contextmenu', handleContextMenu);
            window.removeEventListener('mousedown', handleMouseDown);
            window.removeEventListener('wheel', handleCloseMenu);
        };
    });

    useEffect(() => {
        const menu = menuRef.current;
        if (session.contextMenu.isVisible && menu !== null) {
            adjustMenuPosition({ menu, setPosition, xPos, yPos });
        }
    }, [session.contextMenu.isVisible]);

    const handleContextMenu = (event: MouseEvent): void => {
        const targetElement = event.target as HTMLElement;
        if (targetElement?.closest('.laneWrapper') !== null) {
            xPos.current = event.clientX; yPos.current = event.clientY;
            setPosition({ left: `${xPos.current}px`, top: `${yPos.current}px` });
            openMenu(session);
        }
    };

    const handleMouseDown = (e: MouseEvent): void => {
        if ((e.target as HTMLElement)?.parentNode !== menuRef.current) {
            closeMenu(session);
        }
    };

    const handleCloseMenu = (): void => {
        closeMenu(session);
    };

    return (
        session.contextMenu.isVisible
            ? <MenuContainer ref={menuRef} style={{ ...position }} tabIndex={-1} onBlur={(): void => {
                closeMenu(session);
            }} >
                {getMenuItems(props, t)}
            </MenuContainer>
            : <></>
    );
};

export const ContextMenu = observer(Menu);
