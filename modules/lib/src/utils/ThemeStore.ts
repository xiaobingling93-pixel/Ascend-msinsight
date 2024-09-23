/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { Theme } from '@emotion/react';
import { makeAutoObservable } from 'mobx';

export type ThemeItem = 'light' | 'dark';
interface ThemeType {
    light: Theme;
    dark: Theme;
}

export default class ThemeStore {
    currentTheme: ThemeItem;
    theme: ThemeType;
    constructor(theme = { light: {}, dark: {} }) {
        makeAutoObservable(this);
        let lsTheme = localStorage.getItem('theme') ?? 'dark';
        try {
            lsTheme = JSON.parse(lsTheme);
        } catch (e) {
            lsTheme = 'dark';
        }
        this.currentTheme = ['light', 'dark'].includes(lsTheme) ? lsTheme as ThemeItem : 'dark';
        this.theme = theme as ThemeType;
    }

    getCurrentTheme(): ThemeItem {
        return this.currentTheme;
    }

    setCurrentTheme(currentTheme: ThemeItem): void {
        this.currentTheme = currentTheme;
    }

    getTheme(): ThemeType {
        return this.theme;
    }

    getThemeType(): Theme {
        return this.theme[this.currentTheme];
    }
}
