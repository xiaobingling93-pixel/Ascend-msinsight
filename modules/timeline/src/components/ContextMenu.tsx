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
import type { ThreadTrace, ThreadMetaData, CardMetaData } from '../entity/data';
import type { TimeStamp } from '../entity/common';
import connector from '../connection';
import { type ChartDesc, type InsightUnit, unit } from '../entity/insight';
import { Tooltip } from 'ascend-components';
import type { StackStatusConfig } from '../entity/chart';
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

interface MenuItemModel {
    name: string;
    key: string;
    event: (session: Session, menuItem?: MenuItemModel) => void;
    disabled?: boolean;
    visible: boolean;
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

    const selectUnit = session.selectedUnits[0];
    if (selectUnit !== undefined) {
        if (selectUnit.name === 'Empty') {
            handleEmptyUnit(selectUnit);
        } else {
            showChildrenUnits(selectUnit);
        }
    }
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

const hideUnits = (session: Session, selectUnit: InsightUnit[]): void => {
    const hideEveryUnit = (insightUnit: InsightUnit): void => {
        if (selectUnit[0].metadata === insightUnit.metadata) {
            hideSelectUnit(insightUnit);
        }
        if (insightUnit.children) {
            for (const child of insightUnit.children) {
                hideEveryUnit(child);
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
    if (session.selectedUnits[0] === undefined) {
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
    if (session.selectedUnits[0] === undefined) {
        return false;
    }
    return session.selectedUnits[0].name !== 'Empty';
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
    const selectedUnit = session.selectedUnits?.[0];
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
    const selectedUnit = session.selectedUnits?.[0];
    if (selectedUnit === undefined) {
        return false;
    }
    return selectedUnit.havePythonFunction ?? false;
};

const getShowPythonFunctionButtonText = (session: Session, t: TFunction): string => {
    let isFilteredPythonFunction = true;
    const selectedUnit = session.selectedUnits?.[0];
    if (selectedUnit === undefined) {
        return '';
    }
    const metadata = selectedUnit.metadata as ThreadMetaData;
    if (metadata.cardId !== undefined) {
        isFilteredPythonFunction = (session.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>)?.[`${metadata.cardId}_${metadata.threadName}`] ?? false;
    }

    return isFilteredPythonFunction ? t('Show python call stack') : t('Hide python call stack');
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
    const selectedUnit = session.selectedUnits?.[0];
    if (selectedUnit !== undefined) {
        return haveExpandedChildren(selectedUnit);
    }
    return false;
};

const isExpandAllVisible = (session: Session): boolean => {
    const selectedUnit = session.selectedUnits?.[0];
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
};

const getMenuItems = (props: Props, t: TFunction): JSX.Element => {
    const { session, session: { contextMenu: { zoomHistory } } } = props;
    const isGroupCommunicationUnit = (session.selectedUnits?.[0]?.metadata as ThreadMetaData)?.threadName?.startsWith('Group') ?? false;
    const isCommunicationOperator = (session.selectedData?.name as string)?.startsWith('hcom_') ?? false;
    const findInCommunicationVisible = isGroupCommunicationUnit && isCommunicationOperator && session.isCluster;

    const menuItems: MenuItemModel[] = [
        { name: t('Fit to screen'), key: 'fitToScreen', event: fitToScreen, disabled: session.selectedData?.duration === 0, visible: session.selectedData !== undefined },
        { name: t('Find in Communication'), key: 'findInCommunication', event: findInCommunication, visible: findInCommunicationVisible },
        { name: t('Zoom into selection'), key: 'zoomIntoSelection', event: zoomIntoSelection, visible: session.selectedRange !== undefined },
        { name: `${t('Undo Zoom')} (${zoomHistory.length})`, key: 'undoZoom', event: undoZoom, disabled: zoomHistory.length === 0, visible: true },
        { name: t('Reset Zoom'), key: 'resetZoom', event: resetZoom, disabled: zoomHistory.length === 0, visible: true },
        { name: t('Hide'), key: 'hide', event: hideUnit, disabled: false, visible: isHideText(session) },
        { name: t('Show All Hidden'), key: 'showAllHidden', event: showHidedUnit, disabled: false, visible: isShowHideText(session) },
        { name: t('Show in events view'), key: 'showInEventsView', event: showInEventsView, disabled: false, visible: isShowEventMenu(session) },
        { name: getShowPythonFunctionButtonText(session, t), key: 'showPythonFunction', event: showPythonFunction, disabled: false, visible: isShowPythonFunction(session) },
        { name: t('Collapse all'), key: 'collapseAll', event: collapseOrExpandAll, disabled: false, visible: isCollapseAllVisible(session) },
        { name: t('Expand all'), key: 'expandAll', event: collapseOrExpandAll, disabled: false, visible: isExpandAllVisible(session) },
        { name: t('Hide flag events'), key: 'hideFlagEvents', event: hideOrShowFlagEvents, disabled: false, visible: isHideFlagEventsVisible(session) },
        { name: t('Show flag events'), key: 'showFlagEvents', event: hideOrShowFlagEvents, disabled: false, visible: isShowFlagEventsVisible(session) },
    ];

    return <>
        {menuItems.filter(menuItem => menuItem.visible).map(item => (<MenuItem className={`menu-item ${item.disabled ? 'disabled' : ''}`} key={item.key}
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
