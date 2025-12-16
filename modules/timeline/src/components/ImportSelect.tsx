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

import React from 'react';
import type { Session } from '../entity/session';
import styled from '@emotion/styled';
import { StyledSelect } from './base/StyledSelect';
import { observer } from 'mobx-react';

const ChildrenContainer = styled.div`
    color: ${(props): string => props.theme.fontColor};
    text-align: left;
    padding-left: 20px;
    user-select: none;
`;

interface SelectProps {
    session: Session;
};

const ImportModeSelect = observer((props: SelectProps) => {
    function dropdownRender(): JSX.Element {
        return (
            <ChildrenContainer>
                <div key={'file'}>Open File</div>
                <div key={'folder'}>Open Folder</div>
            </ChildrenContainer>
        );
    }

    return (
        <StyledSelect
            defaultValue="Import Data"
            dropdownRender={dropdownRender}
            height={24} width={140} itemPaddingLeft={0}>
        </StyledSelect>
    );
});

export interface ImportCardInfo {
    cardName: string;
    cluster: string;
    rankId: string;
    dbPath: string;
    result: boolean;
    cardPath: string;
    host?: string;
    dataPathList: string[];
}

export default ImportModeSelect;
