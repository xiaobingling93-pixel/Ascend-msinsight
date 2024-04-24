/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';
import { makeAutoObservable } from 'mobx';
import { light } from '../style/light';
import { dark } from '../style/dark';

export type ThemeItem = 'light' | 'dark';

interface ThemeType {
    light: Theme;
    dark: Theme;
}

class ThemeStore {
    currentTheme: ThemeItem;
    theme: ThemeType;
    constructor() {
        makeAutoObservable(this);
        this.currentTheme = 'dark';
        this.theme = getThemeType();
    }

    setCurrentTheme(currentTheme: ThemeItem): void {
        this.currentTheme = currentTheme;
    }

    getThemeType(): Theme {
        return this.theme[this.currentTheme];
    }
}

function getThemeType(): ThemeType {
    return {
        light,
        dark,
    };
}

window.setTheme = (isDark: boolean): void => {
    const isDarkStr = String(isDark);
    themeInstance.setCurrentTheme(isDarkStr === 'false' ? 'light' : 'dark');
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

export const themeInstance = new ThemeStore();
