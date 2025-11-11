/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useEffect, useRef, useState } from 'react';
import { createPortal } from 'react-dom';
import { useContextMenu } from '../../hooks/useContextMenu';
import { useViewport } from '../../hooks/useViewport';
import { useResizeObserver } from '../../hooks/useResizeObserver';
import styled from '@emotion/styled';
import { useTranslation } from 'react-i18next';

const ContextMenuBox = styled.div<{visible: boolean}>`
    z-index: 99999999;
    position: fixed;
    min-width: 150px;
    padding: 4px 0;
    overflow: hidden;
    font-size: 12px;
    color: ${(props): string => props.theme.textColorPrimary};
    border-radius: 4px;
    background-color: ${(props): string => props.theme.bgColorCommon};
    box-shadow: ${(props): string => props.theme.boxShadow};

    .context-menu-item {
        padding: 4px 16px;
        user-select: none;

        &:not(.disabled):hover{
            background: ${(props): string => props.theme.primaryColorHover};
            color: #ffffff;
        }
        &.disabled{
            color: ${(props): string => props.theme.textColorDisabled};
        }
    }
`;

export interface MenuItem {
    name: string;
    label: string;
    visible?: boolean;
    disabled?: boolean;
    action?: () => void;
}

interface Position {
    x: number;
    y: number;
}

const usePosition = ({ coords, menuSize, vw, vh, menuEl }:
{
    coords: {x: number;y: number};
    menuSize: {width: number;height: number};
    vw: number;
    vh: number;
    menuEl: HTMLDivElement | null;
}): Position | null => {
    const [actualPos, setActualPos] = useState<Position | null>(null);

    useEffect(() => {
        if (!menuEl) {
            setActualPos(null);
            return;
        }

        let posX = coords.x;
        let posY = coords.y;
        const { width: menuWidth, height: menuHeight } = menuSize;

        if (posX > vw - menuWidth) {
            posX -= menuWidth;
        }
        if (posY > vh - menuHeight) {
            posY = vh - menuHeight;
        }

        setActualPos({ x: posX, y: posY });
    }, [coords, menuSize, vw, vh]);

    return actualPos;
};

interface Props {
    menuItems: MenuItem[];
    onSelect?: (item: MenuItem) => void;
    onShow?: (pos: {x: number;y: number}) => void;
    children: React.ReactNode;
}

export const ContextMenu: React.FC<Props> = ({ menuItems, onSelect, onShow, children }) => {
    const containerRef = useRef<HTMLDivElement>(null);
    const [menuEl, setMenuEl] = useState<HTMLDivElement | null>(null);
    const { coords, coordsOffset, visible, setVisible } = useContextMenu(containerRef.current);
    const menuSize = useResizeObserver(menuEl);
    const { vw, vh } = useViewport();
    const pos = usePosition({ coords, menuSize, vw, vh, menuEl });
    const filteredMenuItems = menuItems.filter(item => item.visible);
    const { t } = useTranslation();

    const handleClick = (item: MenuItem): void => {
        if (item.disabled) {
            return;
        }
        setVisible(false);
        onSelect?.(item);
        item?.action?.();
    };

    const handleMenuRef = (el: HTMLDivElement | null): void => {
        if (el) {
            setMenuEl(el);
        }
    };

    useEffect(() => {
        if (visible) {
            onShow?.(coordsOffset);
        }
    }, [visible]);

    return (
        <div ref={containerRef}>
            {children}
            {
                visible &&
                filteredMenuItems.length > 0 &&
                createPortal(
                    <ContextMenuBox
                        ref={handleMenuRef}
                        visible={pos !== null}
                        style={{ left: pos?.x, top: pos?.y }}
                    >
                        {
                            filteredMenuItems.map(item => {
                                const { label, name, disabled } = item;
                                return <div
                                    className={`context-menu-item ${disabled ? 'disabled' : ''}`}
                                    key={name}
                                    onMouseDown={(e) => e.stopPropagation()}
                                    onClick={() => handleClick(item)}
                                >
                                    {t(label)}
                                </div>;
                            })
                        }
                    </ContextMenuBox>,
                    document.body,
                )}
        </div>
    );
};
