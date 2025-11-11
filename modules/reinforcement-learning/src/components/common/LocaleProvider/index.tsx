/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { observer } from 'mobx-react-lite';
import { SharedConfigProvider } from '@insight/lib';
import { useStores } from '@/stores';

interface LocaleProviderProps {
    children: React.ReactNode;
}

export const LocaleProvider: React.FC<LocaleProviderProps> = observer(({ children }) => {
    const { sessionStore } = useStores();

    return (
        <SharedConfigProvider locale={sessionStore.currentLocale}>
            {children}
        </SharedConfigProvider>
    );
});
