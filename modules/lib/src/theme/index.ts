/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
// Timeline Module
import { light } from './light';
import { dark } from './dark';
import ThemeStore from '../utils/ThemeStore';
import './emotion.d';
export { GlobalStyles } from './Global';
declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
    }
}

export type ThemeItem = 'light' | 'dark';
window.setTheme = (isDark: boolean): void => {
    themeInstance.setCurrentTheme(isDark ? 'dark' : 'light');
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

export const themeInstance = new ThemeStore({ light, dark });
