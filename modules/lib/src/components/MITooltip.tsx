/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import React from 'react';
import { Tooltip } from 'antd';
import type { TooltipProps } from 'antd/lib/tooltip';
import { useTheme } from '@emotion/react';

export const MITooltip: React.FC<TooltipProps> = ({ children, overlayInnerStyle, ...props }: TooltipProps) => {
    const theme = useTheme();
    return <Tooltip
        color={theme.bgColorLight}
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
            }
        }
        mouseEnterDelay={0.3}
        {...props}
    >
        {children}
    </Tooltip>;
};
