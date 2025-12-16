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

import styled from '@emotion/styled';
import React, { CSSProperties, useEffect, useRef, useMemo, useState } from 'react';

// tooltip位置偏移量
const OFFSET_X = 20;
const OFFSET_Y = 16;
const MINI_OFFSET_Y = 8;
enum Placement {
    TOP_LEFT = 'topLeft',
    RIGHT_BOTTOM = 'rightBottom',
}
type PlacementType = 'topLeft' | 'rightBottom';

const TooltipContainer = styled.div(props => ({
    position: 'absolute',
    backgroundColor: props.theme.bgColorLight,
    color: props.theme.textColorSecondary,
    borderRadius: 4,
    padding: 16,
    whiteSpace: 'nowrap',
    zIndex: 9999999,
    pointerEvents: 'none',
    fontSize: 14,
    boxShadow: 'rgba(0, 0, 0, 0.2) 1px 2px 10px',
    transition: 'opacity 0.2s cubic-bezier(0.23, 1, 0.32, 1), visibility 0.2s cubic-bezier(0.23, 1, 0.32, 1), transform 0.4s cubic-bezier(0.23, 1, 0.32, 1)',
    visibility: 'hidden',
    opacity: 0,
    left: 0,
    top: 0,
    userSelect: 'none',
    willChange: 'transform',
    transformStyle: 'preserve-3d',
}));

interface ITransParams {
    x: number;
    y: number;
    dom: HTMLElement | null;
    placement: PlacementType;
    position: string;
}
const getTranslatePosition = ({ x, y, dom, placement, position }: ITransParams): TranslateValue => {
    if (!dom?.parentElement) {
        return { x: 0, y: 0 };
    }
    let translateX;
    let translateY;
    const boxWidth = position === 'fixed' ? document.body.clientWidth : dom.parentElement.clientWidth;
    switch (placement) {
        case Placement.RIGHT_BOTTOM:
            translateX = x + OFFSET_X;
            translateY = y + OFFSET_Y;
            if (translateX + dom.clientWidth > boxWidth) {
                translateX = x - dom.clientWidth - OFFSET_X;
            }
            break;
        case Placement.TOP_LEFT:
            translateX = x;
            translateY = y - dom.clientHeight - MINI_OFFSET_Y;
            if (translateX + dom.clientWidth > boxWidth) {
                translateX = boxWidth - dom.clientWidth;
            }
            break;
        default:
            translateX = x + OFFSET_X;
            translateY = y + OFFSET_Y;
            break;
    }

    return { x: translateX, y: translateY };
};

interface IPosParams {
    isEmpty: boolean;
    lastTranslate: TranslateValue;
    translatePos: TranslateValue;
    animation: boolean;
}
const getPosStyle = ({ isEmpty, lastTranslate, translatePos, animation }: IPosParams): CSSProperties => {
    const style: CSSProperties = {
        visibility: isEmpty ? 'hidden' : 'visible',
        opacity: isEmpty ? 0 : 1,
        transform: isEmpty
            ? `translate3d(${lastTranslate.x}px,${lastTranslate.y}px , 0px)`
            : `translate3d(${translatePos.x}px, ${translatePos.y}px, 0px)`,
    };
    if (animation) {
        return style;
    }
    return { ...style, transition: 'opacity 0.2s cubic-bezier(0.23, 1, 0.32, 1), transform 0s' };
};

interface TranslateValue {
    x: number;
    y: number;
}

interface IProps {
    content?: Record<string, any> | React.ReactNode[] | null;
    style?: CSSProperties;
    animation?: boolean;
    placement?: PlacementType;
    position?: 'absolute' | 'fixed';
}

type TooltipArg = TranslateValue & IProps;

const TooltipComp = ({ x, y, content, style, animation = true, placement = Placement.RIGHT_BOTTOM, position = 'absolute' }: TooltipArg): JSX.Element => {
    const tooltipRef = useRef<HTMLDivElement>(null);
    const [currentContent, setCurrentContent] = useState<TooltipArg['content']>();
    // 动画效果，从上次位置滑动到当前位置
    const [lastTranslate, setLastTranslate] = useState<TranslateValue>({ x: 0, y: 0 });
    const [windowSize, setWindowSize] = useState({ width: 0, height: 0 });
    const translatePos = useMemo(() => getTranslatePosition({ x, y, dom: tooltipRef.current, placement, position }), [x, y, windowSize]);
    const isEmpty = useMemo(() => content === null || content === undefined, [content]);
    const styles: CSSProperties = getPosStyle({ isEmpty, lastTranslate, translatePos, animation });

    useEffect(() => {
        if (!isEmpty) {
            setCurrentContent(content);
            setLastTranslate(translatePos);
        }
        setTimeout(() => {
            setWindowSize({ width: tooltipRef.current?.clientWidth ?? 0, height: tooltipRef.current?.clientHeight ?? 0 });
        });
    }, [content]);

    return <TooltipContainer ref={tooltipRef} style={{ ...styles, position, ...(style ?? {}) }}>
        {
            Array.isArray(currentContent)
                ? currentContent?.map((item: React.ReactNode, index: number) => (<div key={index}>{item}</div>))
                : <div className="formatter">
                    {Object.entries(currentContent ?? {}).map(([key, value]) => (
                        <div className="row" key={JSON.stringify(key)}>
                            <div className="label">{key}</div>
                            <div className="value">{value}</div>
                        </div>
                    ))}
                </div>
        }
    </TooltipContainer>;
};

export interface ITooltipProps extends IProps {
    mouseX: number | null;
    mouseY: number | null;
}

export function MIDynamicTooltip({ mouseX, mouseY, ...restProps }: ITooltipProps): JSX.Element | null {
    if (mouseX === null || mouseY === null) { return null; }

    return <TooltipComp x={mouseX} y={mouseY} {...restProps} />;
}
