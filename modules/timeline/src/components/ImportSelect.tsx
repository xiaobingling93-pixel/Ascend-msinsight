/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

export interface CardInfo {
    cardName: string;
    rankId: string;
    result: boolean;
    cardPath: string;
    host?: string;
    dataPathList: string[];
};

export default ImportModeSelect;
