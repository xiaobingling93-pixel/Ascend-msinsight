import { themeInstance } from '../theme';

it('themeInstanceTest', () => {
    const lightTheme = 'light';
    const darkTheme = 'dark';
    expect(themeInstance.getCurrentTheme()).toEqual(darkTheme);
    themeInstance.setCurrentTheme(lightTheme);
    expect(themeInstance.getCurrentTheme()).toEqual(lightTheme);
    window.setTheme(true);
    expect(themeInstance.getCurrentTheme()).toEqual(darkTheme);
    expect(themeInstance.getThemeType()).toEqual(themeInstance.theme[darkTheme]);
    expect(themeInstance.getTheme()).toEqual(themeInstance.theme);
});
