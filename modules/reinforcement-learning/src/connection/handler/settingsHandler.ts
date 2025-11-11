/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { NotificationHandler } from '@/connection/defs';
import { LocaleType, rootStore } from '@/stores';
import { themeInstance, ThemeName } from '@insight/lib/theme';

interface SetThemeHandler {
    isDark: boolean;
}

interface SwitchLanguageHandler {
    lang: LocaleType;
}

export const setThemeHandler: NotificationHandler<SetThemeHandler> = (body): void => {
    themeInstance.setCurrentTheme(body.isDark ? ThemeName.DARK : ThemeName.LIGHT);
};

export const switchLanguageHandler: NotificationHandler<SwitchLanguageHandler> = (body): void => {
    const { sessionStore } = rootStore;
    sessionStore.setLocale(body.lang);
};
