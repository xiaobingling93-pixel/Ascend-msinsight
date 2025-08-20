/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import React from 'react';
import { Tooltip } from 'antd';
import type { TooltipProps } from 'antd/lib/tooltip';
import { useTheme } from '@emotion/react';
import { QuestionCircleOutlined } from '@ant-design/icons';

export const MITooltip: React.FC<TooltipProps> = ({ children, overlayInnerStyle, ...props }: TooltipProps) => {
    const theme = useTheme();
    return <Tooltip
        color={theme.bgColorCommon}
        overlayInnerStyle={
            {
                borderRadius: 4,
                borderColor: theme.borderColorLight,
                color: theme.textColorPrimary,
                boxShadow: theme.boxShadow,
                whiteSpace: 'pre-wrap',
                padding: 8,
                fontSize: 12,
                maxWidth: 400,
                ...overlayInnerStyle,
            }
        }
        mouseEnterDelay={0.3}
        {...props}
    >
        {children}
    </Tooltip>;
};

export const MITooltipHelp: React.FC<TooltipProps> = ({ ...props }: TooltipProps) => {
    const theme = useTheme();

    return <MITooltip {...props}>
        <QuestionCircleOutlined style={{ color: theme.icon }} />
    </MITooltip>;
};
