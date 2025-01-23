/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Tooltip, Switch } from 'ascend-components';
import localStorageService, { LocalStorageKey } from '@/utils/local-storage';
import { ThemeName } from '@/utils/enum';
import { sendTheme } from '@/connection/sendNotification';

const useTheme = (): [ThemeName, (val: ThemeName) => void] => {
    const [theme, setTheme] = useState(localStorageService.getItem(LocalStorageKey.THEME) ?? ThemeName.DARK);

    useEffect(() => {
        window.setTheme(theme === ThemeName.DARK);
        localStorageService.setItem(LocalStorageKey.THEME, theme);
        sendTheme();
    }, [theme]);
    return [theme, setTheme];
};

// 切换主题
function SwitchTheme(): JSX.Element {
    const [theme, setTheme] = useTheme();
    const isDarkTheme = theme === ThemeName.DARK;
    const onChange = (): void => {
        setTheme(isDarkTheme ? ThemeName.LIGHT : ThemeName.DARK);
    };

    return <Tooltip>
        <Switch checked={isDarkTheme} onChange={onChange} size={'small'}/>
    </Tooltip>;
}

export default SwitchTheme;
