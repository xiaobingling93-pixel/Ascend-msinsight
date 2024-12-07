/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { type ReactNode, type CSSProperties, useState } from 'react';
import styled from '@emotion/styled';
import { ExpandIcon, CollapseIcon } from '../icon/Icon';

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
    const { title, collapsible = false, secondary = false, children, style, headerStyle, contentStyle, defaultOpen = true, id } = props;
    const [isOpen, setIsOpen] = useState(defaultOpen);

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
        <PanelContainer secondary={secondary} style={style} id={id}>
            <div className="panel-header" onClick={togglePanel} style={headerStyle}>
                {icon}{title}
            </div>
            {isOpen && <div className="panel-content" style={contentStyle}>{children}</div>}
        </PanelContainer>
    );
};

export default CollapsiblePanel;
