/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useRef, useEffect, useState } from 'react';
import styled from '@emotion/styled';
import type { File, Session } from '@/entity/session';
import type { TFunction } from 'i18next';
import { useTranslation } from 'react-i18next';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { setBaselineData, setCompareData, cancelBaselineData, cancelCompareData } from '@/utils/Compare';

interface Position {
    x: number;
    y: number;
};

interface MenuItemModel {
    label: React.ReactNode;
    key: string;
    action: () => void;
    visible?: boolean;
    disabled?: boolean;
}

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

export const isSameFile = (one: File, anotherOne: File): boolean => {
    return one.projectName === anotherOne.projectName && one.filePath === anotherOne.filePath;
};

// 右键菜单
const getMenuItems = ({ session }: IProps, t: TFunction): JSX.Element => {
    const { selectedFile, compareSet: { baseline, comparison } } = session;
    const { projectName: selectedProjectName, filePath: selectedFilePath } = selectedFile;
    const { projectName: baselineProjectName, filePath: baselineFilePath } = baseline;
    const { projectName: compareProjectName, filePath: compareFilePath } = comparison;
    // 右击的是项目名还是文件名
    const isProject = selectedProjectName !== '' && selectedFilePath === '';
    // 是否基线文件
    const isBaseline = selectedProjectName === baselineProjectName && selectedFilePath === baselineFilePath;
    const isBaselineSetted = baselineProjectName !== '';
    const isBaselineProject = isBaselineSetted && baselineProjectName !== '' && baselineFilePath === '';
    const isComparison = selectedProjectName === compareProjectName && selectedFilePath === compareFilePath;

    const allMenuItems: MenuItemModel[] = [
        {
            label: t('Set as Baseline Data'),
            key: 'setAsBaselineData',
            action: (): void => { setBaselineData(selectedFile); },
            // 此文件（项目）不是基线
            visible: !isBaseline,
        },
        {
            label: t('Unset as Baseline Data'),
            key: 'unsetAsBaselineData',
            action: cancelBaselineData,
            // 此文件（项目）是基线文件
            visible: isBaseline,
        },
        {
            label: t('Set as Comparison Data'),
            key: 'setAsComparisonData',
            action: (): void => { setCompareData(selectedFile); },
            // 不是项目名、不是基线数据、不是对比数据，且基线文件已设置
            visible: !isProject && !isBaseline && !isComparison && isBaselineSetted && !isBaselineProject,
        },
        {
            label: t('Unset as Comparison Data'),
            key: 'unsetAsComparisonData',
            action: cancelCompareData,
            visible: isComparison,
        },
    ];

    return <>
        {allMenuItems.filter(menuItem => menuItem.visible !== false).map(item => (
            <MenuItem className={`menu-item ${item.disabled ? 'disabled' : ''}`} key={item.key}
                onClick={(): void => {
                    if (item.disabled) {
                        return;
                    }
                    item.action();
                    closeMenu(session);
                }}>
                {item.label}
            </MenuItem>))}
    </>;
};

function openMenu(session: Session): void {
    runInAction(() => {
        session.contextMenu.visible = true;
    });
}
function closeMenu(session: Session): void {
    runInAction(() => {
        session.contextMenu.visible = false;
    });
}

// 调整菜单位置：不要超出窗口
function adjustMenuPosition({ menu, setMenuPosition, mousePosition }: {
    menu: HTMLDivElement;
    setMenuPosition: (_: Position) => void;
    mousePosition: Position;
}): void {
    const winWidth = document.documentElement.clientWidth || document.body.clientWidth;
    const winHeight = document.documentElement.clientHeight || document.body.clientHeight;
    let x = mousePosition.x;
    let y = mousePosition.y;
    if (x + menu.offsetWidth >= winWidth) {
        x -= menu.offsetWidth;
    }
    if (y + menu.offsetHeight > winHeight) {
        y -= menu.offsetHeight;
    }
    setMenuPosition({ x, y });
    menu.focus();
};

const MenuContainer = styled.div`
    padding: 3px 0;
    min-width: 100px;
    border-radius: ${(props): string => props.theme.borderRadiusBase};
    background-color:  ${(props): string => props.theme.contextMenuBgColor};
    position: fixed;
    z-index: 99999;
    box-shadow: ${(props): string => props.theme.boxShadowLight};
    user-select: none;
`;
interface IProps {
    session: Session;
}
const Menu = ({ session }: IProps): JSX.Element => {
    const menuRef = useRef<HTMLDivElement>(null);
    const [mousePosition, setMousePosition] = useState<Position>({ x: 0, y: 0 });
    const [menuPosition, setMenuPosition] = useState<Position>({ x: 0, y: 0 });
    const { t } = useTranslation('framework');

    const handleContextMenu = (event: MouseEvent): void => {
        event.preventDefault();
        event.stopPropagation();
        const targetElement = event.target as HTMLElement;
        if (targetElement?.matches('.can-right-click')) {
            setMousePosition({ x: event.clientX, y: event.clientY });
            setMenuPosition({ x: event.clientX, y: event.clientY });
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
        if (session.contextMenu.visible && menu !== null) {
            adjustMenuPosition({ menu, setMenuPosition, mousePosition });
        }
    }, [session.contextMenu.visible, mousePosition]);

    return (
        session.contextMenu.visible
            ? <MenuContainer ref={menuRef} style={{ left: `${menuPosition.x}px`, top: `${menuPosition.y}px` }} tabIndex={-1} onBlur={(): void => { closeMenu(session); }} >
                {getMenuItems({ session }, t)}
            </MenuContainer>
            : <></>
    );
};

const ContextMenu = observer(Menu);
export default ContextMenu;
