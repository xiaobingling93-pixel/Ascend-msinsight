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

import * as React from 'react';
import styled from '@emotion/styled';
import type { Theme, Interpolation } from '@emotion/react';
import { useWatchDomResize } from '../../utils/useWatchDomResize';

type HoverContainerProps = React.ClassAttributes<HTMLDivElement>
& React.HTMLAttributes<HTMLDivElement>
& { css?: Interpolation<Theme> }
& { childrenStyle?: string };
function Support(props: HoverContainerProps, ref: React.ForwardedRef<HTMLDivElement>): JSX.Element {
    return <div {...props} ref={ref}/>;
}
const HoverContainer = styled(React.forwardRef(Support))`
    position: relative;
`;

type CircleIconProps = React.ClassAttributes<HTMLDivElement>
& React.HTMLAttributes<HTMLDivElement>
& { css?: Interpolation<Theme> }
& { width: number; height: number; top: number; left: number; hidden?: boolean };

const CircleIcon = styled((props: CircleIconProps) => <div {...props}/>)`
    width: ${(props): number => props.width}px;
    height: ${(props): number => props.height}px;
    display: ${(props): string => props.hidden ? 'none' : 'unset'};
    border-radius: ${(props): number => props.height / 2}px;
    overflow: hidden;
    position: absolute;
    top: ${(props): number => props.top}px;
    left: ${(props): number => props.left}px;
    z-index: 6;
    transition: width 0.2s 0.6s;
    background-color: ${(props): string => props.theme.contentBackgroundColor} !important;
    box-shadow: 1px 3px 5px 5px ${(props): string => props.theme.toolTipShadowColor},
        0 4px 5px 2px ${(props): string => props.theme.toolTipShadowColor},
        0 4px 5px 2px ${(props): string => props.theme.toolTipShadowColor};
`;

interface HoverTipProps {
    hoverComponent: JSX.Element;
    width: number;
    height: number;
    hidden?: boolean;
    children?: React.ReactNode;
}

export const HoverTip: React.FC<HoverTipProps> = (props) => {
    const [circleWidth, setCircleWidth] = React.useState(props.height);
    const [position, setPosition] = React.useState({ left: 0, top: 0 });
    const [rect, ref] = useWatchDomResize<HTMLDivElement>();
    const isDragRef = React.useRef(false);
    const DefaultBoundary = 100;
    return (
        <HoverContainer
            onMouseMove={(e): void => {
                if (isDragRef.current) {
                    setPosition(({ left, top }) => ({
                        left: Math.max(0, Math.min((rect?.width ?? DefaultBoundary) - props.width, left + e.movementX)),
                    top: Math.max(0, Math.min((rect?.height ?? DefaultBoundary) - props.height, top + e.movementY)),
                    }));
                }
            }}
            onMouseLeave={(): void => {
                isDragRef.current = false;
            }}
            onMouseUp={(): void => {
                isDragRef.current = false;
            }}
            ref={ref}
        >
            <CircleIcon
                hidden={props.hidden}
                width={circleWidth}
                height={props.height}
                onMouseEnter={(): void => {
                    setTimeout(() => {
                        if (!isDragRef.current) {
                            setCircleWidth(props.width);
                        };
                    }, 100);
                }}
                onMouseLeave={(): void => { setCircleWidth(props.height); }}
                onMouseDown={(): void => {
                    isDragRef.current = true;
                }}
                {...position}
            >
                {props.hoverComponent}
            </CircleIcon>
            {props.children}
        </HoverContainer>
    );
};
