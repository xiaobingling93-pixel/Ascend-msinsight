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
import React, { useState, type CSSProperties, useEffect } from 'react';
import styled from '@emotion/styled';
import type { Property } from 'csstype';
import { Resizor } from '@insight/lib/resize';
import { useWatchDomResize } from '@insight/lib/utils';
import { changeIndexWidth } from './codeViewer/CodeViewer';

export const ThContainer = styled.div`
    background:  ${(p): string => p.theme.bgColorLight};
    color: ${(p): string => p.theme.textColorSecondary};
    display: flex;
    height: 32px;
    line-height: 16px;
    font-size: 12px;
    box-shadow: inset 0 -1px 0 0 ${(p): string => p.theme.borderColorLight};

    & div.th {
        position: relative;
        display: inline-block;
        line-height: 32px;
        height: 32px;
        text-overflow: ellipsis;

        //分隔符号
        &::before , &::after{
            position: absolute;
            background-color: ${(p): string => p.theme.borderColorLight};
            content: "";
            height: 1.6em;
            right: 0;
            top: 25%;
            transition: background-color .3s;
            width: 1px;
        }
        &::before {
            left: 0;
        }

        & > span {
            display: inline-block;
            padding: 0 10px;
        }

        &:last-child {
            flex-grow: 1;
        }
    }
    .resizor {
        cursor: col-resize;
    }
`;

const getInitStyle = (col: Col): CSSProperties => {
    const { width, textAlign } = col;
    return {
        width: width !== undefined ? `${width}px` : undefined,
        textAlign,
    };
};

export interface Col {
    name: React.ReactNode;
    class?: string;
    style?: CSSProperties;
    width?: number;
    textAlign?: Property.TextAlign;
}

interface IProps {
    columns: Col[];
    flex?: boolean;
    minWidth?: number;
}

const defaultIndexWidth = 45;
// 表头（与默认表格保持统一样式）
const TableHead = ({ columns, flex = true, minWidth = 40 }: IProps): JSX.Element => {
    const [cols, setCols] = useState<Col[]>([]);
    const [boxWidth, ref] = useWatchDomResize<HTMLDivElement>('width');

    const handleResize = ({ width, nextWidth, index }: {diff: number; width: number; nextWidth?: number;index: number}): void => {
        // th最小最大宽度
        const checkSize = width >= minWidth && width <= boxWidth - minWidth;
        if (checkSize) {
            cols[index] = { ...cols[index], style: { ...(cols[index].style ?? {}), width: `${width}px`, minWidth: `${width}px` } };
            cols[index + 1] = { ...cols[index + 1], style: { ...(cols[index + 1].style ?? {}), width: `${nextWidth}px` } };
            setCols([...cols]);
            if (cols[index].name === '#') {
                changeIndexWidth(width);
            }
        }
    };

    useEffect(() => {
        setCols(columns.map(col => ({ ...col, style: getInitStyle(col) })));
        const indexCol = columns.find(col => col.name === '#');
        if (indexCol !== undefined) {
            changeIndexWidth(indexCol.width ?? defaultIndexWidth);
        }
    }, [columns]);

    return <ThContainer ref={ref}>
        {
            cols.map((col, index) =>
                (<div key={index} className={`th ${col.class ?? ''}`} style={col.style}>
                    <span>{col.name}</span>
                    {
                        index < columns.length - 1 && flex &&
                        <Resizor onResize={(diff: number, width: number, nextWidth?: number): void => handleResize({ diff, width, nextWidth, index })}/>
                    }
                </div>),
            )
        }
    </ThContainer>;
};
export default TableHead;
