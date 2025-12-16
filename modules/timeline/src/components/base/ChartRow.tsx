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
import { observer } from 'mobx-react';
import type { RowProps } from 'antd/lib/grid';

const ChartRowContainer = styled.div`
    display: flex;
    align-items: center;
`;
interface ChartRowLeftProps {
    width: number;
}

export const ChartRowLeft = styled.div<ChartRowLeftProps>`
    width: ${(props): number => props.width}px;
    height: 100%;
    position: relative;
`;

interface ChartRowRightProps {
    width?: React.CSSProperties['width'];
}

export const ChartRowRight = styled.div<ChartRowRightProps>`
    flex-grow: 1;
    width: ${(props): string => props?.width !== undefined ? `${props.width}` : 'unset'};
    height: 100%;
    position: relative;
    margin-right: ${(props): string => `${props.theme.scrollBarWidth}px`};
`;

export interface ChartRowProps extends RowProps {
    className?: string;
    key?: React.Key;
    children: [ JSX.Element | null, JSX.Element ];
    leftWidth: number;
    rightWidth?: React.CSSProperties['width'];
    rightAreaName?: string;
}

export const ChartRow = observer((props: ChartRowProps) => {
    return <ChartRowContainer className={ props.className }>
        <ChartRowLeft width={props.leftWidth}>
            { props.children[0] ?? <div/> }
        </ChartRowLeft>
        <ChartRowRight id={props.rightAreaName} width={props.rightWidth}>
            { props.children[1] }
        </ChartRowRight>
    </ChartRowContainer>;
});
