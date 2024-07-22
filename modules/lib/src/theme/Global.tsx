/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Global, css, type Theme } from '@emotion/react';
import React from 'react';
export const GlobalStyles = ({ theme }: {theme: Theme}): JSX.Element => {
    return <Global
        styles={css`
            color: ${theme.textColorPrimary};
        `}
    />;
};
