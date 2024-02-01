/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React from 'react';

const App = observer(() => {
    return <></>;
});

window.setTheme = (isDark: boolean): void => {
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

export default App;
