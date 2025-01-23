/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { light } from './light';
import { dark } from './dark';
import ThemeStore from '../utils/ThemeStore';
import './emotion';
export { GlobalStyles } from './Global';
declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
    }
}

export type ThemeItem = 'light' | 'dark';
export enum ThemeName {
    DARK = 'dark',
    LIGHT = 'light',
}

window.setTheme = (isDark: boolean): void => {
    themeInstance.setCurrentTheme(isDark ? ThemeName.DARK : ThemeName.LIGHT);
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

export const themeInstance = new ThemeStore({ light, dark });
