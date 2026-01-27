/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

import React from 'react';
import { type Theme, useTheme } from '@emotion/react';

export const DrawerButton = ({ isExpand = false, onClick, style }: { isExpand?: boolean; onClick?: () => void; style?: React.CSSProperties }): JSX.Element => {
    const theme: Theme = useTheme();
    return <div
        style={{
            width: '90px',
            height: '20px',
            backgroundColor: theme.bgColorLight,
            clipPath: 'polygon(0% 0%, 100% 0%, 80% 100%, 20% 100%)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            cursor: 'pointer',
            userSelect: 'none',
            ...style,
        }}
        onClick={onClick}
    >
        <div
            style={{
                width: 0,
                height: 0,
                borderTop: '6px solid #999',
                borderLeft: '6px solid transparent',
                borderRight: '6px solid transparent',
                transform: isExpand ? 'none' : 'rotate(180deg)',
                transition: 'transform 0.2s ease',
            }}
        />
    </div>;
};
