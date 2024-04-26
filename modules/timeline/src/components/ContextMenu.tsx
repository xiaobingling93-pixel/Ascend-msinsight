/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useRef, useEffect, useState } from 'react';
import type { Theme } from '@emotion/react';
import styled from '@emotion/styled';
import type { Session } from '../entity/session';
import type { ChartInteractorHandles, InteractorMouseState } from './charts/ChartInteractor/ChartInteractor';
import { runInAction } from 'mobx';
import type { ThreadTrace } from '../entity/data';
import { observer } from 'mobx-react';
import type { TimeStamp } from '../entity/common';
import connector from '../connection';

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
    min-width: 200px;
    border: 1px solid #ccc;
    background-color:  ${(props): string => props.theme.contentBackgroundColor};
    position: fixed;
    z-index: 99999;
    transition: all .1s ease;
`;

const MenuItem = styled.div`
    padding: 4px 20px;
    color: ${(props): string => props.theme.fontColor};
    text-align: left;

    &:not(.disabled):hover{
      background: ${(props): string => props.theme.selectedChartBorderColor};
      color: #ffffff;
    }
    &.disabled{
      color: ${(props): string => props.theme.disabledFontColor};
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

function findInCommunication(session: Session): void {
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'communication',
            toModuleEvent: 'locateHCCL',
            params: {
                operatorName: session.selectedData?.name,
                iterationId: session.selectedData?.step,
                stage: session.selectedData?.group,
            },
        },
    });
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

const getMenuItems = (props: Props): JSX.Element => {
    const { session, session: { contextMenu: { zoomHistory } } } = props;

    const menuItems: MenuItemModel[] = [{
        name: 'Fit to screen',
        key: 'fitToScreen',
        event: fitToScreen,
        visible: session.selectedData !== undefined,
    }, {
        name: 'Find in Communication',
        key: 'findInCommunication',
        event: findInCommunication,
        visible: ((session.selectedData?.name as string)?.startsWith('hcom_') && session.isCluster) ?? false,
    }, {
        name: 'Zoom into selection',
        key: 'zoomIntoSelection',
        event: zoomIntoSelection,
        visible: session.selectedRange !== undefined,
    }, {
        name: `Undo Zoom (${zoomHistory.length})`,
        key: 'undoZoom',
        event: undoZoom,
        disabled: zoomHistory.length === 0,
        visible: true,
    }, {
        name: 'Reset Zoom',
        key: 'resetZoom',
        event: resetZoom,
        disabled: zoomHistory.length === 0,
        visible: true,
    }];

    return <>
        {menuItems.filter(menuItem => menuItem.visible).map(item => (<MenuItem className={`menu-item ${item.disabled ? 'disabled' : ''}`} key={item.key}
            onClick={(e): void => item.event(session, item)}>
            {item.name}
        </MenuItem>))}
    </>;
};

const Menu = (props: Props): JSX.Element => {
    const { session } = props;
    const menuRef = useRef<HTMLDivElement>(null);
    const [position, setPosition] = useState<Position>({ left: '0px', top: '0px' });
    const xPos = useRef(0); const yPos = useRef(0);

    useEffect(() => {
        document.addEventListener('contextmenu', handleContextMenu);
        window.addEventListener('mousedown', (e) => handleMouseDown(e));
        window.addEventListener('wheel', () => closeMenu(session));

        return () => {
            document.removeEventListener('contextmenu', handleContextMenu);
            window.removeEventListener('mousedown', (e) => handleMouseDown(e));
            window.removeEventListener('wheel', () => closeMenu(session));
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

    return (
        session.contextMenu.isVisible
            ? <MenuContainer ref={menuRef} style={{ ...position }} tabIndex={-1} onBlur={(): void => {
                closeMenu(session);
            }} >
                {getMenuItems(props)}
            </MenuContainer>
            : <></>
    );
};

export const ContextMenu = observer(Menu);
