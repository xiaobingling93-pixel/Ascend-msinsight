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
import React, { type ReactNode } from 'react';
import styled from '@emotion/styled';

const ImgWithFallback = ({
    className = '',
}): JSX.Element => {
    const PictureContainer = styled.picture`
        img {
            height: 18px;
            width: 18px;
        }
    `;
    return (
        <PictureContainer>
            <div className={className}></div>
        </PictureContainer>
    );
};

export const Mask: React.FC<{ unitPhase: string; isShowMask: boolean } & { children: ReactNode }> = ({ children, ...props }): JSX.Element => {
    const MaskLayer = styled.div`
        position: absolute;
        display: flex;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        z-index: 4;
        width: 100%;
        height: 100%;
        justify-content: center;
        align-items: center;
        background-color: ${(p): string => p.theme.maskColor};
    `;
    const Info = styled.div`
        border-radius: 4px;
        position: relative;
        display: flex;
        align-items: center;
        height: 3rem;
        font-size: .8rem;
        .img {
            margin-right: 10px;
        }
    `;
    return <div style={{ position: 'relative' }}>
        {props.isShowMask && <MaskLayer>
            <Info>
                <ImgWithFallback className={'loading'} />
                { props.unitPhase }</Info>
        </MaskLayer>}
        {children}
    </div>;
};
