/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import '@emotion/react';

declare module '@emotion/react' {
    export interface Theme {
        contentBackgroundColor: string; // 区域内容背景色
        switchIconColor: string; // 模板下拉按钮颜色
        dividerColor: string; // 界面面板分割线颜色
        closeDragContainerBG: string; // 关闭按钮背景色
    }
}
