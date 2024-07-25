/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { type ReactNode, type CSSProperties, useState } from 'react';
import styled from '@emotion/styled';

interface CollapsiblePanelProps {
    title: ReactNode;
    collapsible: boolean; // 是否可展开收起
    children: ReactNode;
    secondary?: boolean; // 次级标题
    style?: CSSProperties;
    headerStyle?: CSSProperties;
    contentStyle?: CSSProperties;
}

const PanelContainer = styled.div<Partial<CollapsiblePanelProps>>`
  margin-bottom: 10px;
  .panel-header{
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
export const CollapsiblePanel: React.FC<CollapsiblePanelProps> = (props): JSX.Element => {
    const { title, collapsible = false, secondary = false, children, style, headerStyle, contentStyle } = props;
    const [isOpen, setIsOpen] = useState(true);

    const togglePanel = (): void => {
        if (collapsible) {
            setIsOpen(!isOpen);
        }
    };

    return (
        <PanelContainer secondary={secondary} style={style}>
            <div className="panel-header" onClick={togglePanel} style={headerStyle}>
                {title}
            </div>
            {isOpen && <div className="panel-content" style={contentStyle}>{children}</div>}
        </PanelContainer>
    );
};

export default CollapsiblePanel;
