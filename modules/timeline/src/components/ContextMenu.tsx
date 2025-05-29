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
import { unit } from '../entity/insight';

import {
    actionClearBenchmarkSlice,
    actionCollapseAllUnits,
    actionDisableAutoUnitHeight,
    actionEnableAutoUnitHeight,
    actionExpandAllUnits,
    actionFindInCommunication,
    actionFitToScreen,
    actionHideFlagEvents,
    actionHidePythonCallStack,
    actionHideUnits,
    actionLockSelection,
    actionRecoverDefaultOffset,
    actionResetZoom,
    actionSetBenchmarkSlice,
    actionShowFlagEvents,
    actionShowHiddenUnits,
    actionShowInEventsView,
    actionShowPythonCallStack,
    actionUndoZoom,
    actionUnLockSelection,
    actionUnpinAll,
    actionZoomIntoSelection,
    actionSetCardAlias,
    actionPinByGroupNameValue,
    actionUnpinByGroupNameValue,
    actionParseCardsOfRelatedGroup,
} from '../actions';
import { Action } from '../actions/types';
import { getShortcutFromShortcutName, ShortcutName } from '../actions/shortcuts';
import { EmptyMetaData } from '../entity/data';

interface Position {
    left: string;
    top: string;
}

interface Props {
    session: Session;
    interactorMouseState: InteractorMouseState;
    theme?: Theme;
    chartInteractorRef: React.RefObject<ChartInteractorHandles>;
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
    display: grid;
    grid-template-columns: 1fr 0.2fr;
    align-items: center;
    padding: 4px 16px;
    color: ${(props): string => props.theme.textColorPrimary};

    &:not(.disabled):hover{
      background: ${(props): string => props.theme.primaryColorHover};
      color: #ffffff;
    }
    &.disabled{
        color: ${(props): string => props.theme.textColorDisabled};
    }

    .menu-item__label {
        margin-right: 20px;
        white-space: nowrap; /* 防止文本换行 */
        overflow: hidden;    /* 隐藏溢出的内容 */
        text-overflow: ellipsis; /* 添加省略号 */
        max-width: 300px;        /* 设置一个固定宽度或根据需要调整 */
    }

    .menu-item__shortcut {
        opacity: 0.6;
    }
`;

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

export const EmptyUnit = unit<EmptyMetaData>({
    name: 'Empty',
    pinType: 'copied',
    renderInfo: (session: Session, metadata: { count: number}) =>
        <span style={{ marginLeft: 3, overflow: 'hidden', fontSize: 14, textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
            {metadata.count}{' units hidden'}
        </span>,
});

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

const contextMenuItems: Action[] = [
    actionFitToScreen,
    actionFindInCommunication,
    actionZoomIntoSelection,
    actionUndoZoom,
    actionResetZoom,
    actionUnpinAll,
    actionHideUnits,
    actionShowHiddenUnits,
    actionShowInEventsView,
    actionShowPythonCallStack,
    actionHidePythonCallStack,
    actionCollapseAllUnits,
    actionExpandAllUnits,
    actionHideFlagEvents,
    actionShowFlagEvents,
    actionEnableAutoUnitHeight,
    actionDisableAutoUnitHeight,
    actionRecoverDefaultOffset,
    actionSetBenchmarkSlice,
    actionClearBenchmarkSlice,
    actionLockSelection,
    actionUnLockSelection,
    actionSetCardAlias,
    actionPinByGroupNameValue,
    actionUnpinByGroupNameValue,
    actionParseCardsOfRelatedGroup,
];

const getMenuItems = (props: Props, t: TFunction): JSX.Element => {
    const { session } = props;
    if (!Array.isArray(session.selectedUnits) || session.selectedUnits.length === 0) {
        return <></>;
    }

    return <>
        {[contextMenuItems
            .filter(menuItem => menuItem.visible?.(session) ?? true)
            .map(item => {
                const disabled = item.disabled?.(session) ?? false;
                let label = '';
                if (typeof item.label === 'function') {
                    label = item.label(session, t);
                } else {
                    label = t(item.label);
                }

                return <MenuItem
                    className={`menu-item ${disabled ? 'disabled' : ''}`}
                    key={item.name}
                    title={label}
                    onClick={(e): void => {
                        if (disabled) { return; }
                        item.perform(session);
                        session.contextMenu.isVisible = false;
                    }}
                >
                    <div className="menu-item__label">{label}</div>
                    <kbd className="menu-item__shortcut">{item.name ? getShortcutFromShortcutName(item.name as ShortcutName) : ''}</kbd>
                </MenuItem>;
            }),
        ]}
    </>;
};

const Menu = (props: Props): JSX.Element => {
    const { session } = props;
    const menuRef = useRef<HTMLDivElement>(null);
    const [position, setPosition] = useState<Position>({ left: '0px', top: '0px' });
    const xPos = useRef(0); const yPos = useRef(0);
    const { t } = useTranslation();

    useEffect(() => {
        document.addEventListener('contextmenu', handleContextMenu);
        window.addEventListener('wheel', handleCloseMenu);

        return () => {
            document.removeEventListener('contextmenu', handleContextMenu);
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
