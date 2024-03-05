/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import * as React from 'react';
import { Tooltip } from 'antd';
import type { TooltipProps } from 'antd/lib/tooltip';
import { useTheme } from '@emotion/react';

export const StyledTooltip: React.FC<TooltipProps> = ({ children, overlayInnerStyle, ...props }: TooltipProps) => {
    const theme = useTheme();
    return <Tooltip
        color={theme.tooltipBGColor}
        overlayInnerStyle={
            {
                borderRadius: '12px',
                color: theme.tooltipFontColor,
                boxShadow: theme.tooltipBoxShadow,
                whiteSpace: 'pre-wrap',
                fontSize: 14,
            }
        }
        mouseEnterDelay={0.3}
        {...props}
    >
        {children}
    </Tooltip>;
};
