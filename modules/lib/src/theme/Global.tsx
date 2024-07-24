/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Global, css, useTheme } from '@emotion/react';
import React from 'react';
export const GlobalStyles = (): JSX.Element => {
    const theme = useTheme();
    return <Global
        styles={css`
            color: ${theme.textColorPrimary};
        `}
    />;
};
