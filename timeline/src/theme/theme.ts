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

function getThemeType(): ThemeType {
    return {
        light,
        dark,
    };
}

window.setTheme = (isDark: boolean) => {
    const isDarkStr = String(isDark);
    themeInstance.setCurrentTheme(isDarkStr === 'false' ? 'light' : 'dark');
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

export const themeInstance = new ThemeStore();
