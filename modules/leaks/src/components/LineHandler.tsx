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
import styled from '@emotion/styled';

export const Line = styled.div<{ lineShow: string; offset: number; color: string }>`
    position: absolute;
    top: 45px;
    width: 1.2px;
    height: 386px;
    background-color: #999;
    pointer-events: none;
    display: ${(props): string => props.lineShow};
    transform: translateX(${(props): number => props.offset - 24}px);
    z-index: 10;
    border-left: 1px dashed ${(props): string => props.color};
`;
export const initLine = (mouseEnter: any, mouseMove: any, mouseLeave: any): void => {
    const funcContent = document.getElementById('funcContent');
    const barContent = document.getElementById('barContent');
    [funcContent, barContent].forEach(content => {
        content?.addEventListener('mouseenter', mouseEnter);
        content?.addEventListener('mousemove', mouseMove);
        content?.addEventListener('mouseleave', mouseLeave);
    });
};

export const cancelLine = (mouseEnter: any, mouseMove: any, mouseLeave: any): void => {
    const funcContent = document.getElementById('funcContent');
    const barContent = document.getElementById('barContent');
    [funcContent, barContent].forEach(content => {
        content?.removeEventListener('mouseenter', mouseEnter);
        content?.removeEventListener('mousemove', mouseMove);
        content?.removeEventListener('mouseleave', mouseLeave);
    });
};
