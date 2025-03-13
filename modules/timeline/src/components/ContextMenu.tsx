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
import type { ThreadMetaData, CardMetaData } from '../entity/data';
import { preOrderFlatten } from '../entity/common';
import { type InsightUnit, unit } from '../entity/insight';
import { isPinned, switchPinned } from './ChartContainer/unitPin';
import { getRootUnit } from '../utils';
import { getAutoKey } from '../utils/dataAutoKey';
import { parseCards } from '../api/Request';
import { CardUnit } from '../insight/units/AscendUnit';
import { message } from 'antd';
import {
    actionClearBenchmarkSlice,
    actionCollapseAllUnits, actionDisableAutoUnitHeight,
    actionEnableAutoUnitHeight,
    actionExpandAllUnits,
    actionFindInCommunication,
    actionFitToScreen,
    actionHideFlagEvents, actionHidePythonCallStack,
    actionHideUnits, actionLockSelection,
    actionRecoverDefaultOffset,
    actionResetZoom, actionSetBenchmarkSlice,
    actionShowFlagEvents,
    actionShowHiddenUnits,
    actionShowInEventsView,
    actionShowPythonCallStack,
    actionUndoZoom, actionUnLockSelection,
    actionUnpinAll,
    actionZoomIntoSelection,
} from '../actions';
import { Action } from '../actions/types';
import { getShortcutFromShortcutName, ShortcutName } from '../actions/shortcuts';

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

interface EmptyMetaData {
    count: number;
    dataSource: DataSource;
};

function getRankIdByCardId(cardId: string): string {
    // text 场景：rankId = cardId，直接返回
    // db 场景：cardId = '{文件名} {rankId}'，需特殊处理
    // 匹配最后一个空格之后的所有数字
    const match = cardId.match(/ (?<rankId>\d+)$/);
    if ((match?.groups?.rankId) !== undefined) {
        return match.groups.rankId;
    } else {
        return cardId;
    }
}

function getUnparsedCards(session: Session, rankIds: string[]): InsightUnit[] {
    return preOrderFlatten(getRootUnit(session.units), 0, {
        when: (node) => !(node instanceof CardUnit && node.metadata?.cardName !== 'Host'),
    })
        .filter((item) => item instanceof CardUnit && item.metadata?.cardName !== 'Host' && item.shouldParse)
        .filter((item) => rankIds.includes(getRankIdByCardId((item.metadata as CardMetaData).cardId)));
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
            name: t('timeline:contextMenu.Pin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            title: t('timeline:contextMenu.Pin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            key: 'pinByGroupNameValue',
            event: pinAllSameGroupNameValue,
            disabled: !selectedUnitListStatus.isAllUnpinned,
            visible: selectedUnitListStatus.isAllUnpinned && selectedUnitListStatus.isSameGroupNameValue,
        },
        {
            name: t('timeline:contextMenu.Unpin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            title: t('timeline:contextMenu.Unpin by Group Communication Unit Name', { name: selectedUnitListStatus.groupNameValue }),
            key: 'unpinByGroupNameValue',
            event: unpinAllSameGroupNameValue,
            disabled: !selectedUnitListStatus.isAllPinned,
            visible: selectedUnitListStatus.isAllPinned && selectedUnitListStatus.isSameGroupNameValue,
        },
    ];
}

function buildParseCardsOfRelatedGroupMenuItem(
    session: Session,
    t: TFunction): MenuItemModel[] {
    let name = '';
    let disabled = true;
    let visible = false;
    let unparsedCards: InsightUnit[] = [];

    // 必须只选中一个
    if (session.selectedUnits.length === 1) {
        const rankList = session.selectedUnits.map((item) => (item.metadata as ThreadMetaData).rankList ?? []).flat();
        if (Array.isArray(rankList) && rankList.length !== 0) {
            visible = true;
            unparsedCards = getUnparsedCards(session, rankList);
        }
    }
    const unparsedCardIds = unparsedCards.map((item) => (item.metadata as CardMetaData).cardId);
    if (unparsedCardIds.length === 0) {
        name = t('timeline:contextMenu.Parsed Cards of Related Group');
        disabled = true;
    } else {
        name = t('timeline:contextMenu.Parse Cards of Related Group', { ranks: unparsedCardIds.map(getRankIdByCardId).join(',') });
        disabled = false;
    }

    return [{
        name,
        title: name,
        key: 'parseCardsOfRelatedGroup',
        event: (): void => {
            parseCards({ cards: unparsedCardIds }).then((): void => {
                runInAction((): void => {
                    unparsedCards.forEach((item): void => {
                        item.isParseLoading = true;
                    });
                });
            }).catch(err => {
                message.error(err);
            });
            closeMenu(session);
        },
        disabled,
        visible,
    }];
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
];

const getMenuItems = (props: Props, t: TFunction): JSX.Element => {
    const { session } = props;
    if (!Array.isArray(session.selectedUnits) || session.selectedUnits.length === 0) {
        return <></>;
    }
    // 为多选 unit 功能做准备
    const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
    const menuItems: MenuItemModel[] = [
        ...buildPinAndUnpinByGroupCommunicationUnitNameMenuItem(selectedUnitListStatus, t),
        ...buildParseCardsOfRelatedGroupMenuItem(session, t),
    ];

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
        menuItems.filter(menuItem => menuItem.visible).map(item => (
            <MenuItem
                className={`menu-item ${item.disabled ? 'disabled' : ''}`} key={item.key} title={item.title}
                onClick={(e): void => {
                    if (item.disabled) { return; }
                    item.event(session, item);
                }}
            >
                <div className="menu-item__label">{item.name}</div>
                <kbd className="menu-item__shortcut"></kbd>
            </MenuItem>))]}
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
