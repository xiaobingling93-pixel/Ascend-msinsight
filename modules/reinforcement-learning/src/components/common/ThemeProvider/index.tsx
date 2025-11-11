/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { observer } from 'mobx-react-lite';
import { ThemeProvider as EmotionThemeProvider } from '@emotion/react';
import { themeInstance } from '@insight/lib/theme';

interface ThemeProviderProps {
    children: React.ReactNode;
}

export const ThemeProvider: React.FC<ThemeProviderProps> = observer(({ children }) => {
    return (
        <EmotionThemeProvider theme={themeInstance.themeObject}>
            {children}
        </EmotionThemeProvider>
    );
});
