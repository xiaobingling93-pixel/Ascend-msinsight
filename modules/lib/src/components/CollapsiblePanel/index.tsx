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

import React, { type ReactNode, type CSSProperties, useState, forwardRef, useRef, useImperativeHandle } from 'react';
import styled from '@emotion/styled';
import { ExpandIcon, CollapseIcon } from '../../icon/Icon';
import { MITooltipHelp } from '../index';

type RenderFunction = () => ReactNode;
interface CollapsiblePanelProps {
    title: ReactNode;
    collapsible?: boolean; // 是否可展开收起
    children: ReactNode;
    secondary?: boolean; // 次级标题
    style?: CSSProperties;
    headerStyle?: CSSProperties;
    contentStyle?: CSSProperties;
    padding?: string;
    defaultOpen?: boolean;
    id?: string;
    testId?: string; // 用于 playwright 用例
    destroy?: boolean;
    tooltip?: ReactNode | RenderFunction;
    suffix?: ReactNode | RenderFunction;
}

const PanelContainer = styled.div<Partial<CollapsiblePanelProps>>`
  margin-bottom: 10px;
  .panel-header{
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: ${(props): string => props.secondary ? '0 24px' : '6px 24px'};
    background: ${(props): string => props.secondary ? 'transparent' : props.theme.bgColorLight};
    color: ${(props): string => props.theme.textColorPrimary};
    font-weight: 500;
    font-size: ${(props): string => props.theme.fontSizeMedium};
  }

  .panel-content{
    padding: 24px;
  }
`;
export const CollapsiblePanel = forwardRef<HTMLDivElement, CollapsiblePanelProps>((props, ref): JSX.Element => {
    const { title, collapsible = false, secondary = false, children, style, headerStyle, contentStyle, defaultOpen = true, id, testId, destroy = true, tooltip } = props;
    const [isOpen, setIsOpen] = useState(defaultOpen);
    const containerRef = useRef<HTMLDivElement>(null);

    useImperativeHandle(ref, () => containerRef.current as HTMLDivElement);

    const togglePanel = (): void => {
        if (collapsible) {
            setIsOpen(!isOpen);
        }
    };
    let icon;
    if (!collapsible) {
        icon = <></>;
    } else if (isOpen) {
        icon = <ExpandIcon/>;
    } else {
        icon = <CollapseIcon/>;
    }

    return (
        <PanelContainer ref={containerRef} secondary={secondary} style={style} id={id} data-testid={testId} className="mi-collapsible-panel">
            <div className="panel-header" onClick={togglePanel} style={headerStyle}>
                <div className={'flex align-center'}>
                    <div className={'mr-6'}>{icon}{title}</div>
                    { tooltip && <MITooltipHelp title={tooltip}></MITooltipHelp>}
                </div>
                { typeof props.suffix === 'function' ? props.suffix() : props.suffix }
            </div>
            {
                destroy
                    ? <>{isOpen && <div className="panel-content" style={contentStyle}>{children}</div>}</>
                    : <div className="panel-content" style={{ ...contentStyle, display: isOpen ? contentStyle?.display : 'none' }}>{children}</div>
            }
        </PanelContainer>
    );
});

CollapsiblePanel.displayName = 'CollapsiblePanel';

export default CollapsiblePanel;
