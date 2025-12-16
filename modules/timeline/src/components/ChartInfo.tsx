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
import { observer } from 'mobx-react';
import React from 'react';

export interface ChartInfoProps {
    title: string;
};

const ChartInfoContainer = styled.div`
    display: flex;
    color: #FFFFFF;
    text-align: start;
    flex-direction: column;
    font-size: 1.5rem;
`;
const ChartInfoBottom = styled.div`
    display: flex;
    flex-grow: 1;
`;
const LeftContainer = styled.div`
        display: flex;
        flex-direction: column;
`;
const ItemTitle = styled.div`
    font-size: 1rem;
    margin: 0.5rem 0 1rem 0;
`;
const Item = styled.div`
    display: flex;
`;
const ItemNum = styled.div`
    font-size: 2rem;
    font-weight: 700;
`;
const ItemPercent = styled.div`
    font-size: .75rem;
    margin: 0.75rem 0 0 0.5rem;
`;

export const ChartInfo = observer(({ title }: ChartInfoProps) => {
    return <ChartInfoContainer>
        <div>{title}</div>
        <ChartInfoBottom>
            <LeftContainer>
                <ItemTitle>Current Used {title}</ItemTitle>
                <Item>
                    <ItemNum>25</ItemNum>
                    <ItemPercent>%</ItemPercent>
                </Item>
            </LeftContainer>
        </ChartInfoBottom>
    </ChartInfoContainer>;
});
