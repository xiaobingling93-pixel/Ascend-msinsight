/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { useTheme } from '@emotion/react';
import { Tooltip } from '@insight/lib/components';
import type { TooltipPlacement } from 'antd/lib/tooltip';
import * as React from 'react';

interface AbbreviatureType {
    content: string | JSX.Element;
    availableWidth?: number;
    fontSize?: string;
    placement?: string;
};

const DFT_FONT_SIZE = '1rem';

const containerStyle: React.CSSProperties = {
    overflow: 'hidden',
    width: '100%',
    textOverflow: 'ellipsis',
    whiteSpace: 'nowrap',
};

const calcTextWidth = (container: HTMLDivElement, contentWidth: number,
    setContentWidth: React.Dispatch<React.SetStateAction<number>>): void => {
    if (contentWidth !== 0) {
        return;
    }
    const ele = container.querySelector('span');
    if (!ele) {
        return;
    }
    setContentWidth(ele.offsetWidth);
};

export const Abbreviature = ({ content, availableWidth, fontSize, placement }: AbbreviatureType): JSX.Element => {
    const parent = React.useRef<HTMLDivElement | null>(null);
    const theme = useTheme();
    const [isOverstep, setIsOverstep] = React.useState<boolean>(false);
    const [windowWidth, setWindowWidth] = React.useState<number>(window.innerWidth);
    const [contentWidth, setContentWidth] = React.useState<number>(0);
    const tempFontSize = fontSize ?? DFT_FONT_SIZE;

    // 监听窗口宽度
    const adjustWidth = (): void => setWindowWidth(window.innerWidth);
    React.useEffect(() => {
        window.addEventListener('resize', adjustWidth);
        return () => {
            window.removeEventListener('resize', adjustWidth);
        };
    }, []);

    React.useEffect(() => {
        const parentWidth = availableWidth ?? parent.current?.clientWidth;
        if (parent.current) {
            calcTextWidth(parent.current, contentWidth, setContentWidth);
        }
        const canOverStep = parentWidth !== undefined && contentWidth !== undefined && contentWidth !== 0 && parentWidth !== 0;
        if (canOverStep) {
            setIsOverstep(parentWidth <= contentWidth);
        }
    }, [parent.current, windowWidth, availableWidth, contentWidth]);

    return <>
        {isOverstep
            ? (<Tooltip
                mouseEnterDelay={0.3}
                overlayStyle={{ maxHeight: 200, overflow: 'hidden', fontSize: tempFontSize }}
                placement={(placement ?? 'topLeft') as TooltipPlacement }
                color={theme.selectBackgroundColor}
                overlayInnerStyle={{ color: theme.fontColor, borderRadius: 10, boxShadow: 'none' }}
                title={content}
                destroyTooltipOnHide>
                <div style={{ ...containerStyle, fontSize: tempFontSize }}>
                    <span>{content}</span>
                </div>
            </Tooltip>)
            : (<div ref={parent} style={{ ...containerStyle, fontSize: tempFontSize }}>
                <span>{content}</span>
            </div>)
        }
    </>;
};
