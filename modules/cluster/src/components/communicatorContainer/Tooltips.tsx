/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import styled from '@emotion/styled';
import React, { CSSProperties, useEffect, useMemo, useRef, useState } from 'react';

// tooltip位置偏移量
const OFFSET_X = 20;
const OFFSET_Y = 16;

export interface TooltipArg {
    x: number;
    y: number;
    content?: Record<string, any> | null;
};

const Tooltip = styled.div(props => ({
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
}));

interface TranslateValue {
    x: number;
    y: number;
}

const TooltipComp = ({ x, y, content }: TooltipArg): JSX.Element => {
    const tooltipRef = useRef<HTMLDivElement>(null);
    const [currentContent, setCurrentContent] = useState<TooltipArg['content']>();
    const [lastTranslate, setLastTranslate] = useState<TranslateValue>({ x: 0, y: 0 });
    const isEmpty = content === null || content === undefined;

    const [translateX, translateY] = useMemo(() => {
        if (!tooltipRef?.current?.parentElement) { return [0, 0]; }
        let translateXValue = x + OFFSET_X;
        const translateYValue = y + OFFSET_Y;
        if (tooltipRef.current.clientWidth + x + OFFSET_X > tooltipRef.current.parentElement.clientWidth) {
            translateXValue = x - tooltipRef.current.clientWidth - OFFSET_X;
        }

        return [translateXValue, translateYValue];
    }, [x, y]);

    useEffect(() => {
        if (!isEmpty) {
            setCurrentContent(content);
            setLastTranslate({ x: translateX, y: translateY });
        }
    }, [content]);

    const styles: CSSProperties = isEmpty
        ? {
            visibility: 'hidden',
            opacity: 0,
            transform: `translate3d(${lastTranslate.x}px, ${lastTranslate.y}px, 0px)`,
        }
        : {
            visibility: 'visible',
            opacity: 1,
            willChange: 'transform',
            transform: `translate3d(${translateX}px, ${translateY}px, 0px)`,
        };

    return <Tooltip ref={tooltipRef} style={styles}>
        {
            currentContent && <div className="formatter">
                {Object.entries(currentContent).map(([key, value]) => (
                    <div className="row" key={JSON.stringify(key)}>
                        <div className="label">{key}</div>
                        <div className="value">{value}</div>
                    </div>
                ))}
            </div>
        }
    </Tooltip>;
};

export interface TooltipProps {
    content?: Record<string, unknown> | null;
    mouseX: number | null;
    mouseY: number | null;
}

export function TooltipComponent({ content, mouseX, mouseY }: TooltipProps): JSX.Element | null {
    if (mouseX === null || mouseY === null) { return null; }

    return <TooltipComp x={mouseX} y={mouseY} content={content} />;
}
