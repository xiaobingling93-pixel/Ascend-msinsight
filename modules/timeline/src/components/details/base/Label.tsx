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
import type { Theme } from '@emotion/react';

interface LabelPropType {
    name: keyof Theme['categories'];
}
const Label = ({ name }: LabelPropType): JSX.Element => {
    const StyledLabel = styled.span`
        color: ${(p): string => p.theme.categories[name].color};
        background-color: ${(p): string => p.theme.categories[name].background};
        font-weight: 400;
        font-size: .85rem;
        border-radius: 4px;
        padding: 3px 8px;
    `;

    return name !== undefined
        ? <StyledLabel> { name } </StyledLabel>
        : <div />;
};

export default Label;
